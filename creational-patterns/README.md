# 🏗️ Creational OOP Patterns Challenges

Welcome to the **Creational OOP Patterns Challenges** repository! 🎉  

This repository contains a set of self-designed programming challenges focused on **Creational Design Patterns** in Object-Oriented Programming (OOP). These exercises will help you understand and implement patterns that deal with object creation in a structured and flexible way.  

---

## 📑 Table of Contents

- [Singleton Pattern](#singleton-pattern)  
- [Factory Method Pattern](#factory-method-pattern)  
- [Abstract Factory Pattern](#abstract-factory-pattern)  
- [Builder Pattern](#builder-pattern)  
- [Prototype Pattern](#prototype-pattern)  
- [Getting Started](#getting-started)  
- [Contributing](#contributing)  
- [License](#license)  

---

## 1️⃣ Singleton Pattern

**Scenario:**  
You are building a logging system for a multi-threaded application. Only one logger should exist, and it should keep a list of log messages.  

**Tasks:**  
- Implement a **thread-safe Singleton Logger**.  
- Add a method `log(message)` to append messages.  
- Add a method `show_logs()` to display all messages.  
- Demonstrate that multiple attempts to create a Logger return the **same instance**.  

**Twist:**  
- Try creating multiple loggers in **different threads** and ensure they all share the same instance.  

---

## 2️⃣ Factory Method Pattern

**Scenario:**  
You are designing a notification system that can send messages via **Email** and **Push Notification**.  

**Tasks:**  
- Define a `Notification` interface with a `send()` method.  
- Create concrete classes for `EmailNotification` and `PushNotification`.  
- Implement a `NotificationFactory` class with a factory method to create notifications based on a type string.  
- Demonstrate creating and sending different types of notifications using the factory.  

**Twist:**  
- Add a new notification type later (like **WhatsApp**) **without modifying the original factory class logic**.  

---

## 3️⃣ Abstract Factory Pattern

**Scenario:**  
You are designing a UI toolkit that supports **Light Mode** and **Dark Mode**. Each mode has **Buttons** and **Checkboxes**.  

**Tasks:**  
- Create abstract products: `Button` and `Checkbox`.  
- Create concrete products for `LightButton`, `DarkButton`, `LightCheckbox`, `DarkCheckbox`.  
- Implement `LightFactory` and `DarkFactory` to create matching components.  
- Write a client function that creates a full UI **without knowing concrete classes**.  

**Twist:**  
- Add a new component type **Slider**, while keeping the abstract factory structure intact.  

---

## 4️⃣ Builder Pattern

**Scenario:**  
You are building a **custom pizza ordering system** where a pizza can have:  
- **Size:** Small, Medium, Large  
- **Crust type:** Thin, Thick, Stuffed  
- **Toppings:** multiple options  

**Tasks:**  
- Implement a `PizzaBuilder` that allows building a pizza **step by step**.  
- Implement a `Director` that can create default pizzas like Margherita or Pepperoni.  
- Make sure the `Pizza` object can display its ingredients.  

**Twist:**  
- Allow customers to **skip steps** (like no toppings) and still produce a valid pizza.  

---

## 5️⃣ Prototype Pattern

**Scenario:**  
You are designing a **game character system**. Each character has:  
- **Name**  
- **Health**  
- **Attack Power**  
- **Skills** (list of strings)  

**Tasks:**  
- Implement a `Character` class that supports **cloning**.  
- Create a base character and clone it to produce multiple variants.  
- Modify some properties of the clones (like health or skills) **without affecting the original**.  

**Twist:**  
- Implement **deep cloning** to ensure nested structures (like a list of skills) are independent in each clone.  

---

## 🚀 Getting Started

1. Clone the repository:  
```bash
git clone https://github.com/your-username/creational-oop-patterns.git
