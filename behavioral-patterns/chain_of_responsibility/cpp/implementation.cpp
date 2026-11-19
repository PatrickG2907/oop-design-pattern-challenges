/*
===============================================================================
CHAIN OF RESPONSIBILITY DESIGN PATTERN — CUSTOMER SUPPORT SYSTEM (C++17)
===============================================================================

OVERVIEW
--------
The Chain of Responsibility is a behavioral design pattern that allows a
request to pass through a sequence of handlers until one of them processes it.

Each handler decides whether it can handle the request or must forward it
to its successor.

The client does not need to know which handler will ultimately process
the request.

This example implements a customer support system with three support levels:

1. Level 1
   - Handles Low-severity tickets.
   - Forwards Medium- and High-severity tickets.

2. Level 2
   - Handles Low- and Medium-severity tickets.
   - Forwards High-severity tickets.

3. Level 3
   - Handles tickets of every supported severity.
   - Represents the final level in the chain.

The example also introduces keyword-based routing:

- "urgent" -> Start directly at Level 3.
- "vip"    -> Start directly at Level 2.
- Default  -> Start at Level 1.

If both keywords occur, "urgent" takes priority.


PATTERN PARTICIPANTS
--------------------
Severity          -> Request classification
                     Defines Low, Medium, and High severity.

Ticket            -> Request
                     Contains the ticket's severity and description.

SupportHandler    -> Abstract Handler
                     Defines the common handling interface.
                     Stores a non-owning pointer to its successor.
                     Provides shared forwarding behavior.

Level1Handler     -> ConcreteHandler
                     Handles Low-severity tickets.

Level2Handler     -> ConcreteHandler
                     Handles Low- and Medium-severity tickets.

Level3Handler     -> ConcreteHandler
                     Handles all supported severity levels.

TicketRouter      -> Entry-point Router
                     Examines ticket descriptions for special keywords.
                     Selects where the ticket enters the chain.

main()            -> Client
                     Constructs and connects the handlers.
                     Creates the router and submits example tickets.


EXECUTION FLOW
--------------
Client
   |
   v
TicketRouter::route(ticket)
   |
   +--> Description contains "urgent"
   |       |
   |       +--> Start at Level 3
   |
   +--> Description contains "vip"
   |       |
   |       +--> Start at Level 2
   |
   +--> Otherwise
           |
           +--> Start at Level 1

After selecting the starting handler:

SelectedHandler::handle(ticket)
   |
   +--> Handler can process the severity
   |       |
   |       +--> Process ticket and stop
   |
   +--> Handler cannot process the severity
           |
           +--> forward(ticket)
                   |
                   +--> Successor exists
                   |       |
                   |       +--> next_->handle(ticket)
                   |
                   +--> No successor
                           |
                           +--> Report unhandled ticket


KEY DESIGN DECISIONS
--------------------
- Each concrete handler implements the same SupportHandler interface.
- The successor pointer is non-owning and initially nullptr.
- Shared forwarding behavior is implemented once in the base class.
- Concrete handlers determine whether to process or forward a ticket.
- TicketRouter determines the initial handler, not the final handler.
- Router references are non-owning and initialized during construction.
- main() owns the handlers and ensures their lifetimes remain valid.
- Higher support levels can handle lower-severity requests.
- Keyword matching is case-sensitive and uses substring searching.
- Severity comparisons rely on the ordering Low < Medium < High.

===============================================================================
*/


// ============================================================================
// 1. REQUIRED HEADERS
// ============================================================================

#include <iostream>  // std::cout
#include <string>    // std::string


// ============================================================================
// 2. REQUEST CLASSIFICATION — Severity
// ============================================================================

// Represents the severity of a customer support issue.
//
// The enumerators are ordered from lowest to highest severity.
// This ordering allows handlers to compare severity levels directly.

enum class Severity { 
    Low, 
    Medium, 
    High
};


// ============================================================================
// 3. REQUEST — Ticket
// ============================================================================

// Represents a customer support request.
//
// severity:
//   Determines which support levels are capable of handling the ticket.
//
// description:
//   Describes the issue and may contain routing keywords such as
//   "vip" or "urgent".

struct Ticket { 
    Severity severity;  
    std::string description;
};


// ============================================================================
// 4. ABSTRACT HANDLER — SupportHandler
// ============================================================================

// Defines the common interface for all support handlers.
//
// Each handler can:
//   1. Process a ticket through handle().
//   2. Forward a ticket to its successor through forward().
//
// The base class implements shared forwarding behavior, while concrete
// handlers implement their own severity-handling rules.

class SupportHandler {
private:
    // Non-owning pointer to the next handler in the chain.
    // Notes:
    //  - Initially nullptr because a handler may have no successor.
    //  - The final handler does not require a next handler.
    SupportHandler* next_{nullptr};
    
protected:
    // ------------------------------------------------------------------------
    // 4.1. Shared Forwarding Behavior
    // ------------------------------------------------------------------------

