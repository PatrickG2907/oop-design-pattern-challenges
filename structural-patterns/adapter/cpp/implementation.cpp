// -----------------------------------------------------------------------------
// Architecture Overview
// -----------------------------------------------------------------------------
// This example demonstrates the Adapter pattern by integrating an incompatible
// payment gateway into an existing payment-processing interface.
//
// PaymentProcessor is the Target interface expected by the existing platform.
// Client code depends only on pay(amount_cents) returning bool.
//
// NewPaymentGateway is the Adaptee. Its API is incompatible because it exposes
// make_payment(total_cents) and may report success/failure using either bool or
// std::string.
//
// PaymentGatewayAdapter implements PaymentProcessor and wraps a
// NewPaymentGateway. It translates pay() into make_payment() and normalizes the
// gateway-specific result into the bool expected by the platform.
//
// The existing client therefore does not need to know about NewPaymentGateway,
// std::variant, "OK", or "Error".
//
// Monetary values are represented as integer cents using std::int64_t to avoid
// floating-point representation issues.
// -----------------------------------------------------------------------------


#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <variant>


// -----------------------------------------------------------------------------
// Target Interface
// -----------------------------------------------------------------------------

// Represents the interface already expected by the existing platform.
// Notes:
//  - The client works only with this abstraction, so integrating a new gateway
//    does not require modifying existing payment-processing logic.
//  - The virtual destructor supports safe polymorphic destruction.
class PaymentProcessor {
public:
    virtual ~PaymentProcessor() = default;
    
    virtual bool pay(std::int64_t amount_cents) = 0;
};


// -----------------------------------------------------------------------------
// Adaptee Result Type
// -----------------------------------------------------------------------------

// Models the inconsistent result format returned by the external gateway.
// Notes:
//  - std::variant is appropriate because the set of possible result types is
//    known and finite: either bool or std::string.
//  - A named alias keeps the gateway and adapter signatures readable.
using PaymentResult = std::variant<bool, std::string>;


// -----------------------------------------------------------------------------
// Adaptee
// -----------------------------------------------------------------------------

// Simulates a third-party gateway whose API does not match PaymentProcessor.
// Notes:
//  - The gateway deliberately returns different representations of success and
//    failure so the Adapter has something meaningful to normalize:
//      - true      -> success
//      - "OK"      -> success
//      - false     -> failure
//      - "Error"   -> failure
//  - The behavior is deterministic rather than random so every result case can
//    be tested reproducibly.
//  - Explicit std::string construction selects the string alternative of the
//    variant unambiguously. A string literal is a pointer and can otherwise
//    participate in conversions such as pointer-to-bool.
class NewPaymentGateway {
public:
    PaymentResult make_payment(std::int64_t total_cents) const {
        if (total_cents % 2 == 0) {
            if (total_cents > 100) {
                return true;
            } else {
                return std::string{"OK"};
            }
        } else {
            if (total_cents > 100) {
                return false;
            } else {
                return std::string{"Error"};
            }
        }
    }
};


// -----------------------------------------------------------------------------
// Adapter
// -----------------------------------------------------------------------------

// Adapts NewPaymentGateway to the PaymentProcessor interface expected by the
// existing platform.
// Notes:
//  - The adapter owns its gateway by value and delegates payment processing 
//    to it.
//  - The constructor is explicit because converting a gateway into an adapter 
//    is a meaningful operation and should not happen implicitly.
class PaymentGatewayAdapter : public PaymentProcessor {
private:
    NewPaymentGateway gateway_;
    
    // Converts the gateway-specific PaymentResult into the platform's bool 
    // result.
    // Notes:
    //  - std::get_if<T>() receives a pointer to the variant and returns:
    //      - a pointer to the contained T when that alternative is active
    //      - nullptr when the variant currently contains another type
    //  - If the result contains bool, that value is already normalized and 
    //    can be returned directly.
    //  - If the result contains std::string, only "OK" is treated as success; 
    //    any other string is treated as failure.
    //  - The function takes the variant by const reference because normalization 
    //    only reads the result and should not copy or modify it.
    static bool normalize_result(const PaymentResult& result) {
        if (
            const bool* bool_result = std::get_if<bool>(&result)
        ) {
            return *bool_result;
        }
        
        if (
            const std::string* string_result = 
                std::get_if<std::string>(&result)
        ) {
            // The comparison itself produces bool:
            // "OK" -> true, every other string -> false.
            return *string_result == "OK";
        }
        
        // Fail safely if no recognized alternative can be read.
        return false;
    }
    
public:
    explicit PaymentGatewayAdapter(NewPaymentGateway gateway) 
        : gateway_(std::move(gateway))
    {
    }

    // Implements the Target interface expected by the client.
    // Notes:
    //  - The Adapter delegates the actual payment to the gateway, receives its
    //    gateway-specific PaymentResult, then normalizes it before returning.
    //  - The client therefore always receives bool regardless of how the gateway
    //    represented its result.
    bool pay(std::int64_t amount_cents) override {
        const PaymentResult result = gateway_.make_payment(amount_cents);
        return normalize_result(result);
    }
};


// -----------------------------------------------------------------------------
// Client Logic
// -----------------------------------------------------------------------------

// Existing client logic depends only on PaymentProcessor.
// Notes:
//  - It knows nothing about:
//      - NewPaymentGateway
//      - PaymentGatewayAdapter
//      - PaymentResult
//      - std::variant
//      - gateway-specific strings such as "OK" or "Error"
//  - This separation is the main purpose of the Adapter pattern: incompatibility
//    is contained inside the adapter rather than leaking into existing client 
//    code.
void client_logic(
    PaymentProcessor& processor,
    std::int64_t amount_cents
) {
    const bool result = processor.pay(amount_cents);
    std::cout 
        << "Amount: "
        << amount_cents
        << ", "
        << "Result: "
        << (result ? "Success.\n" : "Error.\n");
}


// -----------------------------------------------------------------------------
// Demo / Entry Point
// -----------------------------------------------------------------------------

int main() {
    // Create the incompatible gateway inside its adapter.
    PaymentGatewayAdapter adapter{NewPaymentGateway{}};
    
    // Use the adapter through the existing client logic.
    // Notes:
    //  - The four test values exercise every simulated gateway result:
    //      - 200   -> true     -> Success
    //      - 2     -> "OK"     -> Success
    //      - 201   -> false    -> Error
    //      - 1     -> "Error"  -> Error
    //  - In all four cases, client_logic() receives only the normalized bool 
    //    result.
    client_logic(adapter, 200);
    client_logic(adapter, 2);
    client_logic(adapter, 201);
    client_logic(adapter, 1);
    
    return 0;
}
