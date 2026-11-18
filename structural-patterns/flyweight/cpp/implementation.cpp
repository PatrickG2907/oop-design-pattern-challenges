/*
============================================================================
// Flyweight Pattern
//
// Flyweight reduces memory consumption by sharing objects that contain common
// intrinsic state instead of storing duplicate copies of that state in every
// individual object.
//
// Each context stores only its individual information and a reference to a
// shared Flyweight. The Flyweight combines its intrinsic state with extrinsic
// state supplied by the context when performing an operation.
//
// Pattern roles in this example:
//   - Abstract Flyweight:    Flyweight
//   - Concrete Flyweight:    CharacterStyle
//   - Flyweight Factory:     StyleFactory
//   - Flyweight Key:         StyleKey
//   - Context:               Character
//   - Client / Coordinator:  TextEditor
//   - Demo:                  main()
//
// Architecture:
//
//   TextEditor
//   |
//   |-- StyleFactory
//   |     |
//   |     |-- map<StyleKey, unique_ptr<const Flyweight>>
//   |            |
//   |            +-- CharacterStyle (shared intrinsic state)
//   |
//   |-- vector<Character>
//   |     |
//   |     +-- Each Character holds a non-owning const Flyweight*
//   |
//   +-- set<size_t> (dynamically highlighted positions)
//
// Intrinsic state:
//   - Bold
//   - Italic
//   - Color
//
// Extrinsic state:
//   - Character value
//   - Highlighting status
//
// CharacterStyle stores immutable intrinsic state, allowing multiple
// Character objects to reference the same style safely.
//
// Character stores its individual character value, while TextEditor maintains
// highlighting separately. Both are supplied to Flyweight::render().
//
// StyleFactory creates and caches CharacterStyle instances. Identical
// formatting requests return the same Flyweight object.
//
// Ownership model:
//   - StyleFactory exclusively owns Flyweights through unique_ptr.
//   - Character holds non-owning const Flyweight* pointers.
//   - TextEditor owns both the factory and its characters.
//
// Sharing objects does not require sharing ownership. The factory must
// outlive every character that references one of its Flyweights.
//
// The design deliberately uses an abstract Flyweight interface to demonstrate
// polymorphism and Dependency Inversion. The factory still constructs the
// concrete CharacterStyle, which is appropriate for this focused example.
//
// Rendering is simulated through console output. Highlighting is represented
// by a set of positions rather than a graphical rendering system.
//
// No cache eviction, text insertion/deletion, or advanced memory management
// is implemented, keeping the example focused on the Flyweight pattern.
//
// Requires C++20 for operator<=> and std::set::contains().
// ============================================================================
*/


#include <iostream>      // std::cout
#include <string>        // std::string
#include <string_view>   // std::string_view
#include <memory>        // std::unique_ptr, std::make_unique
#include <map>           // std::map
#include <vector>        // std::vector
#include <set>           // std::set
#include <compare>       // C++20 three-way comparison (operator<=>)
#include <stdexcept>     // std::invalid_argument, std::out_of_range
#include <utility>       // std::move
#include <cassert>       // assert()
#include <cstddef>       // std::size_t


// ============================================================================
// Abstract Flyweight
// ============================================================================

// Flyweight defines the common interface for shared style objects.
// Notes:
//  - The interface does not store intrinsic or extrinsic state.
//  - render() receives the character value and highlighting status as
//    extrinsic state supplied at runtime.
//  - The const-qualified operation prevents modification of ordinary
//    Flyweight state during rendering.
//  - A virtual destructor enables safe polymorphic destruction.
//  - Keeping the interface minimal follows the Interface Segregation Principle.
class Flyweight {
public:
    virtual ~Flyweight() = default;
    
    virtual void render(char value, bool highlighted) const = 0;
};


// ============================================================================
// Concrete Flyweight
// ============================================================================

