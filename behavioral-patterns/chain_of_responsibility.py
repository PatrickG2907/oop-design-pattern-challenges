from dataclasses import dataclass
from abc import ABC, abstractmethod
from typing import Optional, Set, Dict
from enum import Enum, auto

# ------------------------------------
# Configuration
# ------------------------------------
class Severity(Enum):
    LOW = auto()
    MEDIUM = auto()
    HIGH = auto()
    CRITICAL = auto()

class Tag(Enum):
    URGENT = auto()
    VIP = auto()
    SECURITY = auto()

# ------------------------------------
# Request
# ------------------------------------
@dataclass(frozen=True)
class Request:
    severity: Severity
    name: str
    tag: Optional[Tag] = None

    def __post_init__(self) -> None:
        if not self.name:
            raise ValueError("Request name cannot be empty!")

    def with_escalation(self, new_severity: Severity) -> "Request":
        return Request(new_severity, self.name, self.tag)

# ------------------------------------
# Notifier Interface
# ------------------------------------
class Notifier(ABC):
    @abstractmethod
    def notify_handled(self, handler_name: str, request: Request) -> None: ...
    @abstractmethod
    def notify_routing(self, request: Request) -> None: ...
    @abstractmethod
    def notify_skipped(self, handler_name: str, request: Request) -> None: ...
    @abstractmethod
    def notify_error(self, request: Request) -> None: ...

class ConsoleNotifier(Notifier):
    def notify_handled(self, handler_name: str, request: Request) -> None:
        print(f"[HANDLED] {handler_name} processed request '{request.name}'\n")

    def notify_routing(self, request: Request) -> None:
        print(f"[Routing] Request '{request.name}' (severity: {request.severity}, tag: {request.tag})")

    def notify_skipped(self, handler_name: str, request: Request) -> None:
        print(f"[SKIPPED] {handler_name} skipped request '{request.name}'")

    def notify_error(self, request: Request) -> None:
        print(f"[ERROR] No handler could process request '{request.name}'\n")

class NullNotifier(Notifier):
    def notify_handled(self, handler_name: str, request: Request) -> None: pass
    def notify_routing(self, request: Request) -> None: pass
    def notify_skipped(self, handler_name: str, request: Request) -> None: pass
    def notify_error(self, request: Request) -> None: pass

# ------------------------------------
# Abstract Handler
# ------------------------------------
class AbstractHandler(ABC):
    def __init__(self, successor: Optional["AbstractHandler"] = None, notifier: Notifier = NullNotifier()) -> None:
        self.successor = successor
        self.notifier = notifier

    @abstractmethod
    def handle(self, request: Request) -> None: ...

# ------------------------------------
# Concrete Support Handler
# ------------------------------------
class SupportHandler(AbstractHandler):
    def __init__(self, name: str, supported_levels: Set[Severity], successor: Optional[AbstractHandler] = None, notifier: Notifier = NullNotifier()) -> None:
        super().__init__(successor, notifier)
        self.name = name
        self.supported_levels = supported_levels

    def handle(self, request: Request) -> None:
        if request.severity in self.supported_levels:
            self.notifier.notify_handled(self.name, request)
        else:
            self.notifier.notify_skipped(self.name, request)
            if self.successor:
                self.successor.handle(request)
            else:
                self.notifier.notify_error(request)

# ------------------------------------
# Escalator
# ------------------------------------
class Escalator:
    """Applies severity escalation rules based on request tags."""
    def __init__(self, rules: Dict[Tag, Severity]) -> None:
        self.rules = rules

    def escalate(self, request: Request) -> Request:
        if request.tag in self.rules:
            return request.with_escalation(self.rules[request.tag])
        return request

# ------------------------------------
# Router (only responsible for routing)
# ------------------------------------
class Router:
    def __init__(
        self,
        default_handler: AbstractHandler,
        entry_overrides: Dict[Tag, AbstractHandler],
        escalator: Escalator,
        notifier: Notifier = NullNotifier(),
    ) -> None:
        self.default_handler = default_handler
        self.entry_overrides = entry_overrides
        self.escalator = escalator
        self.notifier = notifier

    def route(self, request: Request) -> None:
        self.notifier.notify_routing(request)
        request = self.escalator.escalate(request)
        handler = self.entry_overrides.get(request.tag, self.default_handler)
        handler.handle(request)

# ------------------------------------
# Build Handler Chain
# ------------------------------------
notifier = ConsoleNotifier()

level3 = SupportHandler("Level3", {Severity.HIGH}, notifier=notifier)
level2 = SupportHandler("Level2", {Severity.MEDIUM}, successor=level3, notifier=notifier)
level1 = SupportHandler("Level1", {Severity.LOW}, successor=level2, notifier=notifier)

ESCALATION_RULES = {Tag.URGENT: Severity.HIGH, Tag.VIP: Severity.MEDIUM}
ENTRY_OVERRIDES = {Tag.URGENT: level3, Tag.VIP: level2}

router = Router(
    default_handler=level1,
    entry_overrides=ENTRY_OVERRIDES,
    escalator=Escalator(ESCALATION_RULES),
    notifier=notifier
)

# ------------------------------------
# Example Requests
# ------------------------------------
requests = [
    Request(severity=Severity.HIGH, name="Password Reset"),
    Request(severity=Severity.MEDIUM, name="Server Down", tag=Tag.URGENT),
    Request(severity=Severity.LOW, name="VIP Inquiry", tag=Tag.VIP),
    Request(severity=Severity.CRITICAL, name="Malware"),
]

for req in requests:
    router.route(req)
