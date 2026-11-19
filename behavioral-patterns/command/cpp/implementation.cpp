/*
===============================================================================
COMMAND DESIGN PATTERN — ROBOT FACTORY (C++17)
===============================================================================

OVERVIEW
--------
The Command Pattern is a behavioral design pattern that encapsulates a
request as an object.

Instead of directly invoking operations on a receiver, a client creates
command objects representing individual actions.

An invoker stores and executes these commands through a common interface,
without needing to know the concrete command types or their implementation
details.

This example implements a simulated robot factory with three operations:

1. PickItem
   - Instructs the robot to pick up an item.
   - Can reverse a successful pick operation.

2. PlaceItem
   - Instructs the robot to place an item.
   - Can reverse a successful placement.

3. WeldItem
   - Instructs the robot to weld an item.
   - Requires a safety check before execution.
   - Can reverse a successful weld in our simulation.

A RobotController manages the command objects in FIFO order.

The controller maintains two queues:

- pending_:
    Commands waiting to be executed.

- retryLater_:
    Commands that failed their safety checks and must be retried later.


PATTERN PARTICIPANTS
--------------------
ICommand          -> Command Interface
                     Defines execute(), undo(), and safetyCheck().

PickItem          -> ConcreteCommand
                     Encapsulates the robot's picking operation.

PlaceItem         -> ConcreteCommand
                     Encapsulates the robot's placing operation.

WeldItem          -> ConcreteCommand
                     Encapsulates the robot's welding operation
                     and provides a custom safety check.

Robot             -> Receiver
                     Performs the actual operations.
                     Maintains the simulated robot state.

RobotController   -> Invoker
                     Owns and queues command objects.
                     Checks safety before execution.
                     Executes commands sequentially.
                     Invokes undo() when execution fails.
                     Defers unsafe commands for later retry.

main()            -> Client
                     Creates the robot and controller.
                     Creates concrete command objects.
                     Transfers command ownership to the controller.
                     Starts execution and triggers deferred retries.


EXECUTION FLOW
--------------
Client
   |
   v
Create ConcreteCommand(robot)
   |
   v
RobotController::enqueue(command)
   |
   v
pending_ queue
   |
   v
RobotController::runPending()
   |
   v
ICommand::safetyCheck()
   |
   +--> false
   |       |
   |       +--> Move command to retryLater_
   |       |
   |       +--> Continue with the next pending command
   |
   +--> true
           |
           v
       ICommand::execute()
           |
           +--> true
           |       |
           |       +--> Report successful execution
           |
           +--> false
                   |
                   +--> Report execution failure
                   |
                   +--> ICommand::undo()

When the robot's safety state changes:

Client
   |
   v
RobotController::retryDeferred()
   |
   v
Move commands from retryLater_ back to pending_
   |
   v
RobotController::runPending()
   |
   v
Recheck safety and attempt execution


KEY DESIGN DECISIONS
--------------------
- The invoker depends on ICommand, not concrete command classes.
- Concrete commands delegate actual operations to the Robot receiver.
- The Robot owns its physical simulation state.
- Each command tracks whether its own operation succeeded.
- Commands hold non-owning Robot references.
- RobotController owns queued commands through std::unique_ptr.
- std::move transfers ownership between queues and local variables.
- std::queue preserves FIFO execution order.
- safetyCheck() returns true by default.
- WeldItem overrides safetyCheck() to query the robot's current state.
- Unsafe commands are deferred without being executed or undone.
- Commands that report execution failure trigger undo().
- Deferred commands are retried only when explicitly rescheduled.
- The Robot must outlive any commands that reference it.

EDUCATIONAL SIMPLIFICATIONS
---------------------------
- Robot operations are simulated rather than physical.
- Welding is treated as reversible for demonstration purposes.
- The robot tracks item possession and welding using booleans.
- Welding does not require the robot to be holding an item.
- Failed robot operations leave their state unchanged.
- Consequently, undo() after a failed execution normally does nothing.
- Recovery from partial execution failures is not simulated.
- Commands are intended for one execution attempt, except commands
  deferred before execution, which may be retried.
- RobotController does not automatically retry unsafe commands.
- Physical robot safety enforcement would require additional safeguards.

===============================================================================
*/


