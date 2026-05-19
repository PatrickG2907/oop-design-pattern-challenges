from abc import ABC, abstractmethod

# ----- Step 1: Notification interface -----
class Notification(ABC):
    @abstractmethod
    def send(self, message: str):
        pass

# ----- Step 2: Concrete notifications -----
class EmailNotification(Notification):
    def send(self, message: str):
        print(f"Sending Email: {message}")

class PushNotification(Notification):
    def send(self, message: str):
        print(f"Sending Push Notification: {message}")

class WhatsAppNotification(Notification):
    def send(self, message: str):
        print(f"Sending WhatsApp message: {message}")

# ----- Step 3: Instance-based Notification Factory -----
class NotificationFactory:
    def __init__(self):
        self._creators = {}

    def register_notification(self, type_name: str, creator):
        self._creators[type_name] = creator

    def create_notification(self, type_name: str) -> Notification:
        if type_name not in self._creators:
            raise ValueError(f"Notification type '{type_name}' not registered.")
        return self._creators[type_name]()

# ----- Step 4: Create factory instance and register types -----
factory = NotificationFactory()
factory.register_notification("email", EmailNotification)
factory.register_notification("push", PushNotification)
factory.register_notification("whatsapp", WhatsAppNotification)

# ----- Step 5: Demo sending notifications -----
notifications_to_send = [
    ("email", "Hello via Email!"),
    ("push", "Hello via Push!"),
    ("whatsapp", "Hello via WhatsApp!")
]

for n_type, message in notifications_to_send:
    notification = factory.create_notification(n_type)
    notification.send(message)

