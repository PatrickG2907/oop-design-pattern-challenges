#include <variant>
#include <vector>
#include <stdexcept>  // std::invalid_argument
#include <utility>    // std::move


struct Circle {
    int id{};         // Unique shape identifier
    double x{};       // Horizontal center position
    double y{};       // Vertical center position
    double radius{};  // Radius of the circle
};


struct Rectangle {
    int id{};	        // Unique shape identifier
    double x{};       // Horizontal position
    double y{};       // Vertical position
    double width{};   // Rectangle width
    double height{};  // Rectangle height    
};


using Shape = std::variant<Circle, Rectangle>;


int getShapeId(const Shape& shape) {
    auto visitor = [](const auto& value) {
        return value.id;
    };
        
    return std::visit(visitor, shape);
}


class Drawing {
private:
    std::vector<Shape> shapes_{};
    
    Shape* findShapeById(int id) {
        // Iterate over existing shapes.
        for (Shape& existingShape : shapes_) {
            // Return the address of the matching shape.
            if (id == getShapeId(existingShape)) {
                return &existingShape;
            }
        }
        // Return nullptr if not found.
        return nullptr;
    }
    
public:
    void addShape(Shape shape) {
        const int id = getShapeId(shape);
        
        if (findShapeById(id)) {
            throw std::invalid_argument("Shape ID already exists!");
        }
        
        shapes_.push_back(std::move(shape));
    }
    
    void moveShape(int id, double dx, double dy) {
        // Locate the shape.
        Shape* shape = findShapeById(id);
    
        // Validate that the shape exists.
        if (!shape) {
            throw std::invalid_argument("Shape not found!");
        }
    
        // Define a generic lambda that updates x and y.
        auto visitor = [dx, dy](auto& value) {
            value.x += dx;
            value.y += dy;
        };
        
        // Invoke the visitor using std::visit().
    }
};
