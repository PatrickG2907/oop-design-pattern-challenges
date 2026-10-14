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


class PizzaBuilder {
private:
    Size size_{Size::MEDIUM};
    CrustType crust_type_{CrustType::THICK};
    std::vector<std::string> toppings_;
    
public:
    PizzaBuilder() = default;
    
    PizzaBuilder& reset() {
        *this = PizzaBuilder{};
        return *this;
    }
    
    PizzaBuilder& set_size(Size size) {
        size_ = size;
        return *this;
    } 
    
    PizzaBuilder& set_crust(CrustType crust_type) {
        crust_type_ = crust_type;
        return *this;
    }
    
    PizzaBuilder& add_topping(std::string topping) {
        toppings_.push_back(std::move(topping));
        return *this;
    }
    
    Pizza build() const {
        return Pizza(size_, crust_type_, toppings_);
    }
};


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
