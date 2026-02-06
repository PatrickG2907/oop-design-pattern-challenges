from abc import ABC, abstractmethod

# ---------- Domain model ----------

class BookInterface(ABC):
    """Common interface so clients can treat Book and BookProxy the same."""
    @abstractmethod
    def get_content(self, user_role: str) -> str:
        pass


class Book(BookInterface):
    """The real, expensive-to-load object."""
    def __init__(self, title: str, author: str, content_loader):
        self.title = title
        self.author = author
        # content_loader simulates an expensive operation (disk/network)
        self._content_loader = content_loader
        self._content = None

    def _load_content(self):
        # Load only once (real book shouldn't reload either)
        if self._content is None:
            self._content = self._content_loader()
        return self._content

    def get_content(self, user_role: str) -> str:
        # Real book assumes access is already validated by proxy
        return self._load_content()


# ---------- Proxy ----------

class BookProxy(BookInterface):
    """Lazy-loading proxy with access control."""
    def __init__(self, title: str, author: str, content_loader):
        self.title = title
        self.author = author
        self._content_loader = content_loader
        self._real_book: Book | None = None  # Not loaded initially

    def _ensure_access(self, user_role: str):
        # Enforce access rules (twist)
        if user_role.lower() == "guest":
            raise PermissionError("Guests are not allowed to read this book.")

    def _load_real_book_if_needed(self):
        # Lazy initialization
        if self._real_book is None:
            print(f"[Proxy] Loading book '{self.title}' by {self.author}...")
            self._real_book = Book(
                title=self.title,
                author=self.author,
                content_loader=self._content_loader
            )

    def get_content(self, user_role: str) -> str:
        # 1) Check access
        self._ensure_access(user_role)
        # 2) Load the real book on first access only
        self._load_real_book_if_needed()
        # 3) Delegate to the real book (no reloading on future calls)
        return self._real_book.get_content(user_role)


# ---------- Example usage (for demonstration) ----------

def expensive_content_fetch():
    # Simulates disk/remote fetch
    return "Once upon a time... (very large book content)"

proxy = BookProxy(
    title="Design Patterns",
    author="GoF",
    content_loader=expensive_content_fetch
)

# Uncomment to test behavior:
# print(proxy.get_content("guest"))  # Raises PermissionError
print(proxy.get_content("member")) # Triggers loading message, then returns content
print(proxy.get_content("member")) # Uses cached book, no loading message
