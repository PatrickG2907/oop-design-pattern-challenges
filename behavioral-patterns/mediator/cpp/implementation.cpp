
/*
===============================================================================
MEDIATOR DESIGN PATTERN — CHAT ROOM
===============================================================================

Purpose:
    Demonstrate how the Mediator Pattern centralizes communication between
    multiple objects.

Pattern participants:
    IUser       - Abstract Colleague
    ChatUser    - Concrete Colleague
    IChatRoom   - Abstract Mediator
    ChatRoom    - Concrete Mediator

Features:
    - Registration and removal of participants
    - Message broadcasting through a mediator
    - Centralized banned-word censorship
    - Rejection of unregistered senders
    - Const-correct interfaces
    - Non-owning participant relationships

Language:
    C++20 or newer (uses std::unordered_set::contains)

===============================================================================
*/


// ============================================================================
// 1. REQUIRED STANDARD LIBRARY HEADERS
// ============================================================================

#include <iostream>       // std::cout
#include <string>         // std::string
#include <string_view>    // std::string_view
#include <unordered_set>  // std::unordered_set
#include <utility>        // std::move
#include <stdexcept>      // std::invalid_argument, std::runtime_error


// ============================================================================
// 2. FORWARD DECLARATION
// ============================================================================

// IUser refers to IChatRoom in its send() method.
//
// A forward declaration lets us mention IChatRoom by reference before
// the full IChatRoom interface is defined.
//
// We can use an incomplete type as a reference or pointer parameter.
// However, calling its methods requires its complete definition.

class IChatRoom;


// ============================================================================
// 3. ABSTRACT COLLEAGUE — IUser
// ============================================================================

/*
    IUser defines the interface shared by all chat participants.

    In the Mediator Pattern, a colleague communicates with other colleagues
    indirectly through the mediator.

    IUser does not know:
        - Which other users are registered
        - How messages are distributed
        - Whether censorship is enabled
        - How the mediator stores its participants

    All of those responsibilities belong to the mediator.
*/

class IUser {
public:

    // Virtual destructor:
    // Ensures correct destruction if a derived user is ever deleted
    // through a pointer to IUser.
    virtual ~IUser() = default;


    // Returns the user's name without copying the underlying string.
    //
    // const std::string&:
    //     Returns a read-only reference to the stored name.
    //
    // Trailing const:
    //     Guarantees this method does not modify the user.
    //
    // = 0:
    //     Makes this method pure virtual and IUser abstract.
    virtual const std::string& name() const = 0;


    // Sends a message through a mediator.
    //
    // IChatRoom& room:
    //     The mediator responsible for routing the message.
    //
    // std::string_view message:
    //     A lightweight, non-owning view of the message text.
    //
    // Trailing const:
    //     Sending does not change the sender's internal state.
    virtual void send(
        IChatRoom& room,
        std::string_view message
    ) const = 0;


    // Receives a message delivered by the mediator.
    //
    // senderName:
    //     Name of the user who originally sent the message.
    //
    // message:
    //     The text delivered by the mediator, potentially censored.
    //
    // Trailing const:
    //     Our current implementation only displays messages and does
    //     not change the receiving user's internal state.
    virtual void receive(
        std::string_view senderName,
        std::string_view message
    ) const = 0;
};


// ============================================================================
// 4. ABSTRACT MEDIATOR — IChatRoom
// ============================================================================

/*
    IChatRoom defines the communication contract.

    Concrete colleagues depend on this interface rather than on ChatRoom.

    The mediator is responsible for:
        - Registering users
        - Removing users
        - Receiving messages from senders
        - Deciding how messages are delivered

    Different chat room implementations could provide different routing
    strategies without requiring changes to ChatUser.
*/

class IChatRoom {
public:

    // Supports safe polymorphic destruction.
    virtual ~IChatRoom() = default;


    // Registers a user as a participant.
    //
    // IUser& allows the mediator to accept any concrete implementation
    // of the IUser interface.
    virtual void join(IUser& user) = 0;


    // Removes a participant from the chat room.
    //
    // The mediator stops referencing the user but does not destroy it.
    virtual void leave(IUser& user) = 0;


