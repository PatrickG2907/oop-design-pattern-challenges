from dataclasses import dataclass, field
from typing import List
from enum import Enum, auto
from abc import ABC, abstractmethod

# --------------------------------------
# Enums
# --------------------------------------

class Gender(Enum):
    MALE = auto()
    FEMALE = auto()
    OTHER = auto()

class Profiles(Enum):
    CHILD = auto()
    TEEN = auto()
    ADULT = auto()

# --------------------------------------
# Profile Assigner
# --------------------------------------

class ProfileAssigner:
    def __init__(self, age_profiles=None):
        self.age_profiles = age_profiles or [
            (0, 13, Profiles.CHILD),
            (13, 18, Profiles.TEEN),
            (18, 150, Profiles.ADULT),
        ]

    def assign_profile(self, age: int) -> Profiles:
        for min_age, max_age, profile in self.age_profiles:
            if min_age <= age < max_age:
                return profile
        raise ValueError(f"No profile defined for age {age}")

# --------------------------------------
# User Entity
# --------------------------------------

@dataclass
class User:
    name: str
    age: int
    gender: Gender
    profile_assigner: ProfileAssigner = field(default_factory=ProfileAssigner)
    profile: Profiles = field(init=False)

    def __post_init__(self):
        self._validate()
        self.profile = self.profile_assigner.assign_profile(self.age)

    def _validate(self):
        if not self.name:
            raise ValueError("Name cannot be empty")
        if self.age < 0:
            raise ValueError("Age cannot be negative")

    def __str__(self):
        return f"{self.name} ({self.profile.name}, {self.gender.name})"

# --------------------------------------
# Message Entity
# --------------------------------------

@dataclass
class Message:
    sender: User
    content: str

# --------------------------------------
# Notifications (Observer Pattern)
# --------------------------------------

class ISystemNotification(ABC):
    @abstractmethod
    def notify_add_user(self, name: str) -> None:
        pass

    @abstractmethod
    def notify_remove_user(self, name: str) -> None:
        pass

    @abstractmethod
    def notify_remove_error(self, name: str) -> None:
        pass

class SystemNotificationPrint(ISystemNotification):
    def notify_add_user(self, name: str) -> None:
        print(f"[Success] User {name} added!")

    def notify_remove_user(self, name: str) -> None:
        print(f"[Success] User {name} removed!")

    def notify_remove_error(self, name: str) -> None:
        print(f"[Error] User {name} cannot be removed!")

class INotifier(ABC):
    @abstractmethod
    def attach(self, observer: ISystemNotification) -> None:
        pass

    @abstractmethod
    def notify_add(self, user: User) -> None:
        pass

    @abstractmethod
    def notify_remove(self, user: User) -> None:
        pass

    @abstractmethod
    def notify_remove_error(self, user: User) -> None:
        pass

class NotifierService(INotifier):
    def __init__(self):
        self._observers: List[ISystemNotification] = []

    def attach(self, observer: ISystemNotification):
        self._observers.append(observer)

    def notify_add(self, user: User):
        for obs in self._observers:
            obs.notify_add_user(user.name)

    def notify_remove(self, user: User):
        for obs in self._observers:
            obs.notify_remove_user(user.name)

    def notify_remove_error(self, user: User):
        for obs in self._observers:
            obs.notify_remove_error(user.name)

# --------------------------------------
# User Management
# --------------------------------------

class IUserManagement(ABC):
    @abstractmethod
    def add_user(self, user: User) -> None:
        pass

    @abstractmethod
    def remove_user(self, user: User) -> None:
        pass

    @property
    @abstractmethod
    def users(self):
        pass

class UserManagementList(IUserManagement):
    def __init__(self, notifier: INotifier):
        self._users: List[User] = []
        self._notifier = notifier

    @property
    def users(self):
        return tuple(self._users)

    def add_user(self, user: User) -> None:
        self._users.append(user)
        self._notifier.notify_add(user)

    def remove_user(self, user: User) -> None:
        if user in self._users:
            self._users.remove(user)
            self._notifier.notify_remove(user)
        else:
            self._notifier.notify_remove_error(user)

# --------------------------------------
# Message Filtering
# --------------------------------------

class IFilterMessage(ABC):
    @abstractmethod
    def filter(self, message: Message) -> Message:
        pass

