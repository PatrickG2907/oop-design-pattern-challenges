// ============================================================================
// Architecture Overview
// ============================================================================
//
//  - Notification defines the abstract Product interface shared by all
//    notification types.
//  - EmailNotification, PushNotification, and WhatsAppNotification are
//    Concrete Products that implement their own send() behavior.
//  - NotificationCreator defines the abstract Creator interface and declares
//    the Factory Method create_notification().
//  - Each Concrete Creator overrides create_notification() and instantiates
//    one corresponding Concrete Product.
//  - Products are returned as std::unique_ptr<Notification>, which transfers
//    exclusive ownership to the caller while preserving polymorphism.
//  - client_logic() depends only on the NotificationCreator and Notification
//    abstractions, so it does not need to know which concrete product is created.
//  - main() stores heterogeneous creators polymorphically through
//    std::unique_ptr<NotificationCreator> and reuses the same client logic
//    for every creator.
//  - Adding WhatsApp support requires only a new Concrete Product and a new
//    Concrete Creator; existing creator and client logic remain unchanged.
//
// Patterns / Concepts:
//  - Factory Method Pattern: Concrete Creators override a factory method to
//    decide which Concrete Product to instantiate.
//  - Polymorphism: creators and notifications are used through their abstract
//    base interfaces.
//  - Ownership: std::unique_ptr expresses exclusive ownership of dynamically
//    created creators and products.
//  - Extensibility: new notification types can be added by extending the
//    Product and Creator hierarchies without modifying existing implementations.


// ============================================================================
// Includes
// ============================================================================

#include <iostream>
#include <memory>
#include <string_view>
#include <vector>


// ============================================================================
// Product Interface
// ============================================================================

// Abstract Product interface for all notification types.
// Notes:
//  - Concrete notifications implement their own send() behavior.
//  - send() accepts a non-owning std::string_view because the message
//    is only needed for the duration of the call.
//  - The virtual destructor enables safe destruction through Notification 
//    pointers.
class Notification {
public:
    virtual ~Notification() = default;
    
    // Send the supplied message using the concrete notification mechanism.
    virtual void send(std::string_view message) const = 0;
};


// ============================================================================
// Concrete Products
// ============================================================================

// Concrete Product that sends notifications through email.
class EmailNotification : public Notification {
public:
    // Implement email-specific notification behavior.
    void send(std::string_view message) const override {
        std::cout 
            << "Sending email notification: " 
            << message 
            << ".\n";
    }
};


// Concrete Product that sends push notifications.
class PushNotification : public Notification {
public:
    // Implement push-specific notification behavior.
    void send(std::string_view message) const override {
        std::cout 
            << "Sending push notification: " 
            << message 
            << ".\n";
    }
};


// Concrete Product added as an extension without modifying existing products
// or creators.
class WhatsAppNotification : public Notification {
public:
    // Implement WhatsApp-specific notification behavior.
    void send(std::string_view message) const override {
        std::cout 
            << "Sending WhatsApp notification: " 
            << message 
            << ".\n";
    }
};


// ============================================================================
// Creator Interface
// ============================================================================

// Abstract Creator that defines the Factory Method.
// Notes:
//  - Concrete creators decide which Notification implementation to 
//    instantiate.
//  - The returned std::unique_ptr transfers ownership of the new product
//    to the caller.
//  - Returning the abstract Notification type keeps client code independent
//    of concrete notification classes.
class NotificationCreator {
public:
    virtual ~NotificationCreator() = default;
    
    // Create and return one owned Notification product.
    virtual std::unique_ptr<Notification> create_notification() const = 0;
};


// ============================================================================
// Concrete Creators
// ============================================================================

// Concrete Creator responsible for producing EmailNotification objects.
class EmailNotificationCreator : public NotificationCreator {
public:
    // Create an EmailNotification and expose it through the Product 
    // abstraction.
    std::unique_ptr<Notification> create_notification() const override {
        return std::make_unique<EmailNotification>();
    }    
};


// Concrete Creator responsible for producing PushNotification objects.
class PushNotificationCreator : public NotificationCreator {
public:
    // Create a PushNotification and expose it through the Product 
    // abstraction.
    std::unique_ptr<Notification> create_notification() const override {
        return std::make_unique<PushNotification>();
    }    
};


// Concrete Creator added for the WhatsApp extension.
// Note:
//  - Existing creator logic does not need to change.
class WhatsAppNotificationCreator : public NotificationCreator {
public:
    // Create a WhatsAppNotification and expose it through the Product 
    // abstraction.
    std::unique_ptr<Notification> create_notification() const override {
        return std::make_unique<WhatsAppNotification>();
    }    
};


// ============================================================================
// Client Logic
// ============================================================================

// Client logic that works only with the Creator and Product abstractions.
// Notes:
//  - The creator is borrowed by const reference; client_logic() does not
//    own it.
//  - create_notification() returns an owned polymorphic Notification.
//  - The client does not need to know which concrete notification was 
//    created.
//  - Virtual dispatch selects the correct concrete send() implementation.
void client_logic(const NotificationCreator& creator) {
    auto notification = creator.create_notification();
    notification->send("Dummy Message");
}


// ============================================================================
// Demo / Entry Point
// ============================================================================

// Demonstrate the Factory Method pattern with several concrete creators.
// Notes:
//  - The vector owns heterogeneous creators through 
//    unique_ptr<NotificationCreator>.
//  - Each concrete creator is added polymorphically without changing client 
//    logic.
//  - The loop borrows each smart pointer by const reference.
//  - Dereferencing the unique_ptr provides a NotificationCreator reference
//    for client_logic().
//  - Adding WhatsApp required only a new Concrete Product and Concrete 
//    Creator.
int main() {
    
    // Store polymorphic creator objects with exclusive ownership.
    std::vector<std::unique_ptr<NotificationCreator>> creators;
    
    // Add one creator for each supported notification type.
    creators.push_back(std::make_unique<EmailNotificationCreator>());
    creators.push_back(std::make_unique<PushNotificationCreator>());
    creators.push_back(std::make_unique<WhatsAppNotificationCreator>());
    
    // Reuse the same client logic for every concrete creator.
    for (const auto& creator : creators) {
        client_logic(*creator);
    }
    
    return 0;
}