    // Routes a message from sender to other participants.
    //
    // const IUser& sender:
    //     The mediator needs only read-only access to the sender.
    //
    // std::string_view message:
    //     Avoids an unnecessary input-string copy.
    //
    // This method is intentionally non-const because different concrete
    // mediators might modify their state while processing messages.
    virtual void sendMessage(
        const IUser& sender,
        std::string_view message
    ) = 0;
};


// ============================================================================
// 5. CONCRETE COLLEAGUE — ChatUser
// ============================================================================

/*
    ChatUser represents an individual chat participant.

    Its responsibilities are intentionally limited:
        - Store the user's name
        - Delegate outgoing messages to the mediator
        - Display incoming messages

    Important:
        ChatUser does not maintain a list of recipients.
        It never calls another ChatUser's receive() method directly.

    This keeps communication logic centralized in ChatRoom.
*/

class ChatUser : public IUser {
private:

    // Each user owns their name.
    std::string name_;


public:

    // Constructs a user by taking ownership of a name.
    //
    // explicit:
    //     Prevents unintended implicit conversions to ChatUser.
    //
    // std::string name:
    //     Passing by value supports both copying and moving.
    //
    // std::move(name):
    //     Allows the parameter's string resources to be moved into
    //     name_ rather than copied.
    explicit ChatUser(std::string name)
        : name_(std::move(name)) {

        // Validate name_ rather than name because the parameter has
        // already been moved from.
        if (name_.empty()) {
            throw std::invalid_argument(
                "User name must not be empty!"
            );
        }
    }


    // Implements IUser::name().
    //
    // Returning by const reference avoids copying the name and prevents
    // callers from modifying it through this reference.
    //
    // override asks the compiler to verify that this method correctly
    // overrides a virtual method in IUser.
    const std::string& name() const override {
        return name_;
    }


    // Implements IUser::send().
    //
    // Instead of contacting recipients directly, the user delegates
    // message handling to the mediator.
    void send(
        IChatRoom& room,
        std::string_view message
    ) const override {

        // *this refers to the current ChatUser object.
        //
        // It can be converted to const IUser& because ChatUser derives
        // publicly from IUser.
        //
        // The mediator decides whether the sender is registered,
        // whether the message needs censorship, and who receives it.
        room.sendMessage(*this, message);
    }


    // Implements IUser::receive().
    //
    // The mediator invokes this method when a message is delivered.
    //
    // For this exercise, receiving a message prints it to the console.
    // The user object's own state is not modified.
    void receive(
        std::string_view senderName,
        std::string_view message
    ) const override {

        // Example:
        // [Bob] Alice: Hello everyone!
        //
        // name_      = recipient's name
        // senderName = original sender's name
        // message    = text delivered by the mediator
        std::cout << '[' << name_ << "] "
                  << senderName << ": "
                  << message << '\n';
    }
};


// ============================================================================
// 6. CONCRETE MEDIATOR — ChatRoom
// ============================================================================

/*
    ChatRoom implements centralized communication between users.

    It handles:
        1. Participant registration and removal
        2. Sender membership validation
        3. Banned-word censorship
        4. Message broadcasting

    The mediator knows users only through the IUser interface.

    This allows different IUser implementations to participate without
    modifying ChatRoom.

    Ownership:
        ChatRoom does NOT own its participants.

        members_ stores raw pointers to existing user objects.
        These pointers are non-owning.

        A registered user must remain alive while its pointer is stored
        in members_.

        Call leave() before destroying a registered user.
*/

class ChatRoom : public IChatRoom {
private:

    // Stores the addresses of currently registered users.
    //
    // const IUser*:
    //     The mediator can access the user through const methods only.
    //
    // std::unordered_set:
    //     - Prevents duplicate pointers
    //     - Supports efficient membership checks
    //     - Supports efficient insertion and removal
    //
    // Iteration order is unspecified.
    std::unordered_set<const IUser*> members_;


    // Stores banned words.
    //
    // Using a set avoids duplicate entries.
    //
    // The current filtering algorithm is case-sensitive and matches
    // substrings, not necessarily complete words.
    std::unordered_set<std::string> bannedWords_;


    // ------------------------------------------------------------------------
    // 6.1. PRIVATE HELPER — Censor banned words
    // ------------------------------------------------------------------------

