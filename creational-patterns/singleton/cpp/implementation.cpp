// ============================================================================
// Architecture Overview
// ============================================================================
//
//  - Logger implements the Singleton Pattern and provides one shared logging
//    instance for the entire application.
//  - The constructor is private so Logger objects cannot be created directly
//    outside the class.
//  - instance() returns the single shared Logger by reference.
//  - The singleton is stored as a function-local static object whose
//    initialization is thread-safe since C++11.
//  - Copy and move operations are explicitly deleted so the singleton cannot
//    be duplicated or transferred into another Logger object.
//  - Logger owns its log messages in a std::vector<std::string>.
//  - A std::mutex protects concurrent access to the shared message collection.
//  - log() validates and appends messages while holding the mutex.
//  - show_logs() reads the stored messages while holding the same mutex.
//  - The mutex is mutable so show_logs() can remain logically const while
//    still synchronizing access.
//  - main() demonstrates that repeated calls to Logger::instance() return the
//    same object and that worker threads also access and modify that same
//    shared Logger.
//
// Patterns / Concepts:
//  - Singleton Pattern: restricts Logger construction to one shared instance.
//  - RAII: std::lock_guard automatically manages mutex locking and unlocking.
//  - Thread Safety: shared Logger state is protected against concurrent access.
//  - Deleted Special Members: copying and moving are explicitly forbidden.


// ============================================================================
// Includes
// ============================================================================

#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>


// ============================================================================
// Singleton Logger
// ============================================================================

// Thread-safe Singleton logger that stores messages in memory.
// Notes:
//  - The constructor is private so callers cannot create Logger objects directly.
//  - instance() provides the single shared Logger instance.
//  - The function-local static instance is initialized exactly once and is
//    thread-safe since C++11.
//  - Copying and moving are explicitly disabled so the singleton cannot be
//    duplicated.
//  - messages_ owns all stored log messages.
//  - mutex_ protects concurrent access to messages_.
//  - mutex_ is mutable so const operations such as show_logs() can still
//    synchronize access without logically modifying the Logger.
class Logger {
private:
    // Prevent direct construction outside the class.
    Logger() = default;
    
    std::vector<std::string> messages_;
    mutable std::mutex mutex_;
    
public:
    // Return the single shared Logger instance.
    // Note:
    //  - The local static object is created only once and persists for the
    //    lifetime of the program.
    static Logger& instance() {
        static Logger logger;
        return logger;
    }
    
    // Prevent creation of additional Logger objects through copying.
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Prevent creation or replacement of Logger objects through moving.
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    // Append one message to the shared log.
    // Notes:
    //  - The message is validated before acquiring the lock.
    //  - lock_guard keeps messages_ protected and releases the mutex
    //    automatically when the function scope ends.
    void log(std::string_view message) {
        if (message.empty()) {
            throw std::invalid_argument(
                "Message must not be empty!"
            );
        }
        std::lock_guard<std::mutex> lock(mutex_);
        messages_.emplace_back(message);
    }
    
    // Display all currently stored messages.
    // Note:
    //  - Access is synchronized because another thread may be logging at the
    //    same time.
    void show_logs() const {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const std::string& message : messages_) {
            std::cout << message << '\n';
        }
    }
};


// ============================================================================
// Demo / Entry Point
// ============================================================================

int main() {
    // Demonstrate that repeated calls to Logger::instance() return the same object.
    Logger& logger_a = Logger::instance();
    Logger& logger_b = Logger::instance();
    
    // Compare addresses to verify both references point to the singleton.
    if (&logger_a == &logger_b) {
        std::cout << "Logger a & logger b: same memory address." << '\n';
    } else {
        std::cout << "Logger a & logger b: different memory addresses." << '\n';
    }
    
    // Log through both references to demonstrate that they share the same state.
    logger_a.log("Application started");
    logger_b.log("Configuration loaded");
    logger_a.log("User authenticated");
    logger_b.log("Background task started");
    logger_a.log("Application shutting down");
    
    // Display the messages collected through the shared Logger.
    logger_a.show_logs();
    
    // Store the Logger addresses obtained independently by worker threads.
    const Logger* address_1 = nullptr;
    const Logger* address_2 = nullptr;
    
    // Each worker independently obtains the singleton and writes a message.
    std::thread worker_1([&address_1] {
        Logger& logger_1 = Logger::instance();
        address_1 = &logger_1;
        logger_1.log("Thread 1");
    });
    
    std::thread worker_2([&address_2] {
        Logger& logger_2 = Logger::instance();
        address_2 = &logger_2;
        logger_2.log("Thread 2");
    });
    
    // Wait until both worker threads have finished before reading their results.
    worker_1.join();
    worker_2.join();
    
    // Verify that both worker threads received the same Logger instance.
    if (address_1 == address_2) {
        std::cout << "Logger 1 & logger 2: same memory address." << '\n';
    } else {
        std::cout << "Logger 1 & logger 2: different memory addresses." << '\n';
    }
    
    // Verify that the worker-thread instance is also the same instance used
    // by the main thread.
    if (address_1 == &logger_a) {
        std::cout << "Logger 1 & logger a: same memory address." << '\n';
    } else {
        std::cout << "Logger 1 & logger a: different memory addresses." << '\n';
    }
    
    // Display the complete shared log, including messages from worker threads.
    logger_a.show_logs();
    
    return 0;
}