// ============================================================================
// 1. REQUIRED HEADERS
// ============================================================================

#include <iostream>  // std::cout
#include <memory>    // std::unique_ptr, std::make_unique
#include <queue>     // std::queue
#include <utility>   // std::move


// ============================================================================
// 2. RECEIVER — Robot
// ============================================================================

// Represents the robot that performs the actual factory operations.
//
// The Robot is the Receiver in the Command Pattern.
//
// It knows HOW to perform operations, but it does not know:
//   - Which command object requested an operation.
//   - Which queue contains a command.
//   - When the controller will execute a command.
//
// Its responsibility is to maintain and modify its own state.
class Robot {
private:
    // ------------------------------------------------------------------------
    // 2.1. Robot State
    // ------------------------------------------------------------------------

    // Indicates whether the robot is currently holding an item.
    // Note:
    //  - Initially false because the robot starts empty.
    bool holdingItem_{false};
    
    // Indicates whether welding has already been completed.
    // Note:
    //  - Initially false because no welding operation has occurred.
    bool welded_{false};
    
    // Indicates whether the environment currently permits welding.
    // Note:
    //  - Initially false to demonstrate the retry-later mechanism.
    bool weldingSafe_{false};
    
public:
    // ------------------------------------------------------------------------
    // 2.2. Pick Operation
    // ------------------------------------------------------------------------

    // Attempts to pick up an item.
    //
    // Returns:
    //  - true  -> The item was picked successfully.
    //  - false -> The robot is already holding an item.
    //
    // Note:
    //  - A failed operation leaves the robot's state unchanged.
    bool pickItem() {
        if (holdingItem_) {
            std::cout << "Robot: Cannot pick item. Already holding an item.\n";
            return false;
        }
        else {
            std::cout << "Robot: Item picked successfully.\n";
            holdingItem_ = true;
            return true;
        }
    }
    
    // ------------------------------------------------------------------------
    // 2.3. Undo Pick Operation
    // ------------------------------------------------------------------------

    // Reverses a successful pick operation.
    // Notes:
    //  - In this simulation, undoing a pick releases the held item.
    //  - Concrete commands are responsible for deciding whether
    //    this operation should be invoked.
    void undoPick() {
        std::cout << "Robot: Pick operation undone. Item released.\n";
        holdingItem_ = false;
    }
    
    // ------------------------------------------------------------------------
    // 2.4. Place Operation
    // ------------------------------------------------------------------------

    // Attempts to place the item currently held by the robot.
    //
    // Returns:
    //   true  -> The item was placed successfully.
    //   false -> The robot was not holding an item.
    //
    // Note:
    //  - A successful placement releases the item.
    bool placeItem() {
        if (holdingItem_) {
            std::cout << "Robot: Item placed successfully.\n";
            holdingItem_ = false;
            return true;
        }
        else {
            std::cout << "Robot: Cannot place item. No item is being held.\n";
            return false;
        }
    }
    
    // ------------------------------------------------------------------------
    // 2.5. Undo Place Operation
    // ------------------------------------------------------------------------

    // Reverses a successful placement.
    // Note:
    //  - We simulate restoring the previous state by making the
    //    robot hold the placed item again.
    void undoPlace() {
        std::cout << "Robot: Place operation undone. Item picked back up.\n";
        holdingItem_ = true;
    }
    
    // ------------------------------------------------------------------------
    // 2.6. Read Welding Safety State
    // ------------------------------------------------------------------------

