// -----------------------------------------------------------------------------
// Architecture Overview
// -----------------------------------------------------------------------------

// Bridge Pattern
//
// Separates two independently varying dimensions:
//
//  - Abstraction hierarchy: Shape -> Circle, Square, Triangle
//  - Implementor hierarchy: Renderer -> Vector, Raster, ASCII
//
// Shape holds a reference to the abstract Renderer. This relationship is
// the bridge between the two hierarchies.
//
// As a result:
//  - new shapes can be added without changing existing renderers
//  - new renderers can be added without changing existing shapes
//  - we avoid subclasses for every combination such as VectorCircle,
//   RasterCircle, VectorSquare, etc.
//
// Renderer is deliberately independent of concrete Shape classes. Concrete 
// shapes identify what should be drawn, while the selected renderer determines 
// how.



#include <iostream>
#include <string_view>


// -----------------------------------------------------------------------------
// Implementor hierarchy
// -----------------------------------------------------------------------------

// Abstract Implementor.
// Note:
//  - Defines the rendering operation used by the Shape hierarchy.
class Renderer {
public:
    virtual ~Renderer() = default;
    
    virtual void render(std::string_view shape_name) const = 0;
};


// Concrete Implementor: vector rendering strategy.
class VectorRenderer : public Renderer {
public:
    void render(std::string_view shape_name) const override {
        std::cout << "Vector rendering: " << shape_name << ".\n";
    }
};


// Concrete Implementor: raster rendering strategy.
class RasterRenderer : public Renderer {
public:
    void render(std::string_view shape_name) const override {
        std::cout << "Raster rendering: " << shape_name << ".\n";
    }
};


// Concrete Implementor: ASCII rendering strategy.
// Note:
//  - New rendering strategy added without changing any Shape class.
class AsciiRenderer : public Renderer {
public:
    void render(std::string_view shape_name) const override {
        std::cout << "Ascii rendering: " << shape_name << ".\n";
    }
};


// -----------------------------------------------------------------------------
// Abstraction hierarchy
// -----------------------------------------------------------------------------

// Abstraction.
// Notes:
//  - renderer_ is the bridge connecting the Shape hierarchy to the
//    Renderer hierarchy.
//  - renderer_ is a non-owning reference: Shape does not manage the Renderer
//    lifetime, so the referenced Renderer must outlive the Shape.
//  - It is protected because refined abstractions delegate directly through it.
class Shape {
protected:
    Renderer& renderer_;
    
public:
    // The base constructor binds the renderer reference.
    // Note:
    //  - Derived shapes must construct this Shape base subobject.
    explicit Shape(Renderer& renderer) 
        : renderer_(renderer)
    {
    }
    
    virtual ~Shape() = default;
    
    virtual void draw() const = 0;
};


// Refined Abstraction.
// Note:
//  - Knows what is being drawn, but delegates how it is rendered.
class Circle : public Shape {
public:
    explicit Circle(Renderer& renderer) 
        : Shape(renderer)
    {
    }
    
    void draw() const override {
        renderer_.render("Circle");
    }
};


// Refined Abstraction using the same Renderer interface.
class Square : public Shape {
public:
    explicit Square(Renderer& renderer) 
        : Shape(renderer)
    {
    }
    
    void draw() const override {
        renderer_.render("Square");
    }
};


// New shape added without modifying any Renderer implementation.
class Triangle : public Shape {
public:
    explicit Triangle(Renderer& renderer) 
        : Shape(renderer)
    {
    }
    
    void draw() const override {
        renderer_.render("Triangle");
    }
};


// -----------------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------------

int main() {
    // Renderers are created before the Shapes because Shapes only borrow
    // references to them. Each Renderer must outlive every Shape using it.
    VectorRenderer vector_renderer;
    RasterRenderer raster_renderer;
    AsciiRenderer ascii_renderer;
    
    // Demonstrate that both dimensions can be combined freely:
    // every Shape can use every Renderer.
    // Note:
    //  - The 3 x 3 matrix avoids needing classes such as VectorCircle,
    //    RasterSquare, AsciiTriangle, etc.
    Circle circle_vector(vector_renderer);
    Circle circle_raster(raster_renderer);
    Circle circle_ascii(ascii_renderer);
    
    Square square_vector(vector_renderer);
    Square square_raster(raster_renderer);
    Square square_ascii(ascii_renderer);
    
    Triangle triangle_vector(vector_renderer);
    Triangle triangle_raster(raster_renderer);
    Triangle triangle_ascii(ascii_renderer);
    
    circle_vector.draw();
    circle_raster.draw();
    circle_ascii.draw();
    
    square_vector.draw();
    square_raster.draw();
    square_ascii.draw();
    
    triangle_vector.draw();
    triangle_raster.draw();
    triangle_ascii.draw();
    
    return 0;
}
