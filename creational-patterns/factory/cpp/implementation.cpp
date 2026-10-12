class Notification {
public:
    virtual ~Notification() = default;
    virtual void send(std::string_view message) const = 0;
};


class EmailNotification : public Notification {
public:
    void send(std::string_view message) const override {
        std::cout 
            << "Sending email notification: " 
            << message 
            << ".\n";
    }
};


class PushNotification : public Notification {
public:
    void send(std::string_view message) const override {
        std::cout 
            << "Sending push notification: " 
            << message 
            << ".\n";
    }
};


class WhatsAppNotification : public Notification {
public:
    void send(std::string_view message) const override {
        std::cout 
            << "Sending WhatsApp notification: " 
            << message 
            << ".\n";
    }
};


class NotificationCreator {
public:
    virtual ~NotificationCreator() = default;
    virtual std::unique_ptr<Notification> create_notification() const = 0;
};


class EmailNotificationCreator : public NotificationCreator {
public:
    std::unique_ptr<Notification> create_notification() const override {
        return std::make_unique<EmailNotification>();
    }    
};


class PushNotificationCreator : public NotificationCreator {
public:
    std::unique_ptr<Notification> create_notification() const override {
        return std::make_unique<PushNotification>();
    }    
};


class WhatsAppNotificationCreator : public NotificationCreator {
public:
    std::unique_ptr<Notification> create_notification() const override {
        return std::make_unique<WhatsAppNotification>();
    }    
};
