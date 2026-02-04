# OOP Patterns Challenges

This repository contains a collection of **self-designed challenges** focused on **structural and behavioral Object-Oriented Programming (OOP) patterns**. Each challenge provides a practical scenario, tasks to implement, and an optional twist to extend the complexity. These challenges are ideal for learning, practicing, and mastering OOP patterns in real-world scenarios.

---

## Table of Contents
1. [Adapter Pattern](#adapter-pattern)
2. [Bridge Pattern](#bridge-pattern)
3. [Composite Pattern](#composite-pattern)
4. [Decorator Pattern](#decorator-pattern)
5. [Facade Pattern](#facade-pattern)
6. [Flyweight Pattern](#flyweight-pattern)
7. [Proxy Pattern](#proxy-pattern)

---

## Adapter Pattern

**Scenario:**  
Integrate a new payment gateway into an existing e-commerce platform. The platform expects a `pay(amount)` method, but the gateway only provides `make_payment(total)`.

**Tasks:**  
- Create an Adapter that allows the platform to use the new gateway without modifying existing code.  
- Test the adapter by processing a payment.

**Twist:**  
The gateway sometimes returns `"OK"` (string) or `True` (boolean) for success. Make your adapter **normalize the output** so the platform always receives a boolean.

---

## Bridge Pattern

**Scenario:**  
Separate abstraction from implementation for flexible payment processing. This allows the platform to support multiple payment methods without modifying high-level code.

**Tasks:**  
- Implement a Payment abstraction and different gateway implementations.  
- Connect them via a Bridge so the platform can use multiple gateways interchangeably.  
- Test payments through multiple implementations.

**Twist:**  
Support dynamically swapping payment gateways at runtime without changing client code.

---

## Composite Pattern

**Scenario:**  
Model a **file system structure** where both files and folders need to be treated uniformly. A folder can contain files or other folders.

**Tasks:**  
- Implement a Component interface.  
- Implement `File` and `Folder` classes.  
- Allow folders to contain other components and perform operations (e.g., display, size calculation) uniformly.

**Twist:**  
Add a feature to calculate total size or perform batch operations recursively without changing client code.

---

## Decorator Pattern

**Scenario:**  
Implement a notification system. The base notification sends an email. Later, SMS and Push notifications are added optionally.

**Tasks:**  
- Implement the base `Notification` class.  
- Implement `SMSDecorator` and `PushDecorator` to extend behavior.  
- Send a notification that goes via **Email + SMS + Push**.

**Twist:**  
Implement a decorator that **logs the notification type** every time it is sent, without modifying existing decorators or the base class.

---

## Facade Pattern

**Scenario:**  
Create a home automation system with multiple subsystems: `Lights`, `AC`, `Music`. Each subsystem has multiple methods (`on()`, `off()`, `set_temperature()`, `play_song()`).

**Tasks:**  
- Implement a Facade class `SmartHome` with simplified methods like `start_party()` and `end_party()`.  
- Use the Facade to start and end a party.

**Twist:**  
Add **Vacation Mode** that schedules lights and AC automatically without exposing underlying subsystems.

---

## Flyweight Pattern

**Scenario:**  
Develop a text editor that displays thousands of characters. Each character has a font style (bold, italic) and color. Storing each character individually wastes memory.

**Tasks:**  
- Implement a Flyweight for character styles.  
- Implement a factory to reuse styles.  
- Display text with multiple characters sharing the same style.

**Twist:**  
Add **dynamic highlighting** for a few characters without creating new style objects, ensuring memory efficiency.

---

## Proxy Pattern

**Scenario:**  
Implement a large image viewer. Loading full-resolution images is slow. Display a placeholder first, then load the real image on demand.

**Tasks:**  
- Implement a Proxy that shows a placeholder initially.  
- Load the real image only when `display()` is called.

**Twist:**  
Add **access control**: only users with a `"premium"` role can view high-resolution images; others see a low-resolution version.

---

## How to Use

1. Clone the repository:
```bash
git clone https://github.com/your-username/oop-patterns-challenges.git

