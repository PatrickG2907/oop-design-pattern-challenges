from abc import ABC, abstractmethod

# =====================================================
# Renderer Abstraction (generic, shape-agnostic)
# =====================================================
class Renderer(ABC):
    @abstractmethod
    def draw(self, shape: str, **data) -> None:
        """
        Render a shape described by:
        - shape: a string identifier ("circle", "square", "triangle", etc.)
        - data: shape-specific parameters
        """
        pass

# =====================================================
# Concrete Renderers
# =====================================================
class VectorRenderer(Renderer):
    def draw(self, shape: str, **data) -> None:
        print(f"Vector: Drawing {shape} with {data}")

class RasterRenderer(Renderer):
    def draw(self, shape: str, **data) -> None:
        print(f"Raster: Drawing pixels for {shape} with {data}")

# =====================================================
# ASCII Renderer using dispatch table instead of if-else
# =====================================================
class ASCIIRenderer(Renderer):
    def __init__(self):
        # Map shape names to drawing functions
        self._draw_methods = {
            "circle": self._draw_circle,
            "square": self._draw_square,
            "triangle": self._draw_triangle,
        }

    def draw(self, shape: str, **data) -> None:
        # Lookup the shape in the dispatch table
        draw_func = self._draw_methods.get(shape)
        if draw_func:
            draw_func(**data)  # Call the appropriate drawing method
        else:
            print(f"(No ASCII implementation for {shape})")

    # Individual drawing methods
    def _draw_circle(self, radius: int):
        print("ASCII: Circle")
        for y in range(-radius, radius + 1):
            for x in range(-radius, radius + 1):
                print("*" if x*x + y*y <= radius*radius else " ", end="")
            print()

    def _draw_square(self, side: int):
        print("ASCII: Square")
        for _ in range(side):
            print("*" * side)

    def _draw_triangle(self, base: int, height: int):
        print("ASCII: Triangle")
        for i in range(1, height + 1):
            width = base * i // height
            print("*" * width)

# =====================================================
# Shape Abstraction (Bridge)
# =====================================================
class Shape(ABC):
    def __init__(self, renderer: Renderer):
        self.renderer = renderer  # Bridge to renderer

    @abstractmethod
    def draw(self) -> None:
        pass

# =====================================================
# Concrete Shapes (self-describing)
# =====================================================
class Circle(Shape):
    def __init__(self, radius: int, renderer: Renderer):
        super().__init__(renderer)
        self.radius = radius

    def draw(self) -> None:
        # Shape provides its identity and required parameters
        self.renderer.draw("circle", radius=self.radius)

class Square(Shape):
    def __init__(self, side: int, renderer: Renderer):
        super().__init__(renderer)
        self.side = side

    def draw(self) -> None:
        self.renderer.draw("square", side=self.side)

class Triangle(Shape):
    def __init__(self, base: int, height: int, renderer: Renderer):
        super().__init__(renderer)
        self.base = base
        self.height = height

    def draw(self) -> None:
        self.renderer.draw("triangle", base=self.base, height=self.height)

# =====================================================
# Client Code / Demo
# =====================================================
if __name__ == "__main__":
    # Instantiate renderers
    vector = VectorRenderer()
    raster = RasterRenderer()
    ascii_renderer = ASCIIRenderer()

    # Mix-and-match shapes with renderers
    shapes = [
        Circle(3, vector),
        Square(4, vector),
        Triangle(6, 4, vector),
        Circle(3, raster),
        Square(4, raster),
        Triangle(6, 4, raster),
        Circle(3, ascii_renderer),
        Square(4, ascii_renderer),
        Triangle(6, 4, ascii_renderer),
    ]

    # Draw all shapes
    for shape in shapes:
        shape.draw()
        print("-" * 30)
