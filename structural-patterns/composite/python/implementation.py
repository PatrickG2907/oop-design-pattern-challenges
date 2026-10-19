from abc import ABC, abstractmethod

# Abstract base class representing both Files and Folders
class FileSystemComponent(ABC):
    def __init__(self, name):
        self.name = name

    @abstractmethod
    def display(self, indent=0):
        """Display the component with indentation for hierarchy"""
        pass

    @abstractmethod
    def count_files(self):
        """Return the total number of files in this component"""
        pass

# Leaf class representing a File
class File(FileSystemComponent):
    def display(self, indent=0):
        print(" " * indent + f"- {self.name}")  # Simple display with indentation

    def count_files(self):
        return 1  # A file counts as one

# Composite class representing a Folder
class Folder(FileSystemComponent):
    def __init__(self, name):
        super().__init__(name)
        self.children = []  # Can contain Files or Folders

    def add(self, component):
        self.children.append(component)  # Add file or folder

    def display(self, indent=0):
        print(" " * indent + f"[{self.name}]")  # Display folder name
        for child in self.children:
            child.display(indent + 4)  # Indent child components

    def count_files(self):
        # Sum the file count recursively from all children
        return sum(child.count_files() for child in self.children)

# ------------------ Example Usage ------------------
if __name__ == "__main__":
    # Create files
    f1 = File("file1.txt")
    f2 = File("file2.txt")
    f3 = File("file3.txt")

    # Create folders and nest files/folders
    folderA = Folder("FolderA")
    folderB = Folder("FolderB")
    folderC = Folder("FolderC")

    folderA.add(f1)
    folderA.add(folderB)
    folderB.add(f2)
    folderB.add(folderC)
    folderC.add(f3)

    # Display full directory structure
    print("Directory Structure:")
    folderA.display()

    # Count total files
    print("\nTotal files in FolderA:", folderA.count_files())