class BannedWordFilter(IFilterMessage):
    def __init__(self, banned_words: List[str]):
        self._banned_words = set(w.lower() for w in banned_words)

    def filter(self, message: Message) -> Message:
        words = message.content.split()
        filtered = [
            "***" if w.lower() in self._banned_words else w
            for w in words
        ]
        message.content = " ".join(filtered)
        return message

class IMessageProcessor(ABC):
    @abstractmethod
    def process(self, message: Message) -> Message:
        pass

class MessageProcessor(IMessageProcessor):
    def __init__(self, filters: List[IFilterMessage]):
        self._filters = filters

    def process(self, message: Message) -> Message:
        for f in self._filters:
            message = f.filter(message)
        return message

# --------------------------------------
# Recipient Filtering
# --------------------------------------

class IFilterRecipient(ABC):
    @abstractmethod
    def should_block(self, sender: User, receiver: User) -> bool:
        pass

class FilterAge(IFilterRecipient):
    def __init__(self, allowed_profiles: List[Profiles]):
        self._allowed_profiles = allowed_profiles

    def should_block(self, sender: User, receiver: User) -> bool:
        return receiver.profile not in self._allowed_profiles

class FilterGender(IFilterRecipient):
    def __init__(self, allowed_genders: List[Gender]):
        self._allowed_genders = allowed_genders

    def should_block(self, sender: User, receiver: User) -> bool:
        return receiver.gender not in self._allowed_genders

# --------------------------------------
# Messaging
# --------------------------------------

class IMessageOutput(ABC):
    @abstractmethod
    def send(self, text: str) -> None:
        pass

class ConsoleOutput(IMessageOutput):
    def send(self, text: str) -> None:
        print(text)

class IMessenger(ABC):
    @abstractmethod
    def send(self, message: Message, recipients: List[User]) -> None:
        pass

class ConsoleMessenger(IMessenger):
    def __init__(self,
                 recipient_filters: List[IFilterRecipient],
                 output: IMessageOutput):
        self._recipient_filters = recipient_filters
        self._output = output

    def send(self, message: Message, recipients: List[User]) -> None:
        for receiver in recipients:
            if receiver == message.sender:
                continue
            if any(f.should_block(message.sender, receiver)
                   for f in self._recipient_filters):
                continue
            self._output.send(
                f"{message.sender.name} → {receiver.name}: {message.content}"
            )

# --------------------------------------
# Mediator (High-Level Policy)
# --------------------------------------

class Mediator(ABC):
    @abstractmethod
    def add_user(self, user: User) -> None:
        pass

    @abstractmethod
    def remove_user(self, user: User) -> None:
        pass

    @abstractmethod
    def send_message(self, sender: User, content: str) -> None:
        pass

class ChatRoom(Mediator):
    def __init__(self,
                 user_management: IUserManagement,
                 message_processor: IMessageProcessor,
                 messenger: IMessenger):
        self._user_management = user_management
        self._message_processor = message_processor
        self._messenger = messenger

    def add_user(self, user: User) -> None:
        self._user_management.add_user(user)

    def remove_user(self, user: User) -> None:
        self._user_management.remove_user(user)

    def send_message(self, sender: User, content: str) -> None:
        if sender not in self._user_management.users:
            print(f"[Error] Sender {sender.name} is not in the chat room!")
            return

        message = Message(sender=sender, content=content)
        processed = self._message_processor.process(message)
        self._messenger.send(processed, self._user_management.users)

# --------------------------------------
# Example Usage
# --------------------------------------

if __name__ == "__main__":
    notifier = NotifierService()
    notifier.attach(SystemNotificationPrint())

    user_mgmt = UserManagementList(notifier)

    processor: IMessageProcessor = MessageProcessor([
        BannedWordFilter(["foo", "bar"])
    ])

    messenger: IMessenger = ConsoleMessenger(
        recipient_filters=[
            FilterAge([Profiles.ADULT]),
            FilterGender([Gender.FEMALE])
        ],
        output=ConsoleOutput()
    )

    chat_room = ChatRoom(user_mgmt, processor, messenger)

    alice = User("Alice", 25, Gender.FEMALE)
    bob = User("Bob", 30, Gender.MALE)
    diana = User("Diana", 28, Gender.FEMALE)

    chat_room.add_user(alice)
    chat_room.add_user(bob)
    chat_room.add_user(diana)

    chat_room.send_message(alice, "Hello foo world!")
    chat_room.send_message(bob, "Hi bar Alice!")
