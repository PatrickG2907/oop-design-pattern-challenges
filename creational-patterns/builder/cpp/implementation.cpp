// -----------------------------------------------------------------------------
// Architecture Overview
// -----------------------------------------------------------------------------
// This example demonstrates the Builder pattern by constructing Pizza objects
// step by step.
//
// Pizza is the final product. It owns its size, crust type, and toppings.
//
// PizzaBuilder stores the configuration while a pizza is being assembled.
// Individual steps can be called in any useful combination, and build()
// produces the finished Pizza.
//
// The builder starts with valid defaults:
//  - Size: Medium
//  - Crust: Thick
//  - Toppings: none
//
// Because the builder always starts in a valid state, callers may skip
// configuration steps and still build a valid Pizza. This demonstrates the
// optional-step requirement of the challenge.
//
// Director contains predefined construction recipes such as Margherita and
// Pepperoni. It knows which builder steps to execute, while PizzaBuilder knows
// how those steps modify the construction state.
//
// Unlike Factory Method or Abstract Factory, Builder is not primarily about
// selecting which concrete product type to instantiate. The product is already
// known to be a Pizza; Builder controls how that product is configured and
// assembled.
//
// PizzaBuilder is intentionally a concrete builder. An abstract builder
// hierarchy would add unnecessary complexity because this example has only one
// product representation.
// -----------------------------------------------------------------------------


#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// -----------------------------------------------------------------------------
// Pizza Configuration Types
// -----------------------------------------------------------------------------

// Scoped enums model the fixed sets of valid pizza sizes and crust types.
// Note:
//  - enum class keeps the values scoped and prevents implicit integer conversion.
enum class Size {
    SMALL,
    MEDIUM,
    LARGE
};

enum class CrustType {
    THIN,
    THICK,
    STUFFED
};


// Convert strongly typed enum values into readable text for display.
// Notes:
//  - std::string_view is sufficient because the functions return fixed string
//    literals and therefore do not need to allocate or own strings.
//  - The functions are constexpr because the mapping can be evaluated at compile
//    time, and noexcept because the conversion cannot throw.
//  - No default case is used so the compiler can warn if a new enumerator is
//    later added but not handled. The final "unknown" return still provides a
//    fallback for an invalid underlying enum value.
constexpr std::string_view to_string(Size size) noexcept {
    switch(size) {
        case Size::SMALL:
            return "small";
        case Size::MEDIUM:
            return "medium";
        case Size::LARGE:
            return "large";
    }
    return "unknown";
}


constexpr std::string_view to_string(CrustType crust_type) noexcept {
    switch(crust_type) {
        case CrustType::THIN:
            return "thin";
        case CrustType::THICK:
            return "thick";
        case CrustType::STUFFED:
            return "stuffed";
    }
    return "unknown";
}


// -----------------------------------------------------------------------------
// Product
// -----------------------------------------------------------------------------

// Represents the finished product produced by PizzaBuilder.
// Notes: 
//  - Pizza owns all of its configuration. 
//  - Size and CrustType are small value types and are copied. 
//  - The toppings collection is received by value and moved into the Pizza 
//    because the finished product takes ownership of it.
class Pizza {
private:
    Size size_;
    CrustType crust_type_;
    std::vector<std::string> toppings_;

public:
    Pizza(
        Size size, 
        CrustType crust_type, 
        std::vector<std::string> toppings
    ) : size_(size),
        crust_type_(crust_type),
        toppings_(std::move(toppings))
    {
    }
    
    // Displays the complete pizza configuration without modifying the object.
    // Note:
    //  - Empty toppings are handled explicitly so a pizza with no toppings 
    //    remains a valid and clearly represented result.
    void display() const {
        std::cout 
            << "Configured pizza:\n"
            << "\tSize: "
            << to_string(size_)
            << '\n'
            << "\tCrustType: "
            << to_string(crust_type_)
            << '\n'
            << "\tToppings: ";
            
        if (toppings_.empty()) {
            std::cout << "None\n";
        } else {
            std::cout << '\n';
            for (const std::string& topping : toppings_) {
                std::cout << "\t\t- " << topping << '\n';
            }
        }
    }
};


// -----------------------------------------------------------------------------
// Builder
// -----------------------------------------------------------------------------

// Builds a Pizza incrementally.
//
// The builder itself stores construction state rather than creating a new
// Pizza after every configuration step.
//
// Valid in-class defaults make every builder immediately buildable. This means
// callers may skip size, crust, or topping steps and still obtain a valid
// Pizza.
//
// Builder operations return PizzaBuilder& so several construction steps can be
// chained while continuing to modify the same builder object.

