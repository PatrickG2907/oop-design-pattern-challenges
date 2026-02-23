from __future__ import annotations
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List
import copy

# =========================
# Shape Abstraction
# =========================
@dataclass
class Shape(ABC):
    x: int
    y: int

    def move(self, dx: int, dy: int) -> None:
        self.x += dx
        self.y += dy

    @abstractmethod
    def deep_copy(self) -> Shape:
        pass

    @abstractmethod
    def accept(self, visitor: ShapeVisitor) -> str:
        pass

# =========================
# Concrete Shapes
# =========================
@dataclass
class Circle(Shape):
    radius: int

    def __post_init__(self) -> None:
        if self.radius <= 0:
            raise ValueError("Radius must be positive.")

    def deep_copy(self) -> Circle:
        return copy.deepcopy(self)

    def accept(self, visitor: ShapeVisitor) -> str:
        return visitor.visit_circle(self)

@dataclass
class Rectangle(Shape):
    width: int
    height: int

    def __post_init__(self) -> None:
        if self.width <= 0 or self.height <= 0:
            raise ValueError("Width and height must be positive.")

    def deep_copy(self) -> Rectangle:
        return copy.deepcopy(self)

    def accept(self, visitor: ShapeVisitor) -> str:
        return visitor.visit_rectangle(self)

# =========================
# Composite Pattern
# =========================
class ShapeGroup(Shape):
    def __init__(self, x: int = 0, y: int = 0) -> None:
        super().__init__(x, y)
        self._children: List[Shape] = []

    def add(self, shape: Shape) -> None:
        self._children.append(shape)

    def remove(self, shape: Shape) -> None:
        if shape not in self._children:
            raise ValueError("Cannot remove shape: not part of this group.")
        self._children.remove(shape)

    @property
    def children(self) -> List[Shape]:
        return self._children

    def move(self, dx: int, dy: int) -> None:
        super().move(dx, dy)
        for shape in self._children:
            shape.move(dx, dy)

    def deep_copy(self) -> ShapeGroup:
        group_copy = ShapeGroup(self.x, self.y)
        for shape in self._children:
            group_copy.add(shape.deep_copy())
        return group_copy

    def accept(self, visitor: ShapeVisitor) -> str:
        return visitor.visit_group(self)

# =========================
# Visitor Interface
# =========================
class ShapeVisitor(ABC):
    @abstractmethod
    def visit_circle(self, circle: Circle) -> str:
        pass

    @abstractmethod
    def visit_rectangle(self, rectangle: Rectangle) -> str:
        pass

    @abstractmethod
    def visit_group(self, group: ShapeGroup) -> str:
        pass

# =========================
# Concrete Visitor: Printer
# =========================
class ShapePrinterVisitor(ShapeVisitor):
    def visit_circle(self, circle: Circle) -> str:
        return f"Circle(x={circle.x}, y={circle.y}, r={circle.radius})"

    def visit_rectangle(self, rectangle: Rectangle) -> str:
        return f"Rectangle(x={rectangle.x}, y={rectangle.y}, w={rectangle.width}, h={rectangle.height})"

    def visit_group(self, group: ShapeGroup) -> str:
        children_str = ", ".join(child.accept(self) for child in group.children)
        return f"ShapeGroup([{children_str}])"

# =========================
# Abstract Memento
# =========================
class AbstractDrawingMemento(ABC):
    @property
    @abstractmethod
    def state(self) -> List[Shape]:
        pass

# =========================
# Concrete Memento
# =========================
class DrawingMemento(AbstractDrawingMemento):
    def __init__(self, shapes_snapshot: List[Shape]) -> None:
        # Store a deep copy to prevent external mutation
        self._state = [shape.deep_copy() for shape in shapes_snapshot]

    @property
    def state(self) -> List[Shape]:
        return [shape.deep_copy() for shape in self._state]

# =========================
# Drawing Interface
# =========================
class DrawingInterface(ABC):
    @abstractmethod
    def save(self) -> AbstractDrawingMemento:
        pass

    @abstractmethod
    def restore(self, memento: AbstractDrawingMemento) -> None:
        pass

# =========================
# Concrete Drawing
# =========================
class Drawing(DrawingInterface):
    def __init__(self) -> None:
        self._shapes: List[Shape] = []

    def add_shape(self, shape: Shape) -> None:
        self._shapes.append(shape)

    def remove_shape(self, shape: Shape) -> None:
        if shape not in self._shapes:
            raise ValueError("Cannot remove shape: not part of this drawing.")
        self._shapes.remove(shape)

    def move_shape(self, shape: Shape, dx: int, dy: int) -> None:
        shape.move(dx, dy)

    def save(self) -> AbstractDrawingMemento:
        return DrawingMemento(self._shapes)

    def restore(self, memento: AbstractDrawingMemento) -> None:
        self._shapes = memento.state

    def accept(self, visitor: ShapeVisitor) -> str:
        children_str = ", ".join(shape.accept(visitor) for shape in self._shapes)
        return f"Drawing([{children_str}])"

# =========================
# Caretaker (Undo/Redo)
# =========================
class HistoryManager:
    def __init__(self) -> None:
        self._undo_stack: List[AbstractDrawingMemento] = []
        self._redo_stack: List[AbstractDrawingMemento] = []

    def save(self, drawing: DrawingInterface) -> None:
        self._undo_stack.append(drawing.save())
        self._redo_stack.clear()

    def undo(self, drawing: DrawingInterface) -> None:
        if not self._undo_stack:
            print("Undo stack empty")
            return
        self._redo_stack.append(drawing.save())
        memento = self._undo_stack.pop()
        drawing.restore(memento)

    def redo(self, drawing: DrawingInterface) -> None:
        if not self._redo_stack:
            print("Redo stack empty")
            return
        self._undo_stack.append(drawing.save())
        memento = self._redo_stack.pop()
        drawing.restore(memento)

# =========================
# Example Usage
# =========================
if __name__ == "__main__":
    # Initialize
    drawing = Drawing()
    history = HistoryManager()
    printer = ShapePrinterVisitor()

    # --- Create shapes ---
    circle1 = Circle(x=10, y=10, radius=5)
    rectangle1 = Rectangle(x=20, y=20, width=10, height=5)

    # --- Group shapes ---
    group1 = ShapeGroup()
    group1.add(circle1)
    group1.add(rectangle1)

    # --- Add group to drawing ---
    drawing.add_shape(group1)
    print("Initial Drawing:")
    print(drawing.accept(printer))

    # --- Save state ---
    history.save(drawing)

    # --- Move the group ---
    drawing.move_shape(group1, dx=5, dy=5)
    print("\nAfter Moving Group by (5,5):")
    print(drawing.accept(printer))

    # --- Save state again ---
    history.save(drawing)

    # --- Add another shape ---
    circle2 = Circle(x=50, y=50, radius=8)
    drawing.add_shape(circle2)
    print("\nAfter Adding a New Circle:")
    print(drawing.accept(printer))

    # --- Undo last action (adding circle2) ---
    history.undo(drawing)
    print("\nAfter Undo (removes last circle):")
    print(drawing.accept(printer))

    # --- Undo move of group ---
    history.undo(drawing)
    print("\nAfter Undo (moves group back):")
    print(drawing.accept(printer))

    # --- Redo move of group ---
    history.redo(drawing)
    print("\nAfter Redo (moves group again):")
    print(drawing.accept(printer))

    # --- Redo adding the circle2 ---
    history.redo(drawing)
    print("\nAfter Redo (adds circle2 again):")
    print(drawing.accept(printer))
