from abc import ABC, abstractmethod
from collections import deque

# === Base Command Interface ===
class Command(ABC):
    @abstractmethod
    def execute(self) -> None:
        pass

    @abstractmethod
    def undo(self) -> None:
        pass

    def safety_check(self) -> bool:
        """By default, commands are safe unless overridden."""
        return True

# === Concrete Commands ===
class PickItem(Command):
    def __init__(self, item: str) -> None:
        self.item = item
        self.picked = False

    def execute(self) -> None:
        print(f"Picking up {self.item}.")
        self.picked = True

    def undo(self):
        if self.picked:
            print(f"Undo picking {self.item}.")
            self.picked = False

    def safety_check(self):
        # Example: cannot pick if item is "fragile"
        return self.item != "fragile"

class PlaceItem(Command):
    def __init__(self, location: str) -> None:
        self.location = location
        self.placed = False

    def execute(self) -> None:
        print(f"Placing item at {self.location}.")
        self.placed = True

    def undo(self) -> None:
        if self.placed:
            print(f"Undo placing item at {self.location}.")
            self.placed = False

class WeldItem(Command):
    def __init__(self, part: str) -> None:
        self.part = part
        self.welded = False

    def execute(self) -> None:
        print(f"Welding {self.part}.")
        self.welded = True

    def undo(self) -> None:
        if self.welded:
            print(f"Undo welding {self.part}.")
            self.welded = False

    def safety_check(self) -> bool:
        # Example: welding requires safety goggles
        from random import choice
        safe = choice([True, False])  # Simulate safety check
        if not safe:
            print(f"Safety check failed for welding {self.part}.")
        return safe

# === Robot Controller ===
class RobotController:
    def __init__(self):
        self.command_queue = deque()
        self.retry_queue = deque()
        self.history = []

    def add_command(self, command: Command) -> None:
        self.command_queue.append(command)

    def execute_commands(self) -> None:
        while self.command_queue:
            command = self.command_queue.popleft()
            try:
                if command.safety_check():
                    command.execute()
                    self.history.append(command)
                else:
                    print(f"Command {command.__class__.__name__} deferred to retry queue.")
                    self.retry_queue.append(command)
            except Exception as e:
                print(f"Execution failed: {e}, undoing command.")
                command.undo()

    def retry_commands(self) -> None:
        print("\nRetrying deferred commands...")
        self.command_queue = self.retry_queue
        self.retry_queue = deque()
        self.execute_commands()

    def undo_last(self) -> None:
        if self.history:
            command = self.history.pop()
            command.undo()

# === Example Usage ===
if __name__ == "__main__":
    controller = RobotController()

    controller.add_command(PickItem("widget"))
    controller.add_command(WeldItem("frame"))
    controller.add_command(PickItem("fragile"))  # Should fail safety check
    controller.add_command(PlaceItem("station A"))

    controller.execute_commands()
    controller.retry_commands()

    print("\nUndoing last action:")
    controller.undo_last()
