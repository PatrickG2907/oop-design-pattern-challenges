/*
===============================================================================
PROXY DESIGN PATTERN — DIGITAL LIBRARY (C++17)
===============================================================================

OVERVIEW
--------
The Proxy pattern provides a substitute for another object, allowing the proxy
to control access to the real object while exposing the same interface.

This example combines two types of Proxy:

1. Virtual Proxy
   - Delays construction of the real Book until its content is requested.
   - Reuses the existing Book on subsequent requests.
   - Allows lightweight metadata access without constructing the real object.

2. Protection Proxy
   - Restricts content access to users with the Member role.
   - Rejects unauthorized requests before constructing the real Book.

PATTERN PARTICIPANTS
--------------------
IBook       -> Subject
               Defines the common interface for books.

Book        -> RealSubject
               Owns the book's metadata and content.
               Initializes the content during construction.

BookProxy   -> Proxy
               Implements the same interface as Book.
               Performs authorization and lazy initialization.
               Owns the real Book through std::unique_ptr.

UserRole    -> Access-control context
               Distinguishes Guests from Members.

main()      -> Client
               Demonstrates lazy loading, object reuse,
               access control, and runtime polymorphism.

EXECUTION FLOW
--------------
Client
   |
   v
BookProxy::getContent()
   |
   +--> authorizeRead()
   |       |
   |       +--> Guest: throw an exception
   |       |
   |       +--> Member: continue
   |
   +--> ensureLoaded()
   |       |
   |       +--> Book does not exist: construct it
   |       |
   |       +--> Book exists: reuse it
   |
   +--> realBook_->getContent()
           |
           +--> Return the book's content

KEY DESIGN DECISIONS
--------------------
- Both Book and BookProxy implement IBook.
- Metadata is stored in the proxy to avoid unnecessary loading.
- Authorization always happens before initialization.
- The proxy exclusively owns the real Book using std::unique_ptr.
- The Book constructor simulates content loading.
- std::string_view provides non-owning access to stored strings.

===============================================================================
*/


// ============================================================================
// 1. REQUIRED HEADERS
// ============================================================================

#include <iostream>      // std::cout
#include <string>        // std::string
#include <string_view>   // std::string_view
#include <memory>        // std::unique_ptr, std::make_unique
#include <stdexcept>     // std::runtime_error, std::invalid_argument
#include <utility>       // std::move


// ============================================================================
// 2. SUBJECT INTERFACE — IBook
// ============================================================================

// Defines the common interface implemented by the RealSubject and Proxy.
//
// Clients can interact with either implementation through IBook without
// depending on its concrete type.
class IBook {
public:
    // Ensures safe destruction through a base-class pointer.
    virtual ~IBook() = default;
    
    // Provides lightweight book metadata.
    virtual std::string_view getTitle() const = 0;
    virtual std::string_view getAuthor() const = 0;
    
    // Provides access to the book's content.
    // Note:
    //  - Non-const because a proxy may need to construct the real object.
    virtual std::string_view getContent() = 0;
};


// ============================================================================
// 3. REAL SUBJECT — Book
// ============================================================================

// Represents the real object whose construction is deferred by the proxy.
//
// The constructor simulates loading book content into memory.
// Once constructed, the Book owns its metadata and content.
class Book : public IBook {
private:
    // Intrinsic book data owned by the real object.
    std::string title_;
    std::string author_; 
    std::string content_;
    
public:
    // Constructs a real Book and initializes its content.
    Book(
        std::string title, 
        std::string author)
        : title_(std::move(title)),
          author_(std::move(author))
    {
        // Validate the metadata before initializing the content.
        if (title_.empty()) {
            throw std::invalid_argument("Title must not be empty!");
        }
        
        if (author_.empty()) {
            throw std::invalid_argument("Author must not be empty!");
        }
        
        // Simulates the expensive initialization operation.
        // Note:
        //  - This message appears only when a real Book is constructed.
        std::cout << "Book: Initializing book content...\n";
        
        // Hardcoded content for our single-book demonstration.
        content_ = "In Oceania, Winston Smith lives under the constant surveillance"
            " of Big Brother. As he begins to question the Party's control over"
            " truth and freedom, he risks everything.";
    }
    
    // Return non-owning views of the stored metadata.
    std::string_view getTitle() const noexcept override {
        return title_;
    }
    
    std::string_view getAuthor() const noexcept override {
        return author_;
    }
    
    // Return a view of the content already owned by this Book.
    // Note:
    //  - No loading or additional allocation occurs here.
    std::string_view getContent() noexcept override {
        return content_;
    }
};


// ============================================================================
// 4. ACCESS-CONTROL CONTEXT — UserRole
// ============================================================================

// Defines the two roles supported by the demonstration.
//
// Guests can retrieve metadata but cannot access book content.
// Members can retrieve both metadata and content.
enum class UserRole { Guest, Member };


// ============================================================================
// 5. PROXY — BookProxy
// ============================================================================

// Provides controlled access to a real Book through the IBook interface.
//
// Virtual Proxy:
//   Delays construction of Book until the first authorized content request.
//
// Protection Proxy:
//   Checks the user's role before allowing content access.
//
// The proxy handles metadata requests directly, without constructing Book.
class BookProxy : public IBook {
private:
    // Lightweight metadata available before the real Book is constructed.
    std::string title_;
    std::string author_;
    
