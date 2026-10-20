// -----------------------------------------------------------------------------
// Architecture Overview
// -----------------------------------------------------------------------------

// Decorator Pattern
//
// Adds optional behavior to an object by wrapping it in another object that
// implements the same interface.
//
// Roles:
//  - Notification          -> Component
//  - EmailNotification     -> Concrete Component
//  - NotificationDecorator -> Decorator
//  - SMSDecorator          -> Concrete Decorator
//  - PushDecorator         -> Concrete Decorator
//  - LoggingDecorator      -> Concrete Decorator
//
// The central idea is that a Decorator both:
//  - is a Notification        -> inheritance / same public contract
//  - owns a Notification      -> composition / wrapped behavior
//
// Because decorators expose the same Notification interface as the wrapped
// object, they can be stacked dynamically:
//
//   Logging -> Push -> SMS -> Email
//
// Each decorator adds behavior and then delegates to the wrapped Notification.
// This avoids subclasses for every possible combination such as
// EmailWithSMS, EmailWithPush, EmailWithSMSAndPush, etc.
//
// Decorators own their wrapped Notification through std::unique_ptr.
// This models an exclusive ownership chain and also allows polymorphic objects
// to be stored through the Notification base type.
//
// description() lets the notification chain describe itself recursively.
// Email provides the base description, while delivery decorators append their
// own channel. Logging can therefore inspect the complete chain without the
// client manually duplicating its description.
//
// Logging itself is intentionally not included in the description because it
// is cross-cutting behavior rather than a notification delivery channel.
//
// Client code ultimately works through the Notification abstraction and calls
// send() only once on the outermost decorator.


#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>


// -----------------------------------------------------------------------------
// Component
// -----------------------------------------------------------------------------

// Common contract shared by the concrete notification and all decorators.
// Notes:
//  - send() defines the behavior being decorated.
//  - description() allows a composed decorator chain to describe itself.
class Notification {
public:
    virtual ~Notification() = default;
    
    virtual void send(std::string_view message) const = 0;
    
    virtual std::string description() const = 0;
};


// -----------------------------------------------------------------------------
// Concrete Component
// -----------------------------------------------------------------------------

// Provides the core notification behavior.
// Note:
//  - Decorators add optional behavior around this object.
class EmailNotification : public Notification {
public:
    void send(std::string_view message) const override {
        std::cout << "Email: " << message << '\n';
    }
    
    // Base description from which decorators build the complete channel list.
    std::string description() const override {
        return "Email";
    } 
};


// -----------------------------------------------------------------------------
// Decorator Base
// -----------------------------------------------------------------------------

// Base Decorator.
// Notes:
//  - It is-a Notification so decorators can be used wherever a Notification is
//    expected, and it has-a Notification that represents the wrapped behavior.
//  - Common forwarding is centralized here so concrete decorators do not need
//    direct access to the wrapped object.
class NotificationDecorator : public Notification {
private:
    std::unique_ptr<Notification> wrapped_;

    static void validate_wrapped(
        const std::unique_ptr<Notification>& wrapped
    ) {
        if (!wrapped) {
            throw std::invalid_argument("Wrapped notification must not be null!");
        }
    }
    
protected:
    // Construction is protected because this class exists as infrastructure
    // for concrete decorators rather than as a decorator used directly.
    explicit NotificationDecorator(std::unique_ptr<Notification> wrapped)
        : wrapped_(std::move(wrapped))
    {
        validate_wrapped(wrapped_);
    }
    
public:
    // Default decorator behavior is delegation to the wrapped Notification.
    void send(std::string_view message) const override {
        wrapped_->send(message);
    }
    
    // Forward the current description so concrete decorators can extend it.
    std::string description() const override {
        return wrapped_->description();
    }
};


// -----------------------------------------------------------------------------
// Concrete Decorators
// -----------------------------------------------------------------------------

// Adds SMS delivery before delegating to the wrapped notification.
class SMSDecorator : public NotificationDecorator {
public:
    explicit SMSDecorator(std::unique_ptr<Notification> wrapped)
        : NotificationDecorator(std::move(wrapped))
    {
    }

    // Explicit base call continues the decorator chain.
    void send(std::string_view message) const override {
        std::cout << "SMS: " << message << '\n';
        NotificationDecorator::send(message);
    }
    
    // Extend the description produced by the wrapped chain.
    std::string description() const override {
        return NotificationDecorator::description() + " + SMS";
    }
};


// Adds Push delivery before delegating to the wrapped notification.
class PushDecorator : public NotificationDecorator {
public:
    explicit PushDecorator(std::unique_ptr<Notification> wrapped)
        : NotificationDecorator(std::move(wrapped))
    {
    }

    void send(std::string_view message) const override {
        std::cout << "Push: " << message << '\n';
        NotificationDecorator::send(message);
    }
    
    std::string description() const override {
        return NotificationDecorator::description() + " + Push";
    }
};


// Adds logging without modifying the existing Component or decorators.
// Notes:
//  - It does not override description(), so it inherits the forwarding behavior
//    from NotificationDecorator. 
//  - The wrapped delivery chain therefore describes itself as 
//    "Email + SMS + Push" without including Logging.
class LoggingDecorator : public NotificationDecorator {
public:
    explicit LoggingDecorator(std::unique_ptr<Notification> wrapped)
        : NotificationDecorator(std::move(wrapped))
    {
    }

    void send(std::string_view message) const override {
        std::cout 
            << "Logging notification description: "
            << description()
            << '\n';
        
        // Continue the decorator chain through the base implementation.
        // Note:
        //  - Calling send(message) directly here would recursively call this 
        //    override.
        NotificationDecorator::send(message);
    }
};


// -----------------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------------

// Build the decorator chain one layer at a time:
//
// LoggingDecorator
//      ↓ owns
// PushDecorator
//      ↓ owns
// SMSDecorator
//      ↓ owns
// EmailNotification
//
// Each std::move transfers ownership into the next decorator.
int main() {
    auto email_notification = std::make_unique<EmailNotification>();
    
    auto sms_decorator = 
        std::make_unique<SMSDecorator>(std::move(email_notification));
        
    auto push_decorator = 
        std::make_unique<PushDecorator>(std::move(sms_decorator));
    
    // The outermost decorator can remain a stack object because nothing else
    // needs to wrap or take ownership of it.
    LoggingDecorator logging_decorator(std::move(push_decorator));
    
    // Client code works through the Component abstraction rather than the
    // concrete outer decorator.
    Notification& notification = logging_decorator;
    
    // A single call executes the complete decorator chain.
    notification.send("System update available");
    
    return 0;
}
