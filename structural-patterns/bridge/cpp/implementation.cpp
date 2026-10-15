class Renderer {
public:
    virtual ~Renderer() = default;
    
    virtual void render(std::string_view shape_name) const = 0;
};


class VectorRenderer : public Renderer {
public:
    void render(std::string_view shape_name) const override {
        std::cout << "Vector rendering: " << shape_name << ".\n";
    }
};


class RasterRenderer : public Renderer {
public:
    void render(std::string_view shape_name) const override {
        std::cout << "Raster rendering: " << shape_name << ".\n";
    }
};


class AsciiRenderer : public Renderer {
public:
    void render(std::string_view shape_name) const override {
        std::cout << "Ascii rendering: " << shape_name << ".\n";
    }
};


class Shape {
protected:
    Renderer& renderer_;
    
public:
    explicit Shape(Renderer& renderer) 
        : renderer_(renderer)
    {
    }
    
    virtual ~Shape() = default;
    
    virtual void draw() const = 0;
};


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