    // Passes a ticket to the next handler when a successor exists.
    // Notes:
    //  - If there is no successor, reports that the ticket could not
    //    be handled.
    //  - This implementation is shared by all concrete handlers,
    //    avoiding duplicated forwarding logic.
    void forward(const Ticket& ticket) {
        if (next_) {
            std::cout 
                << "Forwarding the following ticket to the next support level: "
                << ticket.description
                << '\n';
                
            // Virtual dispatch invokes the successor's handle() method.
            next_->handle(ticket);
        }
        else {
            std::cout 
                << "The following ticket could not be handled: " 
                << ticket.description
                << '\n';
        }
    }

public:
    // ------------------------------------------------------------------------
    // 4.2. Virtual Destructor
    // ------------------------------------------------------------------------

    // Ensures correct polymorphic destruction through a base-class pointer.
    virtual ~SupportHandler() = default;
    
    // ------------------------------------------------------------------------
    // 4.3. Successor Configuration
    // ------------------------------------------------------------------------

    // Connects this handler to its successor.
    // Notes:
    //  - The pointer is non-owning. The client is responsible for ensuring
    //    that the successor remains alive while it may be accessed.
    //  - This method is not virtual because all handlers use the same
    //    successor-configuration behavior.
    void setNext(SupportHandler* next) {
        next_ = next;
    }
    
    // ------------------------------------------------------------------------
    // 4.4. Abstract Handling Interface
    // ------------------------------------------------------------------------

    // Defines the operation that each concrete handler must implement.
    // Notes:
    //  - The method is pure virtual because different support levels
    //    have different handling capabilities.
    //  - The ticket is passed by const reference to avoid copying
    //    and prevent modification.
    virtual void handle(const Ticket& ticket) = 0;
};


// ============================================================================
// 5. CONCRETE HANDLERS
// ============================================================================

// ----------------------------------------------------------------------------
// 5.1. Level 1 Handler — Low Severity
// ----------------------------------------------------------------------------

// Handles Low-severity tickets.
// Note:
//  - Medium- and High-severity tickets are forwarded to the next handler.
class Level1Handler : public SupportHandler {
public:
    void handle(const Ticket& ticket) override {
        if (ticket.severity == Severity::Low) {
            // Level 1 is capable of processing this ticket.
            std::cout 
                << "Level 1 handled the following ticket: "
                << ticket.description
                << '\n';
        }
        else {
            // Delegate more complex issues to the successor.
            forward(ticket);
        }
    }
};


// ----------------------------------------------------------------------------
// 5.2. Level 2 Handler — Low and Medium Severity
// ----------------------------------------------------------------------------

// Handles Low- and Medium-severity tickets.
// Notes:
//  - High-severity tickets are forwarded to the next handler.
//  - Supporting Low severity is important because VIP tickets may
//    enter the chain directly at Level 2, bypassing Level 1.
class Level2Handler : public SupportHandler {
public:
    void handle(const Ticket& ticket) override {
        // The comparison relies on the enum ordering: Low < Medium < High.
        if (ticket.severity <= Severity::Medium) {
            std::cout 
                << "Level 2 handled the following ticket: "
                << ticket.description
                << '\n';
        }
        else {
            // High-severity issues require further escalation.
            forward(ticket);
        }
    }
};


// ----------------------------------------------------------------------------
// 5.3. Level 3 Handler — All Severities
// ----------------------------------------------------------------------------

// Represents the highest support level.
// Note:
//  - Level 3 can handle every supported severity, so no severity
//    comparison or forwarding operation is necessary.
class Level3Handler : public SupportHandler {
public:
    void handle(const Ticket& ticket) override {
        std::cout 
                << "Level 3 handled the following ticket: "
                << ticket.description
                << '\n';
    }
};


// ============================================================================
// 6. ENTRY-POINT ROUTER — TicketRouter
// ============================================================================

// Determines where a ticket enters the Chain of Responsibility.
//
// The router examines the ticket description for routing keywords.
//
// Routing priority:
//   1. "urgent" -> Level 3
//   2. "vip"    -> Level 2
//   3. Default  -> Level 1
//
// The router does not process tickets itself.
// It delegates each request to the selected starting handler.
//
// Subsequent escalation remains the responsibility of the handlers.
class TicketRouter {
private:
    // Non-owning references to the three support levels.
    // Note:
    //  - References are appropriate because all three handlers must exist
    //    and do not need to be replaced after router construction.
    SupportHandler& level1_;
    SupportHandler& level2_;
    SupportHandler& level3_;

public:
    // ------------------------------------------------------------------------
    // 6.1. Constructor — Handler References
    // ------------------------------------------------------------------------

    // Initializes references to the existing support handlers.
    // Notes:
    //  - No handlers are copied, moved, or dynamically allocated.
    //  - The referenced handlers must outlive the TicketRouter.
    TicketRouter(
        SupportHandler& level1,
        SupportHandler& level2,
        SupportHandler& level3)
    : level1_(level1),
      level2_(level2),
      level3_(level3)
    {
    }
    
    // ------------------------------------------------------------------------
    // 6.2. Keyword-Based Routing
    // ------------------------------------------------------------------------

