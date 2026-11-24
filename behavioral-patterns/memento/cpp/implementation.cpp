// ============================================================================
// C++20 DESIGN PATTERN: MEMENTO — DRAWING APPLICATION
// ============================================================================
//
// 0. ARCHITECTURE OVERVIEW AND LEARNING SUMMARY
// ---------------------------------------------
// Goal: capture and restore an object's internal state without exposing its
// representation to the component that manages history. Our challenge extends
// conventional undo/redo with optional restoration of linked shapes.
//
// 0.1. Participants and responsibilities
//   Circle, Rectangle : aggregate value types containing geometry and IDs.
//   Shape             : std::variant<Circle, Rectangle>, not a base class.
//   Drawing           : Originator; owns current shapes, edits and restores.
//   DrawingMemento    : Memento; owns an independent, private snapshot.
//   DrawingHistory    : Caretaker; owns undo/redo stacks, never reads geometry.
//   RestoreMode       : enum class specifying Full, Individual or Linked.
//   HistoryEntry      : snapshot plus optional exact restoration IDs.
//   main() and tests  : Client; create shapes, checkpoint, edit, undo and redo.
//
// 0.2. Relationships (simplified UML notation)
//   Drawing        *-- 0..* Shape          : owns current values
//   DrawingMemento *-- 0..* Shape          : owns saved copies
//   DrawingHistory *-- 0..* HistoryEntry   : owns both history stacks
//   HistoryEntry   *-- 1 DrawingMemento    : owns a snapshot by value
//   Drawing        ..> DrawingMemento     : creates and restores snapshots
//   DrawingHistory ..> Drawing            : requests save and restoration
//   DrawingHistory ..> RestoreMode        : selects restoration scope
//   Shape          ..> Circle, Rectangle  : variant alternatives
//   Client         ..> Drawing, DrawingHistory
//   *-- denotes composition (ownership); ..> denotes dependency (uses).
//   Multiplicity 0..* means zero or more; 1 means exactly one.
//   No inheritance or realization is needed: variants provide type-safe
//   alternatives, not a polymorphic class hierarchy.
//
// 0.3. How we developed the solution
//   1. Define shape values and common ID access using std::visit.
//   2. Implement Drawing: lookup, add, move, resize, print.
//   3. Encapsulate full snapshots in DrawingMemento using friendship.
//   4. Implement two LIFO history stacks and save-before-edit semantics.
//   5. Trace states A (initial), B (circle moved), C (rectangle resized):
//      undo C -> B -> A; redo A -> B -> C.
//   6. Add groupId at the end of both aggregates; existing initialization
//      still works, and snapshots automatically include group membership.
//   7. Add const lookup, getGroupId, getLinkedShapeIds, restoreShape and
//      restoreLinkedShapes. Validate the entire group before restoring it.
//   8. Generalize to restoreShapes with explicit IDs and record the scope
//      in HistoryEntry so redo and repeated undo affect exactly those IDs.
//   9. Share the transfer algorithm and roll back failed history operations.
//  10. Verify the three scopes, multiple undo/redo steps, redo invalidation
//      and a missing group member using independent assertion-based tests.
//
// 0.4. The key undo/redo invariant
//   A checkpoint captures the state BEFORE a new edit. Undo saves the current
//   state onto redo, restores the top undo entry, and pops it on success.
//   Redo performs the reverse transfer. Never call DrawingHistory::save()
//   inside undo/redo: it clears the redo stack. The newest entry is back().
//
//   Operation            Undo (bottom -> top)   Drawing   Redo (bottom -> top)
//   Initial              empty                 A         empty
//   Save and move        A                     B         empty
//   Save and resize      A, B                  C         empty
//   Undo                 A                     B         C
//   Undo                 empty                 A         C, B
//   Redo                 A                     B         C
//   Redo                 A, B                  C         empty
//
// 0.5. Linked-shape twist and chosen semantics
//   groupId == 0 means ungrouped; equal positive IDs identify a group.
//   Full restores the entire vector, including shape additions/removals.
//   Individual restores only the selected existing shape.
//   Linked restores that shape and all its current group members.
//   For selective restoration all requested IDs must exist in both states.
//   Full snapshots are retained even for selective operations. The optional
//   ID list records which parts of a snapshot to apply, not a partial snapshot.
//   Selective undo consumes the latest checkpoint, not an arbitrary earlier
//   edit for a shape. Unselected modifications remain intact for that operation,
//   but the unselected portions of the checkpoint are not retained separately.
//   After redo, ordinary undo uses the recorded scope instead of recalculating
//   group membership. Explicitly changing a recorded scope is rejected.
//
// 0.6. Design principles and trade-offs
//   Encapsulation: the caretaker cannot directly inspect saved geometry.
//   SRP / separation of concerns: drawing edits and history management differ.
//   DRY: shared helpers, restoreShapes and transfer avoid duplicated logic.
//   Value semantics / RAII: containers own their contents and release resources.
//   Strong exception guarantee for restoration in this value-based model:
//   validate before mutation, copy then swap for full restore, roll back the
//   destination entry if restore fails. This is not thread synchronization.
//   Limitations: complete snapshots cost memory proportional to drawing size
//   per checkpoint; shape lookup is linear; each shape has one group; a
//   history should be used with its own drawing. No arbitrary per-shape
//   history, nested groups, history size limit, or selective add/delete undo.
//   Checkpoint creation and editing are separate: a failed edit after save()
//   may still change history. Geometry validation is limited to resize factor
//   and ID/group checks; finite inputs alone do not guarantee finite results.
//   Possible extensions: bounded history, indexed lookup, delta snapshots,
//   transactional edits or Command Pattern integration.
//
// 0.7. Build and run (assertions enabled; do not define NDEBUG for tests)
//   g++ -std=c++20 -Wall -Wextra -Wpedantic memento_drawing_documented.cpp -o memento
//   ./memento
// ============================================================================