    // Returns whether welding is currently permitted.
    // Notes:
    //  - const: Reading the safety state does not modify the Robot.
    //  - noexcept: Reading a boolean cannot throw an exception.
    //  - WeldItem::safetyCheck() will call this method.
    bool isWeldingSafe() const noexcept {
        return weldingSafe_;
    }
    
    // ------------------------------------------------------------------------
    // 2.7. Update Welding Safety State
    // ------------------------------------------------------------------------

    // Updates the simulated welding safety condition.
    // Notes:
    //  - This allows the client to simulate changing conditions
    //    before retrying a previously deferred command.
    //  - Assigning a boolean does not throw.
    void setWeldingSafe(bool safe) noexcept {
        weldingSafe_ = safe;
    }
    
    // ------------------------------------------------------------------------
    // 2.8. Weld Operation
    // ------------------------------------------------------------------------

    // Attempts to weld an item.
    //
    // Returns:
    //   true  -> Welding succeeded.
    //   false -> Welding was already completed.
    //
    // Important:
    //   This method does not perform the safety check itself.
    //   The controller invokes WeldItem::safetyCheck() before
    //   allowing the command to execute.
    //
    // Note:
    //  - A real industrial robot would require additional independent
    //    safety enforcement.
    bool weldItem() {
        if (welded_) {
            std::cout << "Robot: Cannot weld item. Already welded.\n";
            return false;
        }
        else {
            std::cout << "Robot: Item welded successfully.\n";
            welded_ = true;
            return true;
        }
    }
    
    // ------------------------------------------------------------------------
    // 2.9. Undo Weld Operation
    // ------------------------------------------------------------------------

    // Reverses a successful simulated welding operation.
    // Notes:
    //  - Real welding is not generally reversible.
    //  - This method exists to demonstrate the undo mechanism.
    void undoWeld() {
        std::cout << "Robot: Weld operation undone.\n";
        welded_ = false;
    }
};


// ============================================================================
// 3. COMMAND INTERFACE — ICommand
// ============================================================================

// Defines the common interface for all command objects.
//
// The RobotController depends only on ICommand, allowing it to
// process different commands without knowing their concrete types.
//
// This abstraction is the central element of the Command Pattern.
class ICommand {
public:
    // ------------------------------------------------------------------------
    // 3.1. Virtual Destructor
    // ------------------------------------------------------------------------

    // Ensures correct polymorphic destruction.
    // Notes:
    //  - Commands are owned through std::unique_ptr<ICommand>.
    //  - The virtual destructor allows the appropriate derived
    //    destructor to run when the command object is destroyed.
    virtual ~ICommand() = default;
    
    // ------------------------------------------------------------------------
    // 3.2. Execute Command
    // ------------------------------------------------------------------------

    // Defines the execution operation.
    //
    // Returns:
    //   true  -> Execution succeeded.
    //   false -> Execution failed.
    //
    // Note:
    //  - Pure virtual because each concrete command must implement
    //    its own execution behavior.
    virtual bool execute() = 0;
    
    // ------------------------------------------------------------------------
    // 3.3. Undo Command
    // ------------------------------------------------------------------------

    // Defines the rollback operation.
    // Notes:
    //  - Each concrete command decides how to reverse its own effects.
    //  - The controller invokes this method when execute() reports failure.
    virtual void undo() = 0;
    
    // ------------------------------------------------------------------------
    // 3.4. Safety Check
    // ------------------------------------------------------------------------

    // Determines whether the command is currently safe to execute.
    // Notes:
    //  - Default behavior: Return true.
    //  - Commands that do not need special safety checks can inherit
    //    this implementation without overriding it.
    //  - WeldItem overrides this method.
    virtual bool safetyCheck() {
        return true;
    }
};


// ============================================================================
// 4. CONCRETE COMMANDS
// ============================================================================

// ----------------------------------------------------------------------------
// 4.1. PickItem — ConcreteCommand
// ----------------------------------------------------------------------------