    /*
        Returns a new string with banned-word matches replaced by '*'.

        Example:
            Banned word: "secret"
            Input:       "This is a secret!"
            Output:      "This is a ******!"

        Design decisions:
            - The original message is not modified.
            - Every occurrence is searched for.
            - Each matched word is replaced with the same number of '*'.
            - Matching is case-sensitive.
            - Substrings are also matched.

        The method is const because censorship does not modify the
        ChatRoom or its banned-word collection.

        Limitation:
            If banned words overlap, the result can depend on the
            iteration order of bannedWords_.
    */
    std::string censor(std::string_view message) const {

        // Create an owned, mutable copy of the incoming message.
        //
        // std::string_view does not own or modify the original text.
        std::string result{message};


        // Examine every banned word.
        for (const auto& word : bannedWords_) {

            // Find the first occurrence.
            //
            // If no occurrence exists, find() returns std::string::npos.
            auto pos = result.find(word);


            // Continue until no further occurrence is found.
            while (pos != std::string::npos) {

                // Replace the matched word with asterisks.
                //
                // Arguments:
                //     pos         - starting position
                //     word.size() - number of characters to remove
                //     word.size() - number of '*' characters to insert
                //     '*'         - replacement character
                //
                // This overload avoids creating a temporary string.
                result.replace(
                    pos,
                    word.size(),
                    word.size(),
                    '*'
                );


                // Resume the search after the replaced region.
                //
                // Since the replacement has the same length as the
                // original word, later character positions do not shift.
                //
                // Empty banned words are prohibited by addBannedWord(),
                // ensuring that this search position advances.
                pos = result.find(word, pos + word.size());
            }
        }


        // Return the processed message.
        return result;
    }


public:

    // ------------------------------------------------------------------------
    // 6.2. REGISTER A USER — join()
    // ------------------------------------------------------------------------

    /*
        Adds a user to the room.

        &user obtains the address of the existing user object.

        The unordered_set stores this address but does not take ownership.

        Inserting an already-registered pointer has no additional effect
        because unordered_set stores unique elements.
    */
    void join(IUser& user) override {
        members_.insert(&user);
    }


    // ------------------------------------------------------------------------
    // 6.3. REMOVE A USER — leave()
    // ------------------------------------------------------------------------

    /*
        Removes the user's address from the room.

        erase() removes the element if it exists.

        If the user is not registered, no element is removed.

        This does not destroy the ChatUser object.
    */
    void leave(IUser& user) override {
        members_.erase(&user);
    }


    // ------------------------------------------------------------------------
    // 6.4. ROUTE MESSAGES — sendMessage()
    // ------------------------------------------------------------------------

    /*
        The main Mediator Pattern operation.

        Processing sequence:
            1. Verify that the sender is registered.
            2. Censor the message once.
            3. Iterate over registered participants.
            4. Skip the original sender.
            5. Deliver the filtered message to other users.

        Note:
            std::unordered_set does not guarantee iteration order.
            Therefore, recipients can receive messages in any order.
    */
    void sendMessage(
        const IUser& sender,
        std::string_view message
    ) override {

        // Step 1: Reject senders who are not room members.
        //
        // contains() is available in C++20.
        //
        // The check uses object identity (address), not the user's name.
        if (!members_.contains(&sender)) {
            throw std::runtime_error("Unknown sender!");
        }


        // Step 2: Apply censorship exactly once.
        //
        // const prevents accidental modification during broadcasting.
        //
        // Performing this outside the loop avoids repeating the same
        // work separately for each recipient.
        const std::string filteredMessage = censor(message);


        // Step 3: Visit all registered users.
        for (const IUser* member : members_) {

            // Step 4: Exclude the sender from the broadcast.
            if (member != &sender) {

                // Step 5: Deliver the filtered message.
                //
                // Dynamic dispatch invokes the concrete user's
                // implementation of receive().
                //
                // sender.name() identifies the original sender.
                member->receive(sender.name(), filteredMessage);
            }
        }
    }


    // ------------------------------------------------------------------------
    // 6.5. CONFIGURE CENSORSHIP — addBannedWord()
    // ------------------------------------------------------------------------

