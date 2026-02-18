from dataclasses import dataclass, field
from typing import List
from enum import Enum, auto
from abc import ABC, abstractmethod

#--------------------------------------
# Enums and Configuration
#--------------------------------------
class Gender(Enum):
    MALE = auto()
    FEMALE = auto()
    OTHER = auto()

class Profiles(Enum):
    CHILD = auto()
    TEEN = auto()
    ADULT = auto()

#--------------------------------------
# Profile Assigner (Configurable)
#--------------------------------------
class ProfileAssigner:
    def __init__(self, age_profiles=None):
        # Default profiles if none provided
        self.age_profiles = age_profiles or [(0, 13, Profiles.CHILD),
                                             (14, 17, Profiles.TEEN),
                                             (18, 99, Profiles.ADULT)]

    def assign_profile(self, age: int) -> Profiles:
        for min_age, max_age, profile in self.age_profiles:
            if min_age <= age < max_age:
                return profile
        raise ValueError(f"No profile defined for age {age}")

#--------------------------------------
# User Class
#--------------------------------------
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
            raise ValueError("Name cannot be empty!")
        if self.age < 0:
            raise ValueError("Age cannot be negative!")

    def __str__(self):
        return f"{self.name} ({self.profile.name}, {self.gender.name})"

#--------------------------------------
# Notifications (Observer Pattern)
#--------------------------------------
class ISystemNotification(ABC):
    @abstractmethod
    def notify_add_user(self, name: str) -> None: pass
    @abstractmethod
    def notify_remove_user(self, name: str) -> None: pass
    @abstractmethod
    def notify_remove_error(self, name: str) -> None: pass

class SystemNotificationPrint(ISystemNotification):
    def notify_add_user(self, name: str) -> None:
        print(f"[Success] User {name} added!")

    def notify_remove_user(self, name: str) -> None:
        print(f"[Success] User {name} removed!")

    def notify_remove_error(self, name: str) -> None:
        print(f"[Error] User {name} cannot be removed!")

#--------------------------------------
# Notifier Interface & Service
#--------------------------------------
class INotifier(ABC):
    @abstractmethod
    def attach(self, observer: ISystemNotification) -> None: pass
    @abstractmethod
    def notify_add(self, user: User) -> None: pass
    @abstractmethod
    def notify_remove(self, user: User) -> None: pass
    @abstractmethod
    def notify_remove_error(self, user: User) -> None: pass

class NotifierService(INotifier):
    def __init__(self):
        self.observers: List[ISystemNotification] = []

    def attach(self, observer: ISystemNotification):
        self.observers.append(observer)

    def notify_add(self, user: User):
        for obs in self.observers:
            obs.notify_add_user(user.name)

    def notify_remove(self, user: User):
        for obs in self.observers:
            obs.notify_remove_user(user.name)

    def notify_remove_error(self, user: User):
        for obs in self.observers:
            obs.notify_remove_error(user.name)

#--------------------------------------
# User Management
#--------------------------------------
class IUserManagement(ABC):
    @abstractmethod
    def add_user(self, user: User) -> None: pass
    @abstractmethod
    def remove_user(self, user: User) -> None: pass
    @property
    @abstractmethod
    def users(self) -> List[User]: pass

class UserManagementList(IUserManagement):
    def __init__(self, notifier: INotifier):
        self._users: List[User] = []
        self.notifier = notifier

    @property
    def users(self) -> List[User]:
        return self._users

    def add_user(self, user: User) -> None:
        self._users.append(user)
        self.notifier.notify_add(user)

    def remove_user(self, user: User) -> None:
        try:
            self._users.remove(user)
            self.notifier.notify_remove(user)
        except ValueError:
            self.notifier.notify_remove_error(user)

#--------------------------------------
# Recipient Filters
#--------------------------------------
class IFilterRecipient(ABC):
    @abstractmethod
    def should_block(self, sender: User, receiver: User) -> bool: pass

class FilterAge(IFilterRecipient):
    def __init__(self, allowed_profiles: List[Profiles]):
        self.allowed_profiles = allowed_profiles

    def should_block(self, sender: User, receiver: User) -> bool:
        return receiver.profile not in self.allowed_profiles

class FilterGender(IFilterRecipient):
    def __init__(self, allowed_genders: List[Gender]):
        self.allowed_genders = allowed_genders

    def should_block(self, sender: User, receiver: User) -> bool:
        return receiver.gender not in self.allowed_genders