// Encapsulates the robot's picking operation.
//
// The command knows:
//   - Which Robot should perform the operation.
//   - Whether this particular command successfully picked an item.
//
// Note:
//  - The command does not own the Robot.
class PickItem : public ICommand {
private:
    // Non-owning reference to the Receiver.
    // Note:
    //  - The referenced Robot must outlive this command.
    Robot& robot_;
    
    // Tracks whether this command successfully picked an item.
    // Notes:
    //  - This state is distinct from Robot::holdingItem_.
    //  - Robot::holdingItem_ describes the robot's current state.
    //  - PickItem::picked_ describes this command's execution result.
    bool picked_{false};
public:
    // ------------------------------------------------------------------------
    // 4.1.1. Constructor
    // ------------------------------------------------------------------------

    // Initializes the non-owning Robot reference.
    // Note:
    //  - References must be initialized in the constructor initializer list.
    PickItem(Robot& robot) : robot_(robot) {}  
    
    // ------------------------------------------------------------------------
    // 4.1.2. Execute
    // ------------------------------------------------------------------------

    // Delegates the operation to the Robot receiver.
    // Note:
    //  - Stores the result so undo() knows whether this command
    //    performed a successful pick.
    bool execute() override {
        picked_ = robot_.pickItem();
        return picked_;
    }
    
    // ------------------------------------------------------------------------
    // 4.1.3. Undo
    // ------------------------------------------------------------------------

    // Reverses a pick only if this command succeeded.
    // Note:
    //  - This prevents a failed PickItem command from releasing
    //    an item that was picked by another command.
    void undo() override {
        if (picked_) {
            robot_.undoPick();
            picked_ = false;
        }
    }
};


// ----------------------------------------------------------------------------
// 4.2. PlaceItem — ConcreteCommand
// ----------------------------------------------------------------------------

// Encapsulates the robot's placing operation.
// Note:
//  - Like PickItem, this command stores a non-owning Robot reference
//    and tracks whether its own execution succeeded.
class PlaceItem : public ICommand {
private:
    // Non-owning reference to the Receiver.
    Robot& robot_;
    
    // Records whether this command successfully placed an item.
    bool placed_{false};
public:
    // ------------------------------------------------------------------------
    // 4.2.1. Constructor
    // ------------------------------------------------------------------------

    // Initializes the Robot reference without copying the Robot.
    PlaceItem(Robot& robot) : robot_(robot) {}  
    
    // ------------------------------------------------------------------------
    // 4.2.2. Execute
    // ------------------------------------------------------------------------

    // Asks the Robot to place its currently held item.
    // Note:
    //  - Saves the result for conditional rollback.
    bool execute() override {
        placed_ = robot_.placeItem();
        return placed_;
    }
    
    // ------------------------------------------------------------------------
    // 4.2.3. Undo
    // ------------------------------------------------------------------------

    // Reverses a placement only when this command succeeded.
    // Note:
    //  - An unsuccessful placement must not cause the robot
    //    to acquire an item that was never placed.
    void undo() override {
        if (placed_) {
            robot_.undoPlace();
            placed_ = false;
        }
    }
};


// ----------------------------------------------------------------------------
// 4.3. WeldItem — ConcreteCommand with Safety Check
// ----------------------------------------------------------------------------

// Encapsulates the robot's welding operation.
// Notes:
//  - Unlike PickItem and PlaceItem, this command requires a
//    safety check before execution.
//  - The safety condition is stored in the Robot, not the command,
//    so retrying the command checks the current environment.
class WeldItem : public ICommand {
private:
    // Non-owning reference to the Receiver.
    Robot& robot_;
    
    // Records whether this command successfully performed welding.
    bool welded_{false};
public:
    // ------------------------------------------------------------------------
    // 4.3.1. Constructor
    // ------------------------------------------------------------------------

    // Initializes the receiver reference.
    WeldItem(Robot& robot) : robot_(robot) {}  
    
    // ------------------------------------------------------------------------
    // 4.3.2. Execute
    // ------------------------------------------------------------------------

