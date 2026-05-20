from abc import ABC, abstractmethod

# -----------------------------
# Component Interface
# -----------------------------
class Notification(ABC):
    @abstractmethod
    def send(self):
        pass

# -----------------------------
# Concrete Component
# -----------------------------
class EmailNotification(Notification):
    def send(self):
        print("Sending Email notification")

# -----------------------------
# Base Decorator
# -----------------------------
class NotificationDecorator(Notification):
    def __init__(self, wrapped_notification: Notification):
        self._wrapped = wrapped_notification

    def send(self):
        # Delegate to the wrapped notification
        self._wrapped.send()

# -----------------------------
# Concrete Decorators
# -----------------------------
class SMSDecorator(NotificationDecorator):
    def send(self):
        super().send()
        print("Sending SMS notification")


class PushDecorator(NotificationDecorator):
    def send(self):
        super().send()
        print("Sending Push notification")

# -----------------------------
# Logging Decorator (Twist)
# -----------------------------
class LoggingDecorator(NotificationDecorator):
    def __init__(self, wrapped_notification: Notification, notification_type: str):
        super().__init__(wrapped_notification)
        self.notification_type = notification_type

    def send(self):
        # Log BEFORE sending
        print(f"[LOG] Notification type: {self.notification_type}")
        super().send()

# -----------------------------
# Client Code
# -----------------------------
if __name__ == "__main__":
    # Base email notification
    notification = EmailNotification()

    # Add logging + SMS + Push without modifying existing classes
    notification = LoggingDecorator(notification, "Email")
    notification = SMSDecorator(notification)
    notification = LoggingDecorator(notification, "SMS")
    notification = PushDecorator(notification)
    notification = LoggingDecorator(notification, "Push")

    # Send notification through all channels
    notification.send()