// CharacterStyle is the Concrete Flyweight.
// Notes:
//  - Stores intrinsic state shared by multiple Character objects.
//  - The formatting properties are immutable after construction.
//  - No setters are provided because modifying a shared style could affect
//    every character referencing that Flyweight.
//  - render() combines the stored intrinsic state with extrinsic state
//    supplied by the caller.
//  - Implements the Flyweight interface, supporting polymorphic use.
class CharacterStyle : public Flyweight {
private:
    // Intrinsic state: shared and immutable.
    const bool bold_;
    const bool italic_;
    const std::string color_;
    
    // Validation belongs to the class that owns the formatting invariant.
    static void validate_color(std::string_view color) {
        if (color.empty()) {
            throw std::invalid_argument("Color must not be empty!");
        }
    }
    
public:
    CharacterStyle(bool bold, bool italic, std::string color)
        : bold_(bold), italic_(italic), color_(std::move(color))
    {
        validate_color(color_);
    }
    
    // Combines intrinsic formatting with runtime extrinsic state.
    // Notes:
    //  - value and highlighted are not stored in CharacterStyle.
    //  - The same Flyweight can render different characters with different
    //    highlighting states.
    //  - Console output represents rendering for this exercise.
    void render(char value, bool highlighted) const override {
        std::cout
            << "Rendering character: "
            << value
            << ". Formatting options: "
            << "bold: "
            << ((bold_) ? "true" : "false")
            << ", italic: "
            << ((italic_) ? "true" : "false")
            << ", color: "
            << color_
            << ". Highlighted: "
            << ((highlighted) ? "true" : "false")
            << '\n';
    }
};


// ============================================================================
// Flyweight Key
// ============================================================================

// StyleKey identifies a unique combination of intrinsic state.
// Notes:
//  - Two keys with identical formatting properties represent the same style.
//  - The factory uses the key to find an existing Flyweight before creating
//    a new one.
//  - StyleKey is a simple value type, not a Flyweight itself.
//  - The defaulted C++20 spaceship operator provides lexicographical
//    comparison of members in declaration order.
//  - This supports std::map ordering without a custom comparator.
struct StyleKey{
    bool bold;
    bool italic;
    std::string color;
    
    auto operator<=>(const StyleKey& other) const = default;
};


// ============================================================================
// Flyweight Factory
// ============================================================================

// StyleFactory creates and reuses Flyweight objects.
// Notes:
//  - Maintains one Flyweight per unique StyleKey.
//  - Owns cached Flyweights exclusively through unique_ptr.
//  - Returns non-owning const Flyweight* pointers to clients.
//  - Clients depend on the abstract Flyweight rather than CharacterStyle.
//  - The factory knows the concrete type because object creation is its
//    responsibility. A more elaborate factory abstraction is unnecessary.
//  - Cached Flyweights remain alive until the factory is destroyed.
//
// Ownership is deliberately centralized in the factory rather than shared
// through reference-counted smart pointers.
class StyleFactory {
private:
    // Each key identifies one exclusively owned, immutable Flyweight
    std::map<StyleKey, std::unique_ptr<const Flyweight>> styles_;

public:
    // Returns an existing Flyweight or creates one if no matching style exists.
    const Flyweight* getStyle(
        bool bold,
        bool italic,
        std::string_view color
    ) {
        // The cache key owns its string.
        // Note:
        //  - string_view itself is non-owning and must not be stored as a view
        //    without guaranteeing the lifetime of the referenced characters.
        StyleKey key{bold, italic, std::string{color}};

        // Check whether the style already exists
        // Note:
        //  - find() performs one lookup without inserting a missing entry.
        auto it = styles_.find(key);

        if (it != styles_.end()) {
            // Return the previously created Flyweight.
            // Note:
            //  - get() exposes the managed object's address without ownership.
            return it->second.get();
        }

        // No matching Flyweight exists: construct a new CharacterStyle flyweight.
        // Note:
        //  - make_unique dynamically allocates the object and returns
        //    a unique_ptr that exclusively owns it.
        auto style = std::make_unique<CharacterStyle>(
            bold, italic, key.color
        );

        // Save a non-owning pointer before transferring ownership.
        // Note:
        //  - The implicit conversion to const Flyweight* uses the abstract
        //    interface and prevents modification through the returned pointer.
        const Flyweight* ptr = style.get();

        // Transfer ownership of the new Flyweight into the cache.
        // Notes:
        //  - unique_ptr cannot be copied, so std::move is required.
        //  - Moving a unique_ptr transfers ownership, not the managed object.
        //  - The CharacterStyle remains at its original memory address.
        //  - The local style becomes empty after a successful move.
        //  - ptr remains valid while the cached Flyweight remains alive.
        styles_.emplace(std::move(key), std::move(style));

        return ptr;
    }
    
