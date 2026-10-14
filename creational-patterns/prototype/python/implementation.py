import copy

class Character:
    def __init__(self, name, health, attack_power, skills=None):
        # Validate name
        if not isinstance(name, str) or not name.strip():
            raise ValueError("Name must be a non-empty string.")
        self.name = name.strip()

        # Validate health
        if not isinstance(health, (int, float)) or health < 0:
            raise ValueError("Health must be a non-negative number.")
        self.health = health

        # Validate attack_power
        if not isinstance(attack_power, (int, float)) or attack_power < 0:
            raise ValueError("Attack power must be a non-negative number.")
        self.attack_power = attack_power

        # Validate skills
        if skills is None:
            self.skills = []
        elif isinstance(skills, list) and all(isinstance(skill, str) for skill in skills):
            self.skills = skills.copy()  # Make a shallow copy to avoid external mutation
        else:
            raise ValueError("Skills must be a list of strings.")

    def clone(self):
        """Deep clone the character, ensuring independent nested structures."""
        return copy.deepcopy(self)

    def __str__(self):
        return (f"Character(name={self.name}, health={self.health}, "
                f"attack_power={self.attack_power}, skills={self.skills})")


# --- Example Usage ---
try:
    base_character = Character("Warrior", 100, 20, ["Slash", "Block"])
    print("Base:", base_character)

    clone = base_character.clone()
    clone.skills.append("Berserk")
    clone.health = 120
    clone.name = "Warrior Clone"

    print("Clone:", clone)
    print("Original:", base_character)
except ValueError as e:
    print("Validation Error:", e)