    // Delegates the welding operation to the Robot.
    // Note:
    //  - The controller is expected to call safetyCheck()
    //    before invoking this method.
    bool execute() override {
        welded_ = robot_.weldItem();
        return welded_;
    }
    
    // ------------------------------------------------------------------------
    // 4.3.3. Undo
    // ------------------------------------------------------------------------

    // Reverses a successful simulated welding operation.
    // Note:
    //  - If execution failed without changing the robot's state,
    //    welded_ remains false and undo() does nothing.
    void undo() override {
        if (welded_) {
            robot_.undoWeld();
            welded_ = false;
        }
    }
    
    // ------------------------------------------------------------------------
    // 4.3.4. Custom Safety Check
    // ------------------------------------------------------------------------

    // Overrides the default safety check.
    // Notes:
    //  - Delegates the decision to the Robot's current safety state.
    //  - The result is evaluated each time the controller attempts
    //    to process this command.
    //  - If the Robot becomes safe after the command was deferred,
    //    the same WeldItem object can execute successfully later.
    bool safetyCheck() override {
        return robot_.isWeldingSafe();
    }
};


// ============================================================================
// 5. INVOKER — RobotController
// ============================================================================

// Manages command execution without knowing concrete command types.
//
// The controller communicates only through ICommand.
//
// Responsibilities:
//   1. Accept ownership of submitted commands.
//   2. Store pending commands in FIFO order.
//   3. Check safety before execution.
//   4. Defer commands that are currently unsafe.
//   5. Execute safe commands.
//   6. Invoke undo() when execution reports failure.
//   7. Reschedule deferred commands when requested.
//
// Note:
//  - The controller does not own or directly operate the Robot.
class RobotController {
private:
    // ------------------------------------------------------------------------
    // 5.1. Pending Command Queue
    // ------------------------------------------------------------------------

    // Stores commands waiting for execution.
    // Notes:
    //  - std::queue provides FIFO behavior:
    //    The first command enqueued is the first command processed.
    //  - std::unique_ptr gives the queue exclusive ownership
    //    of its command objects.
    std::queue<std::unique_ptr<ICommand>> pending_;
    
    // ------------------------------------------------------------------------
    // 5.2. Retry-Later Queue
    // ------------------------------------------------------------------------

    // Stores commands that could not pass their safety checks.
    // Notes:
    //  - A deferred command is not destroyed.
    //  - Its ownership moves from pending_ to retryLater_.
    //  - The command can later be transferred back to pending_.
    std::queue<std::unique_ptr<ICommand>> retryLater_;
public:
    // ------------------------------------------------------------------------
    // 5.3. Enqueue Command
    // ------------------------------------------------------------------------

    // Accepts ownership of a command object.
    // Notes:
    //  - The parameter is passed by value, transferring ownership
    //    into the function.
    //  - std::move then transfers ownership into pending_.
    //  - The client no longer owns the enqueued command.
    void enqueue(std::unique_ptr<ICommand> command) {
        pending_.push(std::move(command));
    } 
    
    // ------------------------------------------------------------------------
    // 5.4. Execute Pending Commands
    // ------------------------------------------------------------------------