// ============================================================
// 1. Headers
// ============================================================
#include <cassert>      // assert(): verify expected test results
#include <cmath>        // std::isfinite(): validate resize factor
#include <iostream>     // std::cout
#include <optional>     // std::optional, std::nullopt
#include <stdexcept>    // std::invalid_argument
#include <type_traits>  // std::remove_cvref_t, std::is_same_v
#include <utility>      // std::move
#include <variant>      // std::variant, std::visit, std::get
#include <vector>       // std::vector: shapes and LIFO stacks

// ============================================================
// 2. Shape data structures and helper functions
// ============================================================
// Aggregates: braces initialize members in declaration order and reject
// narrowing conversions. Parenthesized aggregate initialization is also
// valid in C++20, but we preferred braces. Omitted groupId defaults to zero.
struct Circle {
    int id{};
    double x{};
    double y{};
    double radius{};
    int groupId{}; // 0 = ungrouped; positive = group identifier.
};

struct Rectangle {
    int id{};
    double x{};
    double y{};
    double width{};
    double height{};
    int groupId{};
};

// A variant holds exactly one active alternative in this application.
// It does not expose the members of either alternative directly.
using Shape = std::variant<Circle, Rectangle>;

// Our shared helper avoids writing shape.id on a variant (not valid).
// std::visit invokes a callable on the currently active alternative.
// [] captures nothing; const auto& is the read-only argument supplied by visit.
// Both alternatives contain int id, so no if constexpr is necessary.
int getShapeId(const Shape& shape) {
    return std::visit([](const auto& value) {
        return value.id;
    }, shape);
}

// Same visitor technique for groupId. If shape is a Shape*, call
// getGroupId(*shape), not *shape.groupId or shape->groupId: even after
// dereferencing, the object is a variant, not a Circle or Rectangle.
int getGroupId(const Shape& shape) {
    return std::visit([](const auto& value) {
        return value.groupId;
    }, shape);
}

// ============================================================
// 3. Memento - encapsulated snapshot
// ============================================================
// Define this class before Drawing: inline save/restore require a complete
// type. A forward declaration alone is not enough; do not define it twice.
// The private constructor is explicit to prevent implicit conversion.
// Friendship grants Drawing access, not DrawingHistory. Implicit copy/move
// special members allow snapshots to be stored by value in standard containers.
class DrawingMemento {
private:
    friend class Drawing;
    std::vector<Shape> shapes_;

    // Only Drawing can create a snapshot from drawing data.
    explicit DrawingMemento(const std::vector<Shape>& shapes)
        : shapes_(shapes) {}
};

// ============================================================
// 4. Originator - drawing and restoration logic
// ============================================================
class Drawing {
private:
    std::vector<Shape> shapes_;

