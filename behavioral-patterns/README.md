# Design Patterns Challenges Repository

This repository contains hands-on challenges and implementations for common software design patterns. Each challenge includes a real-world scenario, tasks to implement, and a twist to make it more interesting.

---

## 1. Strategy Pattern
**Challenge:**  
**Description:**  
- **Scenario:** You are developing an RPG where enemies can behave differently depending on the environment.  
- **Tasks:**  
  - Implement an `Enemy` class that can attack using different strategies: `AggressiveAttack`, `DefensiveAttack`, `HitAndRunAttack`.  
  - The attack strategy can be switched at runtime depending on the enemy’s health or player proximity.  
  - Each strategy should implement an `execute_attack(player)` method.  
- **Twist:** Add a special “berserk” mode if health < 20% that temporarily overrides any strategy.  

---

## 2. Observer Pattern
**Challenge:**  
**Description:**  
- **Scenario:** You are building a weather station system where multiple display devices need updates when the weather changes.  
- **Tasks:**  
  - Implement a `WeatherStation` class that holds temperature and humidity.  
  - Implement `DisplayDevice` classes that subscribe to the WeatherStation and update when weather changes.  
  - Allow multiple display types (e.g., `PhoneDisplay`, `WindowDisplay`).  
- **Twist:** Add a priority system. Some displays only get updates if the temperature changes more than 2°C.  

---

## 3. Command Pattern
**Challenge:**  
**Description:**  
- **Scenario:** A robot factory uses commands to perform tasks on an assembly line.  
- **Tasks:**  
  - Implement commands like `PickItem`, `PlaceItem`, `WeldItem`.  
  - Implement a `RobotController` class that queues commands and executes them sequentially.  
  - Each command should be undoable if it fails.  
- **Twist:** Some commands require a safety check before execution. If the safety check fails, the command should go to a “retry later” queue.  

---

## 4. State Pattern
**Challenge:**  
**Description:**  
- **Scenario:** You are designing a media player that can be in states: `Stopped`, `Playing`, `Paused`.  
- **Tasks:**  
  - Implement a `MediaPlayer` class with state transitions.  
  - Each state should define what actions are valid (`play`, `pause`, `stop`).  
- **Twist:** Add a shuffle mode. When active, play randomly picks the next track. Ensure the state logic still works correctly with shuffle.  

---

## 5. Template Method
**Challenge:**  
**Description:**  
- **Scenario:** You are building a quiz system where quizzes have a common flow but different rules for scoring or feedback.  
- **Tasks:**  
  - Create an abstract `Quiz` class with a template method `take_quiz()` that defines the steps:  
    1. Display questions  
    2. Collect answers  
    3. Score answers  
    4. Show feedback  
  - Subclasses implement scoring differently: `MathQuiz`, `HistoryQuiz`, `CodingQuiz`.  
- **Twist:** Some quizzes have bonus questions that are only scored if the student gets the main questions right. Include hooks to allow optional steps.  

---

## 6. Iterator Pattern
**Challenge:**  
**Description:**  
- **Scenario:** Music Playlist.  
- **Tasks:**  
  - Create a `Playlist` class that contains `Song` objects.  
  - Implement an iterator for: Normal play order & Shuffle mode.  
  - Allow the iterator to skip songs based on a user preference (e.g., skip explicit tracks).  
- **Twist:** Implement a “repeat n times” iterator that can loop a subset of the playlist multiple times.  

---

## 7. Chain of Responsibility
**Challenge:**  
**Description:**  
- **Scenario:** A customer support system has multiple support tiers: `Level1`, `Level2`, `Level3`.  
- **Tasks:**  
  - Implement a `SupportHandler` class. Each handler can either handle a ticket or pass it up the chain.  
  - Each handler should handle issues of different severity (low, medium, high).  
- **Twist:** Some tickets have keywords that skip levels. For example, "urgent" tickets go straight to Level3; "vip" tickets go straight to Level2.  

---

## 8. Mediator Pattern
**Challenge:**  
**Description:**  
- **Scenario:** You are building a chat application where users communicate through a chat room.  
- **Tasks:**  
  - Implement `ChatRoom` as a mediator. Users send messages through the chat room, not directly.  
  - Users can join or leave the chat room dynamically.  
- **Twist:** Add a filter system in the mediator. Some words are banned, and the mediator automatically censors them before sending messages.  

---

## 9. Memento Pattern
**Challenge:**  
**Description:**  
- **Scenario:** Drawing Application.  
- **Tasks:**  
  - Implement a `Drawing` class with shapes (`Circle`, `Rectangle`).  
  - Implement a Memento to store snapshots of the drawing.  
  - Add undo/redo functionality using mementos.  
- **Twist:** Some shapes are linked (like a group). Undoing a change to one shape should optionally restore the linked shapes too.  

---

## 10. Visitor Pattern
**Challenge:**  
**Description:**  
- **Scenario:** E-commerce Order System.  
- **Tasks:**  
  - Define an `OrderItem` hierarchy: `PhysicalProduct`, `DigitalProduct`, `Service`.  
  - Implement visitors: `TaxCalculator`, `ShippingCostCalculator`, `DiscountApplier`.  
  - Each visitor applies operations differently depending on the item type.  
- **Twist:** Implement a visitor chain. The result of one visitor affects the next (e.g., discount affects tax calculation).  

---

## License
This repository is open-source and available under the MIT License.  