class PizzaBuilder {
private:
    // The defaults are the single source of truth for a fresh builder state.
    // Note:
    //  - toppings_ is default-constructed as an empty collection.
    Size size_{Size::MEDIUM};
    CrustType crust_type_{CrustType::THICK};
    std::vector<std::string> toppings_;
    
public:
    PizzaBuilder() = default;
    
    // Restores exactly the same state as a freshly default-constructed builder.
    // Notes:
    //  - Assigning PizzaBuilder{} avoids duplicating the default size and crust 
    //    values inside reset(), keeping one source of truth for the builder 
    //    defaults.
    //  - This approach is appropriate while PizzaBuilder remains an ordinary
    //    assignable value type.
    PizzaBuilder& reset() {
        *this = PizzaBuilder{};
        return *this;
    }
    
    // Configuration methods modify one part of the current construction state and
    // return *this so the fluent interface can continue on the same builder.
    PizzaBuilder& set_size(Size size) {
        size_ = size;
        return *this;
    } 
    
    PizzaBuilder& set_crust(CrustType crust_type) {
        crust_type_ = crust_type;
        return *this;
    }
    
    // Takes the topping by value because the builder will own the stored string.
    // Note:
    //  - Moving it into the vector avoids another copy of that local value.
    PizzaBuilder& add_topping(std::string topping) {
        toppings_.push_back(std::move(topping));
        return *this;
    }
    
    // Creates the finished Pizza without consuming or resetting the builder.
    // Notes:
    //  - build() is const because construction of the product does not modify 
    //    the builder's configuration. This makes the builder reusable and allows 
    //    the same configuration to produce multiple Pizza objects.
    //  - toppings_ is copied here so the builder retains its toppings; Pizza then
    //    moves that copied vector into its own storage.
    Pizza build() const {
        return Pizza(size_, crust_type_, toppings_);
    }
};


// -----------------------------------------------------------------------------
// Director
// -----------------------------------------------------------------------------

// Encapsulates predefined construction sequences.
// Notes:
//  - The Director knows the recipe -- which size, crust, and toppings to request.
//  - PizzaBuilder remains responsible for applying those individual construction
//    steps and producing the final Pizza.
//  - The Director borrows an existing PizzaBuilder by reference. It neither owns
//    the builder nor stores it.
//  - Each recipe resets the builder first so previous customizations or toppings
//    cannot leak into a predefined pizza. This makes each recipe deterministic.
//  - Director methods are const because the Director itself is stateless; only
//    the supplied builder is modified.
class Director {
public:
    Pizza make_margherita(PizzaBuilder& pizza_builder) const {
        pizza_builder.reset();
        pizza_builder.set_size(Size::SMALL);
        pizza_builder.set_crust(CrustType::THIN);
        pizza_builder.add_topping("tomato");
        pizza_builder.add_topping("mozzarella");
        return pizza_builder.build();
    }
    
    Pizza make_pepperoni(PizzaBuilder& pizza_builder) const {
        pizza_builder.reset();
        pizza_builder.set_size(Size::LARGE);
        pizza_builder.set_crust(CrustType::STUFFED);
        pizza_builder.add_topping("tomato");
        pizza_builder.add_topping("mozzarella");
        pizza_builder.add_topping("pepperoni");
        return pizza_builder.build();
    }
};


// -----------------------------------------------------------------------------
// Demo / Entry Point
// -----------------------------------------------------------------------------

// Demonstrates both common ways to use the Builder pattern:
//  1. The Director uses the builder to produce predefined recipes.
//  2. A customer uses PizzaBuilder directly for a custom pizza.
// Notes:
//  - The same builder is reused throughout the demo. 
//  - Director recipes reset the builder before construction, while the custom 
//    example resets it explicitly.
//  - The custom pizza deliberately configures only its size. Crust therefore uses
//    the builder default and no toppings are added, demonstrating that optional
//    construction steps can be skipped while still producing a valid Pizza.
int main() {
    PizzaBuilder pizza_builder;
    Director director;
    
    auto margherita = director.make_margherita(pizza_builder);
    margherita.display();
    
    auto pepperoni = director.make_pepperoni(pizza_builder);
    pepperoni.display();
    
    pizza_builder.reset();
    pizza_builder.set_size(Size::SMALL);
    auto custom_pizza = pizza_builder.build();
    custom_pizza.display();
    
    return 0;
}