    // Mutable lookup for editing and restoration.
    // Return the address of the matching element, or nullptr if not found.
    // These are borrowed pointers, not owning allocations. Do not retain
    // them across vector reallocation or full restoration.
    Shape* findShapeById(int id) {
        for (Shape& shape : shapes_) {
            if (getShapeId(shape) == id) {
                return &shape;
            }
        }
        return nullptr;
    }

    // Const overload for read-only operations.
    // The leading const qualifies the pointed-to Shape; the trailing const
    // qualifies the Drawing member function. A const method must iterate
    // over this vector with const Shape&, not Shape&. Overload resolution
    // chooses the appropriate method from the constness of the Drawing.
    const Shape* findShapeById(int id) const {
        for (const Shape& shape : shapes_) {
            if (getShapeId(shape) == id) {
                return &shape;
            }
        }
        return nullptr;
    }

    // Read-only validation helper. Return immediately when an ID matches;
    // return false only after searching the whole saved vector.
    bool mementoContainsShape(const DrawingMemento& memento,
                              int id) const {
        for (const Shape& shape : memento.shapes_) {
            if (getShapeId(shape) == id) {
                return true;
            }
        }
        return false;
    }

public:
    // 4.1. Add and modify shapes.
    // Accept by value: Circle/Rectangle convert to Shape; move into storage.
    // A shape ID identifies one object; a group ID may be shared.
    void addShape(Shape shape) {
        if (findShapeById(getShapeId(shape))) {
            throw std::invalid_argument("Shape ID already exists!");
        }
        if (getGroupId(shape) < 0) {
            throw std::invalid_argument("Invalid group ID!");
        }
        shapes_.push_back(std::move(shape));
    }

    void moveShape(int id, double dx, double dy) {
        Shape* shape = findShapeById(id);
        if (!shape) {
            throw std::invalid_argument("Shape not found!");
        }

        // Both shape alternatives have x and y members.
        // [dx, dy] captures values from the enclosing method; auto& value
        // is a separate parameter supplied by std::visit. The reference
        // makes changes affect the stored shape rather than a copy.
        std::visit([dx, dy](auto& value) {
            value.x += dx;
            value.y += dy;
        }, *shape);
    }