#--------------------------------------
# Message Filters
#--------------------------------------
class IFilterMessage(ABC):
    @abstractmethod
    def filter_message(self, message: str) -> str: pass

class BannedWordFilter(IFilterMessage):
    def __init__(self, banned_words: List[str]):
        self.banned_words = set(word.lower() for word in banned_words)

    def filter_message(self, message: str) -> str:
        words = message.split()
        filtered_words = [
            "***" if word.lower() in self.banned_words else word
            for word in words
        ]
        return " ".join(filtered_words)

#--------------------------------------
# Message Processor & Messenger (Testable)
#--------------------------------------
class IMessageOutput(ABC):
    @abstractmethod
    def send(self, text: str) -> None: pass

class ConsoleOutput(IMessageOutput):
    def send(self, text: str) -> None:
        print(text)

class MessageProcessor:
    def __init__(self, filters: List[IFilterMessage]):
        self.filters = filters

    def process(self, message: str) -> str:
        for f in self.filters:
            message = f.filter_message(message)
        return message

class ConsoleMessenger:
    def __init__(self, recipient_filters: List[IFilterRecipient], output: IMessageOutput):
        self.recipient_filters = recipient_filters
        self.output = output

    def send(self, sender: User, recipients: List[User], message: str):
        for receiver in recipients:
            if receiver == sender:
                continue
            if any(f.should_block(sender, receiver) for f in self.recipient_filters):
                continue
            self.output.send(f"{sender.name} to {receiver.name}: {message}")

#--------------------------------------
# Mediator
#--------------------------------------
class Mediator(ABC):
    @abstractmethod
    def add_user(self, user: User) -> None: pass
    @abstractmethod
    def remove_user(self, user: User) -> None: pass
    @abstractmethod
    def send_message(self, user: User, message: str) -> None: pass

#--------------------------------------
# Chat Room
#--------------------------------------
class ChatRoom(Mediator):
    def __init__(self, user_management: IUserManagement,
                 message_processor: MessageProcessor,
                 messenger: ConsoleMessenger):
        self.user_management = user_management
        self.message_processor = message_processor
        self.messenger = messenger

    def add_user(self, user: User) -> None:
        self.user_management.add_user(user)

    def remove_user(self, user: User) -> None:
        self.user_management.remove_user(user)

    def send_message(self, sender: User, message: str) -> None:
        if sender not in self.user_management.users:
            print(f"[Error] Sender {sender.name} is not in the chat room!")
            return
        processed_message = self.message_processor.process(message)
        self.messenger.send(sender, self.user_management.users, processed_message)

#--------------------------------------
# Setup Example
#--------------------------------------
# Notification
notifier_service = NotifierService()
notifier_service.attach(SystemNotificationPrint())

# User Management
user_mgmt = UserManagementList(notifier_service)

# Filters
banned_words_filter = BannedWordFilter(["foo", "bar"])
age_filter = FilterAge([Profiles.ADULT])
gender_filter = FilterGender([Gender.FEMALE])

# Messaging
processor = MessageProcessor([banned_words_filter])
output = ConsoleOutput()
messenger = ConsoleMessenger([age_filter, gender_filter], output)

# Chat Room
chat_room = ChatRoom(user_mgmt, processor, messenger)

# Users with custom profile assigner (optional)
custom_profiles = [(0, 10, Profiles.CHILD), (11, 17, Profiles.TEEN), (18, 120, Profiles.ADULT)]
assigner = ProfileAssigner(custom_profiles)

alice = User("Alice", 25, Gender.FEMALE, profile_assigner=assigner)
bob = User("Bob", 30, Gender.MALE, profile_assigner=assigner)
charlie = User("Charlie", 12, Gender.MALE, profile_assigner=assigner)
diana = User("Diana", 28, Gender.FEMALE, profile_assigner=assigner)

chat_room.add_user(alice)
chat_room.add_user(bob)
chat_room.add_user(charlie)
chat_room.add_user(diana)

# Messages
chat_room.send_message(alice, "Hello everyone! foo should be censored.")
chat_room.send_message(bob, "Hi Alice, bar is not allowed!")
chat_room.send_message(charlie, "Hi all, I am a child!")  # Blocked by age filter
chat_room.send_message(diana, "Hi Alice and Bob!")          # Bob blocked by gender filter
