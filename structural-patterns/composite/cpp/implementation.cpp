// -----------------------------------------------------------------------------
// Architecture Overview
// -----------------------------------------------------------------------------

// Composite Pattern
//
// Models a tree structure while allowing individual objects and groups of
// objects to be treated uniformly through the same interface.
//
// Roles:
//  - FileSystemComponent -> Component
//  - File                -> Leaf
//  - Folder              -> Composite
//
// Both File and Folder implement the same operations: display() and size().
// The client therefore does not need to distinguish between a single File
// and an entire Folder subtree.
//
// Folder owns a collection of FileSystemComponent objects. Because Folder is
// itself a FileSystemComponent, folders can contain files or other folders,
// creating a recursive tree structure.
//
// Recursive operations are handled inside the Composite:
//  - File::size() returns its own size.
//  - Folder::size() asks every child for its size and sums the results.
//  - Folder::display() delegates recursively to all children.
//
// This keeps traversal logic out of the client. Calling display() or size()
// on the root automatically processes the entire subtree.


#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// -----------------------------------------------------------------------------
// Component
// -----------------------------------------------------------------------------


// Common interface for both Leaves and Composites.
// Note:
//  - Client code can operate on FileSystemComponent without knowing whether
//    the concrete object is a File or a Folder.
class FileSystemComponent {
public:
    virtual ~FileSystemComponent() = default;
    
    // depth is used only for recursive display formatting.
    virtual void display(std::size_t depth = 0) const = 0;
    
    // Uniform size operation: File returns its own size, while Folder 
    // calculates recursively.
    virtual std::size_t size() const = 0;
};


// -----------------------------------------------------------------------------
// Leaf
// -----------------------------------------------------------------------------

// Leaf.
// Note:
//  - Represents an individual file and has no child components.
class File : public FileSystemComponent {
private:
    std::string name_;
    std::size_t size_;
    
    static void validate_name(std::string_view name) {
        if (name.empty()) {
            throw std::invalid_argument("File name must not be empty!");
        }
    }
    
public:
    File(std::string name, std::size_t size)
        : name_(std::move(name)), size_(size)
    {
        validate_name(name_);
    }
    
    // A Leaf handles the operation directly; no recursion is required.
    void display(std::size_t depth = 0) const override {
        std::cout
            << std::string(depth * 2, ' ')
            << "File name: " 
            << name_ 
            << ", File size: "
            << size_
            << '\n';
    }
    
    // A File contributes only its own size.
    std::size_t size() const override {
        return size_;
    }
};


// -----------------------------------------------------------------------------
// Composite
// -----------------------------------------------------------------------------

// Composite.
// Note:
//  - Owns child Components rather than specifically Files, so each child can be
//    either a File or another Folder.
class Folder : public FileSystemComponent {
private:
    std::string name_;
    
    // Polymorphic ownership.
    // Notes:
    //  - unique_ptr expresses exclusive ownership while allowing polymorphic
    //    children to be stored through the FileSystemComponent base type.
    //  - Because Folder also derives from FileSystemComponent, this container
    //    naturally supports recursive nesting.
    std::vector<std::unique_ptr<FileSystemComponent>> children_;
    
    static void validate_name(std::string_view name) {
        if (name.empty()) {
            throw std::invalid_argument("Folder name must not be empty!");
        }
    }
    
public:
    explicit Folder(std::string name) 
        : name_(std::move(name))
    {
        validate_name(name_);
    }
    
    // Transfers ownership of a child into this Folder.
    // Note:
    //  - Null children are rejected because every stored component must be usable.
    void add(std::unique_ptr<FileSystemComponent> component) {
        if (!component) {
            throw std::invalid_argument("Component must not be null!");
        }
        children_.push_back(std::move(component));
    }
    
    // Displays this Folder, then recursively delegates display() to every child.
    // Note:
    //  - Each child receives depth + 1 so nested levels are indented correctly.
    void display(std::size_t depth = 0) const override {
        std::cout
            << std::string(depth * 2, ' ')
            << "Folder name: "
            << name_
            << '\n';
        for (const auto& component : children_) {
            component->display(depth + 1);
        }
    }
    
    // Recursively asks every child for its size.
    // Note:
    //  - Virtual dispatch handles Files and nested Folders uniformly.
    std::size_t size() const override {
        std::size_t total_size = 0;
        for (const auto& component : children_) {
            total_size += component->size();
        }
        return total_size;
    }
};


// -----------------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------------

// Build a tree of Components:
//
// root/
// ├── readme.txt
// └── documents/
//     ├── notes.txt
//     └── archive/
//         └── report.pdf
int main() {
    Folder root("root");
    
    root.add(std::make_unique<File>("readme.txt", 100));
    
    auto archive = std::make_unique<Folder>("archive");
    archive->add(std::make_unique<File>("report.pdf", 300));
    
    auto documents = std::make_unique<Folder>("documents");
    documents->add(std::make_unique<File>("notes.txt", 200));
    
    // unique_ptr is move-only: ownership is transferred upward through the tree.
    documents->add(std::move(archive));
    root.add(std::move(documents));
    
    // Client code operates only on the root Component.
    // Note:
    //  - Recursive traversal is handled internally by the Composite structure.
    root.display();
    
    std::cout << "Total size: " << root.size() << '\n';
    
    return 0;
}