    void resizeShape(int id, double factor) {
        if (!(factor > 0.0 && std::isfinite(factor))) {
            throw std::invalid_argument("Invalid resize factor!");
        }
        Shape* shape = findShapeById(id);
        if (!shape) {
            throw std::invalid_argument("Shape not found!");
        }

        // Resize the properties specific to the active alternative.
        // factor must be positive AND finite (check factor, not value).
        // decltype(value) includes reference/cv qualifiers; remove_cvref_t
        // yields Circle or Rectangle. is_same_v gives a compile-time bool.
        // if constexpr discards the irrelevant dependent branch, unlike an
        // ordinary if: Circle has no width and Rectangle has no radius.
        // visit selects the active alternative at runtime; the lambda is
        // instantiated separately for each alternative at compile time.
        std::visit([factor](auto& value) {
            using T = std::remove_cvref_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Circle>) {
                value.radius *= factor;
            } else if constexpr (std::is_same_v<T, Rectangle>) {
                value.width *= factor;
                value.height *= factor;
            }
        }, *shape);
    }

    // 4.2. Save and restore the entire drawing.
    // Snapshot creation is read-only. Copying the vector copies all geometry
    // and group IDs, so later edits do not alter the snapshot. [[nodiscard]]
    // encourages callers to use the returned memento.
    [[nodiscard]] DrawingMemento save() const {
        return DrawingMemento(shapes_);
    }

    void restore(const DrawingMemento& memento) {
        // Copy first: if allocation fails, the drawing is unchanged.
        // Then swap the vectors (non-throwing with our standard allocator).
        auto restoredShapes = memento.shapes_;
        shapes_.swap(restoredShapes);
    }

    // 4.3. Determine the current group membership.
    [[nodiscard]] std::vector<int> getLinkedShapeIds(int id) const {
        const Shape* shape = findShapeById(id);
        if (!shape) {
            throw std::invalid_argument("Shape not found!");
        }

        // Dereference the borrowed pointer, then use the visitor helper.
        const int groupId = getGroupId(*shape);
        // Ungrouped shapes are not linked to each other.
        if (groupId == 0) {
            // The declared return type is vector<int>; braces create a
            // one-element vector. Other ungrouped shapes are NOT linked.
            return {id};
        }

        // Declare the vector before using it. The loop below includes the
        // selected shape itself and preserves drawing order. existingShape
        // is already a reference: no pointer dereference is necessary.
        std::vector<int> linkedShapeIds;
        for (const Shape& existingShape : shapes_) {
            if (getGroupId(existingShape) == groupId) {
                linkedShapeIds.push_back(getShapeId(existingShape));
            }
        }
        return linkedShapeIds;
    }

    // 4.4. Restore one existing shape from a saved snapshot.
    void restoreShape(const DrawingMemento& memento, int id) {
        Shape* currentShape = findShapeById(id);
        if (!currentShape) {
            throw std::invalid_argument("Shape not found!");
        }

        for (const Shape& savedShape : memento.shapes_) {
            if (getShapeId(savedShape) == id) {
                // Copy the entire variant (all fields and active type).
                // No visitor is needed for whole-value assignment.
                // Do not return &savedShape: this method returns void.
                *currentShape = savedShape;
                // Early return is essential: otherwise the final throw
                // would execute even after successfully restoring the shape.
                return;
            }
        }
        throw std::invalid_argument("Shape not found in memento!");
    }

    // 4.5. Restore an explicit set of shape IDs atomically.
    void restoreShapes(const DrawingMemento& memento,
                       const std::vector<int>& shapeIds) {
        // Phase 1: validate everything before changing any shape.
        // We discussed why one validation/restoration loop is insufficient:
        // if the third group member is absent from the memento, the first
        // two might already have changed. Separate passes prevent that.
        for (int id : shapeIds) {
            if (!findShapeById(id) || !mementoContainsShape(memento, id)) {
                throw std::invalid_argument(
                    "Shape missing from drawing or memento!");
            }
        }

        // Phase 2: these value-based shape assignments cannot throw.
        // For our int/double alternatives the variant assignment is safe;
        // if future shape types can throw during copying, reconsider this
        // strong-guarantee argument (e.g. stage changes in a temporary vector).
        for (int id : shapeIds) {
            restoreShape(memento, id);
        }
    }

    // 4.6. Restore the selected shape and its current group.
    void restoreLinkedShapes(const DrawingMemento& memento, int id) {
        // Collect IDs before any group membership can be restored. A direct
        // range-for over getLinkedShapeIds(id) is also valid: the temporary
        // vector survives the loop. Here we store it to support validation.
        // When iterating IDs, use each linkedShapeId, not the original id.
        const auto linkedShapeIds = getLinkedShapeIds(id);
        restoreShapes(memento, linkedShapeIds);
    }

    // 4.7. Read-only inspection for tests and display.
    // Return a copy so callers cannot modify internal drawing data.
    [[nodiscard]] Shape getShape(int id) const {
        const Shape* shape = findShapeById(id);
        if (!shape) {
            throw std::invalid_argument("Shape not found!");
        }
        return *shape;
    }

    // 4.8. Display without modifying the drawing.
    // [] captures no outside variables; const auto& prevents mutation.
    // Use width (not with) and pass the actual loop variable to std::visit.
    // Both alternatives expose groupId, so print it outside if constexpr.
    void print() const {
        auto visitor = [](const auto& value) {
            using T = std::remove_cvref_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Circle>) {
                std::cout << "Circle [ID: " << value.id
                          << "] Position: (" << value.x << ", " << value.y
                          << "), Radius: " << value.radius;
            } else if constexpr (std::is_same_v<T, Rectangle>) {
                std::cout << "Rectangle [ID: " << value.id
                          << "] Position: (" << value.x << ", " << value.y
                          << "), Width: " << value.width
                          << ", Height: " << value.height;
            }
            std::cout << ", Group: " << value.groupId << '\n';
        };

        for (const Shape& shape : shapes_) {
            std::visit(visitor, shape);
        }
    }
};

// ============================================================
// 5. Restoration scope and history entries
// ============================================================
// Three modes need more than a bool. enum class provides scoped names
// and no implicit conversion to int. Enumerators use commas, not semicolons.
enum class RestoreMode {
    Full,
    Individual,
    Linked
};

// Aggregate initialization: {drawing.save(), std::nullopt} for full, or
// {drawing.save(), std::vector<int>{1, 2}} for specific IDs. An optional
// containing an empty vector is distinct from nullopt (restore nothing).
struct HistoryEntry {
    DrawingMemento memento;
    // nullopt = full restoration; a vector = exact selected IDs.
    std::optional<std::vector<int>> shapeIds;
};