    // Processes pending commands sequentially in FIFO order.
    //
    // Each command goes through the following steps:
    //
    //   1. Extract ownership from the pending queue.
    //   2. Check whether execution is safe.
    //   3. Defer unsafe commands.
    //   4. Execute safe commands.
    //   5. Call undo() if execution reports failure.
    //
    // Notes:
    //  - The controller uses virtual dispatch through ICommand.
    //  - It does not need to identify the concrete command type.
    void runPending() {
        // Continue until there are no pending commands.
        while (!pending_.empty()) {
            // ---------------------------------------------------------------
            // 5.4.1. Extract the Next Command
            // ---------------------------------------------------------------

            // Move ownership from the queue into a local unique_ptr.
            // Notes:
            //  - std::unique_ptr cannot be copied.
            //  - front() returns a reference to the queue's first element.
            std::unique_ptr<ICommand> command = std::move(pending_.front());
            
            // Remove the now-empty unique_ptr from the queue.
            // Notes:
            //  - Moving must happen before pop().
            //  - Calling pop() first would destroy the queued command.
            pending_.pop();
            
            // ---------------------------------------------------------------
            // 5.4.2. Perform the Safety Check
            // ---------------------------------------------------------------

            // Dynamically dispatches to the command's safetyCheck().
            // Notes:
            //  - PickItem and PlaceItem inherit the default true result.
            //  - WeldItem checks the Robot's current welding safety state.
            if (!command->safetyCheck()) {
                std::cout 
                    << "Controller: Safety check failed. Deferring command.\n";
                    
                // Transfer ownership into the retry-later queue.
                // Note:
                //  - The same command object remains alive.
                retryLater_.push(std::move(command));
                
                // Skip execution and continue with the next command.
                // Notes:
                //  - The local command pointer is now empty.
                //  - Dereferencing it after the move would be invalid.
                continue;
            }

             // ---------------------------------------------------------------
            // 5.4.3. Execute the Command
            // ---------------------------------------------------------------

            // The controller does not know whether this command
            // represents picking, placing, or welding.
            // Note:
            //  - Virtual dispatch selects the concrete execute() method.
            if (command->execute()) {
                std::cout
                    << "Controller: Command executed successfully.\n";
            }
            else {
                // -----------------------------------------------------------
                // 5.4.4. Handle Execution Failure
                // -----------------------------------------------------------
                
                std::cout
                    << "Controller: Command execution failed. Rolling back.\n";
                    
                // Ask the concrete command to undo its effects.
                // Notes:
                //  - Each command tracks whether its own operation succeeded.
                //  - In this simulation, failed Robot operations do not
                //    partially change state, so undo() typically does nothing.
                command->undo();
            }
            
            // ---------------------------------------------------------------
            // 5.4.5. Automatic Command Destruction
            // ---------------------------------------------------------------

            // At the end of this iteration, the local unique_ptr
            // releases the processed command object.
            //
            // No manual delete is necessary.
        }
    }
    
    // ------------------------------------------------------------------------
    // 5.5. Reschedule Deferred Commands
    // ------------------------------------------------------------------------

    // Moves every deferred command back into the pending queue.
    // Notes:
    //  - This method does not execute commands directly.
    //  - The client must call runPending() afterward.
    //  - Commands retain their relative FIFO order.
    //  - If pending_ already contains commands, deferred commands
    //    are appended after the existing pending commands.
    void retryDeferred() {
        while (!retryLater_.empty()) {
            // Extract ownership of the next deferred command.
            std::unique_ptr<ICommand> command = std::move(retryLater_.front());
            
            // Remove the empty entry from retryLater_.
            retryLater_.pop();
            
            // Transfer ownership back to the pending queue.
            pending_.push(std::move(command));
        }
    }
};


// ============================================================================
// 6. CLIENT — main()
// ============================================================================