    // Access permissions associated with this proxy instance.
    UserRole role_;
    
    // Exclusive ownership of the real Book.
    // Note:
    //  - Initially nullptr; populated during the first authorized request.
    std::unique_ptr<Book> realBook_;
    
    // ------------------------------------------------------------------------
    // 5.1. Protection Proxy — Authorization
    // ------------------------------------------------------------------------

    // Verifies that the associated role permits content access.
    //
    // Throws an exception for Guests, preventing any subsequent loading.
    void authorizeRead() const {
        if (role_ != UserRole::Member)
            throw std::runtime_error("Access denied: member role required!");
    }
    
    // ------------------------------------------------------------------------
    // 5.2. Virtual Proxy — Lazy Initialization
    // ------------------------------------------------------------------------

    // Constructs the real Book only when it does not already exist.
    //
    // The unique_ptr retains the instance so later requests can reuse it.
    void ensureLoaded() {
        if (!realBook_) {
            std::cout << "Proxy: Creating the real Book instance...\n";
            
            // Construct the RealSubject on demand.
            // Note:
            //  - Its constructor initializes the actual content.
            realBook_ = std::make_unique<Book>(
                title_,
                author_);
        }
        else {
            // No additional Book is constructed.
            std::cout << "Proxy: Reusing the existing Book instance.\n";
        }
    }
    
public:
    // Constructs only the proxy and stores lightweight information.
    // Note:
    //  - Importantly, this constructor does NOT construct the real Book.
    BookProxy(
        std::string title,
        std::string author,
        UserRole role)
    : title_(std::move(title)),
      author_(std::move(author)),
      role_(role)
    {
        // Validate metadata before accepting the proxy.
        if (title_.empty()) {
            throw std::invalid_argument("Title must not be empty!");
        }
        
        if (author_.empty()) {
            throw std::invalid_argument("Author must not be empty!");
        }
    }
    
    // ------------------------------------------------------------------------
    // 5.3. Lightweight Metadata Access
    // ------------------------------------------------------------------------
    
    // Return metadata directly from the proxy.
    // Note:
    //  - Neither method requires authorization or real-object construction.
    std::string_view getTitle() const noexcept override {
        return title_;
    }
    
    std::string_view getAuthor() const noexcept override {
        return author_;
    }
    
    // ------------------------------------------------------------------------
    // 5.4. Controlled Content Access
    // ------------------------------------------------------------------------

    // Enforces the central Proxy workflow:
    //
    //   1. Authorize the request.
    //   2. Construct the real Book if necessary.
    //   3. Delegate to the real Book.
    //
    // Authorization must occur before initialization so Guests cannot
    // trigger the construction of the real object.
    std::string_view getContent() override {
        authorizeRead();
        ensureLoaded();
        return realBook_->getContent();
    }
};


// ============================================================================
// 6. CLIENT — main()
// ============================================================================

// Demonstrates both proxy behaviors and polymorphic access through IBook.
int main() {
    
    // ------------------------------------------------------------------------
    // 6.1. Member Access — Lazy Loading and Reuse
    // ------------------------------------------------------------------------
    
    std::cout << "Simulating member access...\n";
    
    // Only the proxy is constructed at this point.
    BookProxy memberProxy("1984", "George Orwell", UserRole::Member); 

    // Metadata access does not construct the real Book.
    std::cout << "Title: " << memberProxy.getTitle() << '\n';
    std::cout << "Author: " << memberProxy.getAuthor() << '\n';
    
    // First content request:
    // Authorization succeeds and the real Book is constructed.
    auto content = memberProxy.getContent();
    std::cout << "Content: " << content << '\n';
    
    // Second content request:
    // The same Book instance is reused without loading again.
    content = memberProxy.getContent();
    std::cout << "Content: " << content << '\n';
    
    // ------------------------------------------------------------------------
    // 6.2. Guest Access — Authorization
    // ------------------------------------------------------------------------
    
    std::cout << "\nSimulating guest access...\n";
    
    // Construct a separate proxy associated with the Guest role.
    // Note:
    //  - Its realBook_ is initially nullptr.
    BookProxy guestProxy("1984", "George Orwell", UserRole::Guest);
    
    // Guests can access lightweight metadata.
    std::cout << "Title: " << guestProxy.getTitle() << '\n';
    std::cout << "Author: " << guestProxy.getAuthor() << '\n';
    
    // Guest content access is rejected before Book construction.
    try {
        guestProxy.getContent();
    } 
    catch (const std::runtime_error& e) {
        // Access denial is the expected outcome for this scenario.
        std::cout << e.what() << std::endl;
    }

    // ------------------------------------------------------------------------
    // 6.3. Polymorphic Access — Shared Subject Interface
    // ------------------------------------------------------------------------

    std::cout << "\nDemonstrating polymorphic access...\n";

    // Reference the existing proxy through the abstract IBook interface.
    // Note:
    //  - No new object is created or copied.
    IBook& bookRef = memberProxy;
    
    // Virtual dispatch invokes BookProxy's implementations.
    // Notes:
    //  - Content retrieval still enforces authorization.
    //  - The existing Book instance is reused without loading again.
    std::cout << "Title: " << bookRef.getTitle() << '\n';
    std::cout << "Author: " << bookRef.getAuthor() << '\n';
    content = bookRef.getContent();
    std::cout << "Content: " << content << '\n';
    
    return 0;  
}