// ============================================================
// 6. Caretaker - undo and redo history
// ============================================================
class DrawingHistory {
private:
    std::vector<HistoryEntry> undoStack_;
    std::vector<HistoryEntry> redoStack_;

    // 6.1. Shared algorithm for undo and redo. The stacks are distinct.
    // Both operations save the current state to the opposite stack and
    // restore from the source stack. Only the direction changes. The
    // source and destination must be different vectors; references to
    // source.back().shapeIds remain valid until source.pop_back().
    static void transfer(Drawing& drawing,
                         std::vector<HistoryEntry>& source,
                         std::vector<HistoryEntry>& destination,
                         const std::optional<std::vector<int>>& shapeIds) {
        // Save the current state with the exact restoration scope.
        // If saving or pushing throws, nothing has been restored yet.
        destination.push_back({drawing.save(), shapeIds});

        try {
            if (shapeIds) {
                drawing.restoreShapes(source.back().memento, *shapeIds);
            } else {
                drawing.restore(source.back().memento);
            }
        } catch (...) {
            // Restoration failed: remove the temporary history entry.
            destination.pop_back();
            throw;
        }

        // Consume the source entry only after successful restoration.
        source.pop_back();
    }

public:
    // 6.2. Save before a new edit. A new checkpoint clears redo history.
    // Call drawing.save() (with parentheses) to obtain the memento.
    // Clear redo only after successfully adding the new undo entry.
    void save(const Drawing& drawing) {
        undoStack_.push_back({drawing.save(), std::nullopt});
        redoStack_.clear();
    }

    // 6.3. Availability checks: vector::empty() is non-throwing.
    [[nodiscard]] bool canUndo() const noexcept {
        return !undoStack_.empty();
    }

    [[nodiscard]] bool canRedo() const noexcept {
        return !redoStack_.empty();
    }

    // 6.4. Ordinary undo: full for a fresh checkpoint, or the previously
    // recorded scope if this entry came from a selective redo.
    void undo(Drawing& drawing) {
        if (canUndo()) {
            transfer(drawing, undoStack_, redoStack_,
                     undoStack_.back().shapeIds);
        }
    }

    // 6.5. Explicitly choose a scope for a fresh, unrestricted checkpoint.
    // Linked computes membership once; later history transfers use exact IDs.
    void undo(Drawing& drawing, int id, RestoreMode mode) {
        if (!canUndo()) {
            return;
        }

        // Never silently widen or replace an already recorded scope.
        if (undoStack_.back().shapeIds) {
            throw std::invalid_argument(
                "Scope already recorded; use undo(drawing)!");
        }

        std::optional<std::vector<int>> shapeIds;
        switch (mode) {
            case RestoreMode::Full:
                // Leave shapeIds disengaged; id is ignored.
                break;
            case RestoreMode::Individual:
                shapeIds = std::vector<int>{id};
                break;
            case RestoreMode::Linked:
                shapeIds = drawing.getLinkedShapeIds(id);
                break;
            default:
                throw std::invalid_argument("Invalid restoration mode!");
        }

        transfer(drawing, undoStack_, redoStack_, shapeIds);
    }

    // 6.6. Redo always uses the scope recorded by the preceding undo.
    // Do not call history.save() here: it would erase pending redo entries.
    void redo(Drawing& drawing) {
        if (canRedo()) {
            transfer(drawing, redoStack_, undoStack_,
                     redoStack_.back().shapeIds);
        }
    }
};

// ============================================================
// 7. Client - demonstration and verification
// ============================================================
// 7.1. Sample drawing: two linked shapes and one independent shape.
Drawing makeDrawing() {
    Drawing drawing;
    drawing.addShape(Circle{1, 10.0, 20.0, 5.0, 1});
    drawing.addShape(Rectangle{2, 30.0, 40.0, 15.0, 10.0, 1});
    drawing.addShape(Circle{3, 50.0, 60.0, 8.0});
    return drawing;
}