// Demonstrates the complete Command Pattern implementation.
//
// The client is responsible for:
//   - Constructing the Robot receiver.
//   - Constructing the RobotController invoker.
//   - Creating concrete command objects.
//   - Transferring command ownership to the controller.
//   - Triggering command execution.
//   - Updating the simulated welding safety state.
//   - Requesting retries of deferred commands.
int main() {
    // ------------------------------------------------------------------------
    // 6.1. Create Receiver and Invoker
    // ------------------------------------------------------------------------

    // Construct the Robot receiver.
    //
    // Initial state:
    //   holdingItem_ = false
    //   welded_      = false
    //   weldingSafe_ = false
    Robot robot;
    
    // Construct the invoker.
    // Note:
    //  - Both command queues initially contain no commands.
    RobotController controller;
    
    // ------------------------------------------------------------------------
    // 6.2. Enqueue First PickItem — Successful Execution
    // ------------------------------------------------------------------------

    // Create a PickItem command and transfer ownership
    // directly to the controller.
    //
    // Notes:
    //  - std::make_unique constructs the command dynamically.
    //  - The temporary std::unique_ptr<PickItem> converts into
    //    std::unique_ptr<ICommand> through public inheritance.
    //  - No explicit std::move is required for this temporary.
    controller.enqueue(std::make_unique<PickItem>(robot));
    
    // ------------------------------------------------------------------------
    // 6.3. Enqueue Second PickItem — Execution Failure
    // ------------------------------------------------------------------------

    // Add another independent PickItem command.
    //
    // Notes:
    //  - The first PickItem will make the Robot hold an item.
    //  - Therefore, this second PickItem will fail because
    //    the Robot cannot pick up another item.
    //  - This tests the controller's execution-failure path.
    controller.enqueue(std::make_unique<PickItem>(robot));
    
    // ------------------------------------------------------------------------
    // 6.4. Enqueue WeldItem — Initial Safety Failure
    // ------------------------------------------------------------------------

    // Create a welding command.
    //
    // Notes:
    //  - The Robot initially reports that welding is unsafe.
    //  - Therefore, the controller will move this command into
    //    retryLater_ rather than executing it.
    controller.enqueue(std::make_unique<WeldItem>(robot));
    
    // ------------------------------------------------------------------------
    // 6.5. Enqueue PlaceItem — Successful Execution
    // ------------------------------------------------------------------------

    // Add a placement command.
    //
    // Notes:
    //  - It should execute successfully because the first
    //    PickItem command will have picked up an item.
    //  - The second PickItem's failed execution and undo()
    //    will not disturb that item.
    controller.enqueue(std::make_unique<PlaceItem>(robot));
    
    // ------------------------------------------------------------------------
    // 6.6. Execute the Initial Pending Queue
    // ------------------------------------------------------------------------

    // Process commands in insertion order:
    //
    //   1. PickItem  -> Success
    //   2. PickItem  -> Failure, then undo()
    //   3. WeldItem  -> Safety failure, deferred
    //   4. PlaceItem -> Success
    //
    // Note:
    //  - Unsafe welding does not block later commands.
    controller.runPending();
    
    // ------------------------------------------------------------------------
    // 6.7. Change the Robot's Welding Safety State
    // ------------------------------------------------------------------------

    // Simulate the environment becoming safe for welding.
    //
    // Notes:
    //  - The deferred WeldItem object has not yet been executed.
    //  - Its next safetyCheck() will read this updated Robot state.
    robot.setWeldingSafe(true);
    
    // ------------------------------------------------------------------------
    // 6.8. Reschedule Deferred Commands
    // ------------------------------------------------------------------------

    // Move the deferred WeldItem command from retryLater_ back into pending_.
    //
    // Notes:
    //  - No new WeldItem object is created.
    //  - The controller transfers ownership of the existing command.
    controller.retryDeferred();
    
    // ------------------------------------------------------------------------
    // 6.9. Execute the Rescheduled Commands
    // ------------------------------------------------------------------------

    // Process the pending queue again.
    //
    // Notes:
    //  - WeldItem now passes its safety check and performs welding.
    //  - Expected result:
    //      Robot: Item welded successfully.
    //      Controller: Command executed successfully.
    controller.runPending();
    
    // ------------------------------------------------------------------------
    // 6.10. Automatic Resource Cleanup
    // ------------------------------------------------------------------------

    // The controller and Robot are local objects.
    //
    // They are destroyed automatically when main() returns.
    //
    // Command objects that were processed have already been
    // destroyed through their local std::unique_ptr instances.
    //
    // Commands still present in either controller queue would
    // also be destroyed when the controller is destroyed.
    //
    // The Robot was constructed before the controller, so
    // it remains alive until after the controller is destroyed.
    return 0;
}
