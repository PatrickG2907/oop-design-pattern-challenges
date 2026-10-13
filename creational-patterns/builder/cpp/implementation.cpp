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
            << size_
            << '\n'
            << "\tCrustType: "
            << crust_type_
            << '\n'
            << "\tToppings: "
            << (!toppings_.empty() ? toppings : "None")
        << std::endl;
    }
};