// 7.2. Test all three scopes. One checkpoint covers three modifications.
//                 Modified      Full undo     Individual     Linked
// Circle 1 x/y    (25,30)       (10,20)       (10,20)        (10,20)
// Rectangle 2    30 x 20       15 x 10       30 x 20        15 x 10
// Circle 3 radius 16            8             16             16
// Redo reapplies the selected changes; unselected shapes remain untouched.
void testRestoration(RestoreMode mode, const char* label) {
    Drawing drawing = makeDrawing();
    DrawingHistory history;

    // One checkpoint for three modifications, so selective undo
    // can demonstrate which of those modifications remain intact.
    history.save(drawing);
    drawing.moveShape(1, 15.0, 10.0);
    drawing.resizeShape(2, 2.0);
    drawing.resizeShape(3, 2.0);

    std::cout << "\n=== " << label << " ===\nBefore undo:\n";
    drawing.print();

    if (mode == RestoreMode::Full) {
        history.undo(drawing);
    } else {
        history.undo(drawing, 1, mode);
    }
    std::cout << "After undo:\n";
    drawing.print();

    // Verify exactly which shapes were restored.
    assert(std::get<Circle>(drawing.getShape(1)).x == 10.0);
    assert(std::get<Rectangle>(drawing.getShape(2)).width ==
           (mode == RestoreMode::Individual ? 30.0 : 15.0));
    assert(std::get<Circle>(drawing.getShape(3)).radius ==
           (mode == RestoreMode::Full ? 8.0 : 16.0));
    assert(!history.canUndo() && history.canRedo());

    history.redo(drawing);
    std::cout << "After redo:\n";
    drawing.print();
    assert(std::get<Circle>(drawing.getShape(1)).x == 25.0);
    assert(std::get<Rectangle>(drawing.getShape(2)).width == 30.0);
    assert(std::get<Circle>(drawing.getShape(3)).radius == 16.0);

    // Undo again without selecting a new scope.
    history.undo(drawing);
    assert(std::get<Circle>(drawing.getShape(1)).x == 10.0);
    assert(std::get<Rectangle>(drawing.getShape(2)).width ==
           (mode == RestoreMode::Individual ? 30.0 : 15.0));
    assert(std::get<Circle>(drawing.getShape(3)).radius ==
           (mode == RestoreMode::Full ? 8.0 : 16.0));

    // A new checkpoint invalidates redo history.
    history.save(drawing);
    drawing.moveShape(1, 1.0, 0.0);
    assert(history.canUndo() && !history.canRedo());
}

// 7.3. Verify LIFO ordering with the A -> B -> C example from our exercise.
void testMultipleUndoRedo() {
    Drawing drawing = makeDrawing();
    DrawingHistory history;
    history.save(drawing);
    drawing.moveShape(1, 15.0, 10.0);
    history.save(drawing);
    drawing.resizeShape(2, 2.0);

    history.undo(drawing);
    assert(std::get<Rectangle>(drawing.getShape(2)).width == 15.0);
    history.undo(drawing);
    assert(std::get<Circle>(drawing.getShape(1)).x == 10.0);
    assert(!history.canUndo());

    history.redo(drawing);
    assert(std::get<Circle>(drawing.getShape(1)).x == 25.0);
    history.redo(drawing);
    assert(std::get<Rectangle>(drawing.getShape(2)).width == 30.0);
    assert(!history.canRedo());
}

// 7.4. Verify failure before mutation and rollback of the destination entry.
void testFailedLinkedUndo() {
    Drawing drawing = makeDrawing();
    DrawingHistory history;
    history.save(drawing);
    drawing.moveShape(1, 15.0, 10.0);
    // This group member does not exist in the saved snapshot.
    drawing.addShape(Circle{4, 70.0, 80.0, 5.0, 1});

    bool failed = false;
    try {
        history.undo(drawing, 1, RestoreMode::Linked);
    } catch (const std::invalid_argument& error) {
        failed = true;
        std::cout << "\nExpected failure: " << error.what() << '\n';
    }
    // Neither the drawing nor the history should have changed.
    assert(failed);
    assert(std::get<Circle>(drawing.getShape(1)).x == 25.0);
    assert(std::get<Circle>(drawing.getShape(4)).id == 4);
    assert(history.canUndo() && !history.canRedo());
}

// 7.5. Client entry point. Each test constructs an independent drawing.
int main() {
    testRestoration(RestoreMode::Full, "Full restoration");
    testRestoration(RestoreMode::Individual, "Individual restoration");
    testRestoration(RestoreMode::Linked, "Linked restoration");
    testMultipleUndoRedo();
    testFailedLinkedUndo();
    std::cout << "\nAll tests passed.\n";
    return 0;
}