    // Examines the ticket description and selects the starting handler.
    // Notes:
    //  - Keyword matching is case-sensitive.
    //  - The urgent condition is checked first to ensure it takes
    //    precedence when both "urgent" and "vip" are present.
    void route(const Ticket& ticket) {
        if (ticket.description.find("urgent") != std::string::npos) {
            // Urgent tickets bypass Level 1 and Level 2.
            std::cout 
                << "Router: Urgent ticket detected. Routing directly to Level 3.\n";
            level3_.handle(ticket);   
        }
        else if (ticket.description.find("vip") != std::string::npos) {
            // VIP tickets bypass Level 1.
            // Note: 
            //  - Level 2 may still escalate the ticket if its severity
            //    exceeds Level 2's capabilities.
            std::cout 
                << "Router: VIP ticket detected. Skipping Level 1.\n";
            level2_.handle(ticket);   
        }
        else {
            // Ordinary tickets begin at Level 1 and follow
            // the normal escalation chain.
            std::cout 
                << "Router: Standard ticket. Starting at Level 1.\n";
            level1_.handle(ticket);
        }
    }
};


// ============================================================================
// 7. CLIENT — main()
// ============================================================================

// Constructs the support chain, configures the router, and submits
// tickets demonstrating normal escalation and keyword-based shortcuts.
int main() {
    // ------------------------------------------------------------------------
    // 7.1. Create and Connect the Support Handlers
    // ------------------------------------------------------------------------

    // Create the three concrete handlers as local objects.
    // Notes:
    //  - Their lifetimes are managed automatically.
    //  - No dynamic allocation or smart pointers are necessary.
    Level1Handler level1;
    Level2Handler level2;
    Level3Handler level3;
    
    // Establish the normal escalation chain: Level 1 -> Level 2 -> Level 3
    // Note:
    //  - Level 3 has no successor because it handles every severity.
    level1.setNext(&level2);
    level2.setNext(&level3);
    
    // ------------------------------------------------------------------------
    // 7.2. Create the Ticket Router
    // ------------------------------------------------------------------------

    // The router receives non-owning references to the existing handlers.
    // It determines where tickets enter the chain based on keywords.
    TicketRouter router(
        level1, 
        level2, 
        level3
    );

    // ------------------------------------------------------------------------
    // 7.3. Normal Routing — Low Severity
    // ------------------------------------------------------------------------

    // No special keyword is present.
    // The ticket starts at Level 1 and is handled immediately.
    Ticket lowTicket = {
        Severity::Low,
        "password reset required."    
    };
    
    router.route(lowTicket);
    
    // ------------------------------------------------------------------------
    // 7.4. Normal Routing — Medium Severity
    // ------------------------------------------------------------------------

    // The ticket starts at Level 1.
    // Level 1 cannot handle Medium severity and forwards it to Level 2.
    Ticket mediumTicket = {
        Severity::Medium,
        "application crashes frequently."
    };
    
    router.route(mediumTicket);
    
    // ------------------------------------------------------------------------
    // 7.5. Normal Routing — High Severity
    // ------------------------------------------------------------------------

    // The ticket starts at Level 1 and passes through Level 2.
    // Neither level can handle High severity, so Level 3 processes it.
    Ticket highTicket = {
        Severity::High,
        "production database unavailable."    
    };

    router.route(highTicket);
    
    // ------------------------------------------------------------------------
    // 7.6. VIP Shortcut — Low Severity
    // ------------------------------------------------------------------------

    // The "vip" keyword causes the router to skip Level 1.
    // Level 2 handles the Low-severity ticket directly.
    Ticket vipTicket = {
        Severity::Low,
        "vip customer requests password reset."   
    };
    
    router.route(vipTicket);
    
    // ------------------------------------------------------------------------
    // 7.7. Urgent Shortcut — Medium Severity
    // ------------------------------------------------------------------------

    // The "urgent" keyword causes the router to start at Level 3.
    // Level 1 and Level 2 are bypassed even though Level 2
    // would otherwise be capable of handling Medium severity.
    Ticket urgentTicket = {
        Severity::Medium,
        "urgent: payment processing is unavailable."   
    };
    
    router.route(urgentTicket);
    
    // ------------------------------------------------------------------------
    // 7.8. Combined Keywords — Urgent Takes Priority
    // ------------------------------------------------------------------------

    // Both "vip" and "urgent" are present.
    // Since TicketRouter checks "urgent" first, the ticket
    // starts directly at Level 3 regardless of keyword position.
    Ticket urgentVipTicket = {
        Severity::Low,
        "vip customer reports urgent account issue."   
    };
    
    router.route(urgentVipTicket);
    
    // ------------------------------------------------------------------------
    // 7.9. VIP Escalation — High Severity
    // ------------------------------------------------------------------------

    // The "vip" keyword causes the ticket to start at Level 2.
    // However, Level 2 cannot handle High severity.
    // It therefore forwards the ticket to Level 3.
    // This demonstrates how initial routing and normal escalation
    // cooperate without duplicating their responsibilities.
    Ticket highVipTicket = {
        Severity::High,
        "vip customer reports critical server failure."   
    };
    
    router.route(highVipTicket);
    
    return 0;
}