    // Exposes the number of cached Flyweights without exposing the cache.
    // Note:
    //  - This diagnostic operation helps verify that equivalent formatting
    //    requests reuse existing objects.
    std::size_t styleCount() const {
        return styles_.size();
    }
};


// ============================================================================
// Context
// ============================================================================

// Character represents one character in the text editor.
// Notes:
//  - Stores its individual character value as extrinsic state.
//  - Holds a non-owning pointer to a shared Flyweight.
//  - Does not duplicate bold, italic, or color properties.
//  - Depends on the abstract Flyweight rather than CharacterStyle (DIP).
//  - Delegates rendering instead of implementing formatting itself.
//
// The factory must outlive Character because the raw pointer does not
// participate in ownership or lifetime management.
class Character {
private:
    char value_;
    const Flyweight* style_;
    
    // Reject null pointers to maintain the invariant that every Character
    // has an associated Flyweight.
    // Note:
    //  - A non-null pointer alone does not guarantee that the object is alive.
    static void validate_style(const Flyweight* style) {
        if (!style) {
            throw std::invalid_argument("Flyweight pointer must not be null!");
        }
    }

public:
    Character(char value, const Flyweight* style)
        : value_(value), style_(style)
    {
        validate_style(style_);
    }
    
    // Delegates rendering to the shared Flyweight.
    // Notes:
    //  - Character supplies its own value.
    //  - Highlighting is supplied dynamically by TextEditor.
    //  - Character does not need to know the concrete style implementation.
    void display(bool highlighted) const {
        style_->render(value_, highlighted);
    }
};


// ============================================================================
// Client / Text Editor
// ============================================================================

// TextEditor coordinates character storage, Flyweight reuse, and highlighting.
// Notes:
//  - Owns StyleFactory and the collection of Character objects.
//  - Obtains shared styles through StyleFactory rather than constructing
//    CharacterStyle objects directly.
//  - Maintains dynamic highlighting independently of shared styles.
//  - Provides the public operations used by main().
//
// Ownership and lifetime:
//  - styleFactory_ is declared before characters_.
//  - C++ destroys members in reverse declaration order.
//  - Characters are therefore destroyed before the factory.
//  - Copying and moving are disabled to keep the ownership model simple.
//
// Highlighting:
//  - A set stores only highlighted positions, which suits sparse highlighting.
//  - A position being present in the set means that it is highlighted.
//  - Highlighting does not create new Flyweights or modify existing ones.
class TextEditor {
private:
    // Owns all shared Flyweight objects.
    StyleFactory styleFactory_;
    
    // Stores character contexts containing non-owning Flyweight pointers.
    std::vector<Character> characters_;
    
    // Stores extrinsic highlighting information independently of the styles.
    std::set<std::size_t> highlightedPositions_;
    
public:
    // Default constructor.
    TextEditor() = default;
    
    // Disable copying.
    // Note:
    //  - Copying character pointers into another editor without reconnecting
    //    them to that editor's factory could violate ownership assumptions.
    TextEditor(const TextEditor&) = delete;
    TextEditor& operator=(const TextEditor&) = delete;
    
