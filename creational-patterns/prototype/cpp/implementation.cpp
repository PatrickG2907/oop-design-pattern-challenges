// -----------------------------------------------------------------------------
// Architecture Overview
// -----------------------------------------------------------------------------
// This example demonstrates the Prototype pattern by creating new Character
// objects as copies of an existing configured Character.
//
// Character acts as both the product and the prototype. clone() creates a new
// Character by copying the current object.
//
// The class uses value-semantic members:
//  - std::string
//  - int
//  - std::vector<std::string>
//
// Because std::string and std::vector perform deep value copies, the generated
// copy operation already gives each clone independent owned state. No custom
// copy constructor or manual heap allocation is required.
//
// After cloning, callers can modify a clone's name, health, attack power, or
// skills without affecting the original Character or other clones.
//
// This is an important C++ distinction: deep cloning does not necessarily
// require custom pointer-copying logic. When members already have correct value
// semantics, ordinary copying is the idiomatic solution.
// -----------------------------------------------------------------------------


#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// -----------------------------------------------------------------------------
// Character (Prototype)
// -----------------------------------------------------------------------------

class Character {
private:
    // Named bounds avoid magic numbers and make the valid ranges part of the
    // Character definition.
    // Notes:
    //  - Health may be zero, allowing zero to represent a valid "dead" state.
    //  - Attack power may also be zero, while the upper bounds are domain limits for
    //    this example.
    static constexpr int minimum_health_ = 0;
    static constexpr int maximum_health_ = 100;
    static constexpr int minimum_attack_power_ = 0;
    static constexpr int maximum_attack_power_ = 1000;

    std::string name_;
    int health_;
    int attack_power_;
    std::vector<std::string> skills_;
    
    // Validation helpers are private because they protect Character invariants and
    // are implementation details rather than part of the public API.
    // Notes:
    //  - They are static because validation depends only on the supplied value and
    //    does not require access to an existing Character object.
    //  - std::string_view is used for read-only string validation because no 
    //    ownership or copy is needed.
    static void validate_name(std::string_view name) {
        if (name.empty()) {
            throw std::invalid_argument("Name must not be empty!");
        }
    }
    
    static void validate_health(int health) {
        if (health < minimum_health_ || health > maximum_health_) {
            throw std::invalid_argument(
                "Health must be in the valid range: [" + 
                std::to_string(minimum_health_) + 
                ',' +
                std::to_string(maximum_health_) + 
                ']'
            );
        }
    }
    
    static void validate_attack_power(int attack_power) {
        if (
            attack_power < minimum_attack_power_ || 
            attack_power > maximum_attack_power_
        ) {
            throw std::invalid_argument(
                "Attack power must be in the valid range: [" + 
                std::to_string(minimum_attack_power_) + 
                ',' +
                std::to_string(maximum_attack_power_) + 
                ']'
            );
        }
    }
    
    // An empty skills list is valid, but every individual skill name must be
    // non-empty.
    // Note:
    //  - validate_skills() reuses validate_skill() so the same rule is applied 
    //    both when constructing a Character and when adding a skill later.
    static void validate_skill(std::string_view skill) {
        if (skill.empty()) {
            throw std::invalid_argument("Skill must not be empty!");
        }
    }
    
    static void validate_skills(const std::vector<std::string>& skills) {
        for (const std::string& skill : skills) {
            validate_skill(skill);
        }
    }

public:
    // Construct a fully owned Character and validate all supplied state.
    // Notes:
    //  - name and skills are accepted by value because Character will own them, then
    //    moved into the corresponding members.
    //  - health and attack_power are small value types and are copied.
    //  - Validation is performed on the initialized members. If validation throws,
    //    construction fails and C++ automatically destroys already-initialized
    //    members, so no invalid Character object can escape.
    Character(
        std::string name, 
        int health, 
        int attack_power, 
        std::vector<std::string> skills
    ) : name_(std::move(name)),
        health_(health),
        attack_power_(attack_power),
        skills_(std::move(skills))
    {
        validate_name(name_);
        validate_health(health_);
        validate_attack_power(attack_power_);
        validate_skills(skills_);
    }
    
    // Mutators preserve class invariants by validating first and committing the
    // change only after validation succeeds.
    // Note:
    //  - String values are taken by value and moved into storage because Character
    //    takes ownership of the new data.
    void set_name(std::string name) {
        validate_name(name);
        name_ = std::move(name);
    }
    
    void set_health(int health) {
        validate_health(health);
        health_ = health;
    }
    
    void set_attack_power(int attack_power) {
        validate_attack_power(attack_power);
        attack_power_ = attack_power;
    }
    
    // Adds one independently owned skill after validating it.
    // Note:
    //  - Taking the skill by value and moving it into skills_ avoids an additional
    //    copy of the local parameter.
    void add_skill(std::string skill) {
        validate_skill(skill);
        skills_.push_back(std::move(skill));
    }
    
    // Prototype: Creates a new Character by copying the current object.
    // Notes:
    //  - Returning by value is essential: it creates a distinct Character rather 
    //    than another reference to the same instance.
    //  - return *this invokes the normal copy operation. Because all members have
    //    value semantics, the clone receives its own std::string and
    //    std::vector<std::string> storage.
    //  - In particular, modifying a clone's skills_ does not affect the original
    //    Character's skills_.
    Character clone() const {
        return *this;
    }
    
    // Display the complete Character state without modifying the object.
    // Note:
    //  - An empty skills vector is represented explicitly as "None"; otherwise each
    //    owned skill is printed individually.
    void display() const {
        std::cout 
            << "Character:\n"
            << "\tName: " 
            << name_
            << '\n'
            << "\tHealth: "
            << health_ 
            << '\n'
            << "\tAttack Power: "
            << attack_power_
            << '\n'
            << "\tSkills: ";
        if (skills_.empty()) {
            std::cout << "None\n";
        } else {
            std::cout << '\n';
            for (const std::string& skill : skills_) {
                std::cout << "\t\t- " << skill << '\n';
            }
        }
    }
};


// -----------------------------------------------------------------------------
// Demo / Entry Point
// -----------------------------------------------------------------------------

// Demonstrates Prototype by creating one base Warrior and cloning it into two
// independent variants.
//
// Guardian and Berserker are both cloned directly from the same original
// Warrior, then modified independently.
//
// Each clone adds its own unique skill. Displaying the original afterward
// demonstrates that the copied skills vectors are independent and that clone
// mutations do not affect the prototype.
int main() {
    Character warrior(
        "Warrior",
        100,
        250,
        {"Slash", "Block", "Charge"}
    );
    warrior.display();
    
    Character guardian = warrior.clone();
    guardian.set_name("Guardian");
    guardian.set_attack_power(180);
    guardian.add_skill("Shield Bash");
    
    Character berserker = warrior.clone();
    berserker.set_name("Berserker");
    berserker.set_health(70);
    berserker.add_skill("Berserk");
    
    warrior.display();
    guardian.display();
    berserker.display();
    
    return 0;
}
