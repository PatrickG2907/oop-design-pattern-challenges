// -----------------------------------------------------------------------------
// Architecture Overview
// -----------------------------------------------------------------------------
// This example demonstrates the Abstract Factory pattern with two related
// product families: Light and Dark.
//
// Abstract products:
//  - Button
//  - Checkbox
//  - Slider
//
// Concrete product families:
//  - LightButton, LightCheckbox, LightSlider
//  - DarkButton, DarkCheckbox, DarkSlider
//
// UIFactory defines how a complete family of products is created.
// LightFactory and DarkFactory guarantee that all products created through
// one factory belong to the same theme.
//
// The client depends only on UIFactory and the abstract product types.
// It therefore works unchanged with either concrete product family.
//
// Factory methods return std::unique_ptr because each factory creates a new
// product and transfers exclusive ownership to the caller. The local smart
// pointers in client_logic() automatically destroy their products when they
// go out of scope.
//
// Abstract Factory is well suited for adding new product families.
// Adding a new product type, however, requires extending UIFactory and every
// concrete factory. Slider demonstrates this tradeoff in this example.
// -----------------------------------------------------------------------------


#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>


// -----------------------------------------------------------------------------
// Abstract Products
// -----------------------------------------------------------------------------

// Abstract product representing a button.
// Notes:
//  - Rendering varies between themes and is therefore polymorphic.
//  - click() is shared by all buttons, so the behavior lives in the base class
//    instead of being duplicated by every concrete product.
class Button {
public:
    virtual ~Button() = default;
    
    virtual void render() const = 0;
    
    void click() const {
        std::cout << "Button clicked.\n";
    }
};


// Abstract product representing a checkbox.
// Notes:
//  - The checked state and its manipulation are common to every checkbox, so
//    they are implemented once in the base class.
//  - checked_ remains private to preserve encapsulation. 
//  - Concrete checkbox implementations access the state through checked() 
//    rather than directly.
//  - Only rendering varies between themes.
class Checkbox {
private:
    bool checked_{false};
public:
    virtual ~Checkbox() = default;
    
    virtual void render() const = 0;
    
    void set_checked(bool checked) {
        checked_ = checked;
    }
    
    bool checked() const noexcept {
        return checked_;
    }
};


// Abstract product representing a slider.
// Notes:
//  - Slider owns the common value-management logic while concrete subclasses
//    provide theme-specific rendering.
//  - The valid range is identical for every Slider instance, so minimum_ and
//    maximum_ are static constexpr class constants rather than per-object state.
//  - set_value() validates the requested value before changing the object's 
//    state.
class Slider {
private:
    int value_{0};
    static constexpr int minimum_ = 0;
    static constexpr int maximum_ = 100;
public:
    virtual ~Slider() = default;
    
    virtual void render() const = 0;
    
    void set_value(int value) {
        if (value < minimum_ || value > maximum_) {
            throw std::invalid_argument(
                "Value must be within [" 
                    + std::to_string(minimum_) 
                    + ',' 
                    + std::to_string(maximum_)
                    + "]!");
        }
        value_ = value;
    }
    
    int value() const noexcept {
        return value_;
    }
};


// -----------------------------------------------------------------------------
// Light Product Family
// -----------------------------------------------------------------------------

// Concrete Light-theme products.
// Each class implements only the behavior that actually varies by theme;
// common behavior remains in the corresponding abstract product.

// Light-theme concrete button.
class LightButton : public Button {
public:
    void render() const override {
        std::cout << "Render light button." << '\n';
    }
};


// Light-theme concrete checkbox.
class LightCheckbox : public Checkbox {
public:
    void render() const override {
        std::cout 
            << "Render light checkbox: state [" 
            << (checked() ? "checked" : "unchecked")
            << "]."
            << '\n';
    }
};


// Light-theme concrete slider.
class LightSlider : public Slider {
public:
    void render() const override {
        std::cout 
            << "Render light slider: value ["
            << value()
            << "]\n";
    }
};


// -----------------------------------------------------------------------------
// Dark Product Family
// -----------------------------------------------------------------------------

// Concrete Dark-theme products.
// Together with the Light products, these classes demonstrate two complete
// and interchangeable product families.

// Dark-theme concrete button.
class DarkButton : public Button {
public:
    void render() const override {
        std::cout << "Render dark button." << '\n';
    }
};