    // Disable moving.
    // Note: 
    //  - Moving could be supported with a suitable implementation, but is not
    //    required for this pattern-focused exercise.
    TextEditor(TextEditor&&) = delete;
    TextEditor& operator=(TextEditor&&) = delete;

    // Adds a character using a shared style obtained from the factory.
    // Notes:
    //  - Identical formatting requests reuse the same Flyweight.
    //  - The returned pointer is non-owning.
    //  - emplace_back constructs Character directly in the vector.
    void addCharacter(char value, bool bold, bool italic, std::string_view color) {
        auto ptr = styleFactory_.getStyle(bold, italic, color);
        characters_.emplace_back(value, ptr);
    }
    
    // Marks an existing character position as highlighted.
    // Notes:
    //  - Validation prevents highlighting positions outside the text.
    //  - set::insert avoids duplicate highlighted positions.
    //  - Neither Character nor CharacterStyle needs to be modified.
    void highlight(std::size_t position) {
        if (position >= characters_.size()) {
            throw std::out_of_range("Invalid position!");
        }
        
        highlightedPositions_.insert(position); 
    }
    
    // Removes highlighting without modifying the underlying shared style.
    // Note:
    //  - Removing a valid but non-highlighted position has no effect.
    void removeHighlight(std::size_t position) {
        if (position >= characters_.size()) {
            throw std::out_of_range("Invalid position!");
        }
        
        highlightedPositions_.erase(position); 
    }
    
    // Displays the text by combining character-specific information with
    // its current dynamic highlighting state.
    // Notes:
    //  - contains() checks whether a position is highlighted (C++20).
    //  - Character::display() forwards extrinsic state to Flyweight::render().
    //  - The const qualifier communicates that displaying does not change
    //    the editor's logical state.
    void display() const {
        for (std::size_t i = 0; i < characters_.size(); i++) {
            bool highlight = highlightedPositions_.contains(i);
            characters_[i].display(highlight);
        }
    }

    // Delegates to the factory's diagnostic operation while keeping the
    // factory and its cache private.
    std::size_t styleCount() const {
        return styleFactory_.styleCount();
    }
};


// ============================================================================
// Client / Demo
// ============================================================================

// Demonstrates the central behavior of the Flyweight pattern.
// Notes:
//  - Five Character objects use only two unique CharacterStyle objects.
//  - Four characters share the bold, black style.
//  - The final character uses an italic, blue style.
//  - Dynamic highlighting changes rendering without creating new styles.
//  - Assertions verify that the number of cached Flyweights remains constant.
//
// The assertions validate the cache size, not exact memory savings.
// assert() checks can be disabled when NDEBUG is defined.
int main() {
    TextEditor textEditor;
    
    // Four characters share the same intrinsic formatting.
    textEditor.addCharacter('H', true, false, "black");
    textEditor.addCharacter('E', true, false, "black");
    textEditor.addCharacter('L', true, false, "black");
    textEditor.addCharacter('L', true, false, "black");
    
    // Different formatting requires a second Flyweight instance.
    textEditor.addCharacter('O', false, true, "blue");
    
    // Initial display: no characters are highlighted.
    textEditor.display();
    
    // Five characters share exactly two cached Flyweight objects.
    assert(textEditor.styleCount() == 2);
    
    // Highlight E and the first L.
    // Note:
    //  - This changes only extrinsic rendering state.
    textEditor.highlight(1);
    textEditor.highlight(2);
    
    // Highlighting must not create additional Flyweights.
    assert(textEditor.styleCount() == 2);
    
    // The two L characters share a style, but only one is highlighted.
    textEditor.display();
    
    // Remove the highlight from E while keeping the first L highlighted.
    textEditor.removeHighlight(1);
    
    // The shared Flyweight count remains unchanged.
    assert(textEditor.styleCount() == 2);
    
    // Final display: only the first L remains highlighted.
    textEditor.display();
    
    return 0;
}