    /*
        Adds a word to the mediator's censorship list.

        This method belongs to the concrete ChatRoom rather than the
        IChatRoom interface because censorship is implementation-specific.

        Passing by value allows the string to be moved into the set.
    */
    void addBannedWord(std::string word) {

        // Empty words are invalid.
        //
        // They would also interfere with the search loop in censor().
        if (word.empty()) {
            throw std::invalid_argument(
                "Banned word must not be empty!"
            );
        }


        // Store the banned word, moving its contents where possible.
        //
        // Duplicate banned words do not create duplicate set entries.
        bannedWords_.insert(std::move(word));
    }
};


// ============================================================================
// 7. CLIENT CODE — main()
// ============================================================================

/*
    Demonstrates the complete chat room.

    Tests:
        1. Creating the mediator and colleagues
        2. Registering participants
        3. Broadcasting a normal message
        4. Configuring censorship
        5. Broadcasting a censored message
        6. Removing a participant
        7. Rejoining a participant
        8. Rejecting an unregistered sender
        9. Cleaning up registered participants
*/

int main() {

    // ------------------------------------------------------------------------
    // 7.1. Create the mediator and users
    // ------------------------------------------------------------------------

    // The room is constructed before the participants.
    ChatRoom room;

    // Each ChatUser owns its name.
    ChatUser alice("Alice");
    ChatUser bob("Bob");
    ChatUser charlie("Charlie");


    // ------------------------------------------------------------------------
    // 7.2. Register the users
    // ------------------------------------------------------------------------

    // The mediator stores non-owning pointers to these objects.
    room.join(alice);
    room.join(bob);
    room.join(charlie);


    // ------------------------------------------------------------------------
    // 7.3. Test ordinary message broadcasting
    // ------------------------------------------------------------------------

    // Alice sends a message through the mediator.
    //
    // Bob and Charlie receive it.
    // Alice does not receive her own message.
    alice.send(room, "Hello everyone!");


    // ------------------------------------------------------------------------
    // 7.4. Configure banned-word censorship
    // ------------------------------------------------------------------------

    // Add words that the mediator should replace with asterisks.
    room.addBannedWord("secret");
    room.addBannedWord("spoiler");


    // ------------------------------------------------------------------------
    // 7.5. Test censorship with multiple occurrences
    // ------------------------------------------------------------------------

    // The mediator replaces:
    //     secret  -> ******
    //     spoiler -> *******
    //
    // "secret" appears twice, so both occurrences are censored.
    alice.send(
        room,
        "This secret contains a spoiler and another secret!"
    );


    // ------------------------------------------------------------------------
    // 7.6. Test dynamic membership — Charlie leaves
    // ------------------------------------------------------------------------

    // Remove Charlie from the room.
    //
    // Charlie still exists as an object, but the mediator no longer
    // includes him in broadcasts.
    room.leave(charlie);


    // Only Bob receives this message.
    alice.send(room, "Charlie, are you still here?");


    // ------------------------------------------------------------------------
    // 7.7. Test dynamic membership — Charlie rejoins
    // ------------------------------------------------------------------------

    // Re-register Charlie using the same object.
    room.join(charlie);


    // Bob and Charlie both receive this message.
    alice.send(room, "Welcome back, Charlie!");


    // ------------------------------------------------------------------------
    // 7.8. Test rejection of an unregistered sender
    // ------------------------------------------------------------------------

    // Create David, but deliberately do not register him.
    ChatUser david("David");


    try {

        // ChatUser::send() delegates to ChatRoom::sendMessage().
        //
        // The mediator checks membership and throws std::runtime_error
        // because David's address is not in members_.
        david.send(room, "Hello, can anyone hear me?");
    }
    catch (const std::runtime_error& e) {

        // Catch the expected error so execution can continue.
        //
        // Expected:
        // Rejected message: Unknown sender!
        std::cout << "Rejected message: "
                  << e.what() << '\n';
    }


    // ------------------------------------------------------------------------
    // 7.9. Explicitly unregister all remaining participants
    // ------------------------------------------------------------------------

    // Remove every registered user before the local objects are destroyed.
    //
    // This maintains the room's non-owning pointer contract.
    room.leave(alice);
    room.leave(bob);
    room.leave(charlie);

    // David was never registered, so no leave() call is needed.


    // ------------------------------------------------------------------------
    // 7.10. End the program
    // ------------------------------------------------------------------------

    return 0;
}