// Dark-theme concrete checkbox.
class DarkCheckbox : public Checkbox {
public:
    void render() const override {
        std::cout 
            << "Render dark checkbox: state [" 
            << (checked() ? "checked" : "unchecked")
            << "]."
            << '\n';
    }
};


// Dark-theme concrete slider.
class DarkSlider : public Slider {
public:
    void render() const override {
        std::cout 
            << "Render dark slider: value ["
            << value()
            << "]\n";
    }
};


// -----------------------------------------------------------------------------
// Abstract Factory
// -----------------------------------------------------------------------------

// Abstract Factory for creating a complete family of related UI products.
//
// The client depends on this abstraction instead of LightFactory or
// DarkFactory, so it does not need to know which concrete classes are created.
//
// Each factory method returns std::unique_ptr because the factory creates a
// new object and transfers exclusive ownership to the caller.
//
// The methods are const because creating these stateless products does not
// logically modify the factory.
//
// Abstract Factory makes adding a new family straightforward: a new factory
// can provide implementations for all existing product types.
//
// Adding a new product type is less flexible: adding Slider required adding
// create_slider() here and implementing it in every concrete factory.
class UIFactory {
public:
    virtual ~UIFactory() = default;
    
    virtual std::unique_ptr<Button> create_button() const = 0;
    
    virtual std::unique_ptr<Checkbox> create_checkbox() const = 0;
    
    virtual std::unique_ptr<Slider> create_slider() const = 0;
};


// -----------------------------------------------------------------------------
// Concrete Factories
// -----------------------------------------------------------------------------

// Concrete factory for the Light product family.
// Note:
//  - Every creation method returns the Light implementation of its corresponding
//    abstract product, keeping the generated family consistent.
class LightFactory : public UIFactory {
public:
    std::unique_ptr<Button> create_button() const override {
        return std::make_unique<LightButton>();
    }
    
    std::unique_ptr<Checkbox> create_checkbox() const override {
        return std::make_unique<LightCheckbox>();
    }
    
    std::unique_ptr<Slider> create_slider() const override {
        return std::make_unique<LightSlider>();
    }
};


// Concrete factory for the Dark product family.
// Note:
//  - The public interface is identical to LightFactory; only the concrete
//    products selected by the factory differ.
class DarkFactory : public UIFactory {
public:
    std::unique_ptr<Button> create_button() const override {
        return std::make_unique<DarkButton>();
    }
    
    std::unique_ptr<Checkbox> create_checkbox() const override {
        return std::make_unique<DarkCheckbox>();
    }
    
    std::unique_ptr<Slider> create_slider() const override {
        return std::make_unique<DarkSlider>();
    }
};


// -----------------------------------------------------------------------------
// Client Logic
// -----------------------------------------------------------------------------

// Creates and uses a complete UI family through abstractions only.
//
// The factory is received by const reference because client_logic() borrows
// an existing factory, does not own it, and does not modify it.
//
// The returned unique_ptrs become the owners of the newly created products.
// When client_logic() finishes, these local smart pointers go out of scope
// and automatically destroy their products.
//
// No Light/Dark classes or theme-specific conditionals appear here.
// Dynamic dispatch selects the correct concrete render() implementations,
// allowing this function to work unchanged with any UIFactory implementation.
void client_logic(const UIFactory& factory) {
    auto button_ptr = factory.create_button();
    auto checkbox_ptr = factory.create_checkbox();
    auto slider_ptr = factory.create_slider();
    
    button_ptr->render();
    button_ptr->click();
    
    checkbox_ptr->render();
    checkbox_ptr->set_checked(true);
    checkbox_ptr->render();
    
    slider_ptr->render();
    slider_ptr->set_value(100);
    slider_ptr->render();
}


// -----------------------------------------------------------------------------
// Demo / Entry Point
// -----------------------------------------------------------------------------

// Demonstrates that the same client logic can operate on two different
// product families simply by supplying a different concrete factory.
//
// The factories themselves can live on the stack because main() owns them
// directly and no dynamic ownership is required.
int main() {
    LightFactory light_factory;
    DarkFactory dark_factory;
    
    client_logic(light_factory);
    client_logic(dark_factory);
    return 0;
}
