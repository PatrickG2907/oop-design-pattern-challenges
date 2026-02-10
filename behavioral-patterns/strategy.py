from abc import ABC, abstractmethod
import random

# ----------------------
# Player Class
# ----------------------
class Player:
    def __init__(self, health: int, position: int) -> None:
        self.health = health
        self.position = position

    def move(self, step: int) -> None:
        self.position += step

    def take_damage(self, damage: int) -> None:
        self.health = max(0, self.health - damage)
        print(f"Player takes {damage} damage! Health now {self.health}.")

# ----------------------
# Attack Strategies
# ----------------------
class Strategy(ABC):
    @abstractmethod
    def execute_attack(self, enemy, player: Player) -> None: pass

class AggressiveAttack(Strategy):
    def execute_attack(self, enemy, player: Player) -> None:
        damage = random.randint(15, 30)
        print(f"{enemy.name} aggressively attacks for {damage} damage!")
        player.take_damage(damage)

class DefensiveAttack(Strategy):
    def execute_attack(self, enemy, player: Player) -> None:
        damage = random.randint(5, 10)
        print(f"{enemy.name} defends while attacking for {damage} damage.")
        player.take_damage(damage)

class HitAndRunAttack(Strategy):
    def execute_attack(self, enemy, player: Player) -> None:
        damage = random.randint(10, 20)
        # Retreat after attack
        retreat_steps = 2
        enemy.position -= retreat_steps
        print(f"{enemy.name} hits and retreats {retreat_steps} steps, dealing {damage} damage!")
        player.take_damage(damage)

class BerserkAttack(Strategy):
    def execute_attack(self, enemy, player: Player) -> None:
        damage = random.randint(25, 50)
        print(f"{enemy.name} goes BERSERK and deals {damage} damage!")
        player.take_damage(damage)

# ----------------------
# Orchestrator
# ----------------------
class Orchestrator:
    AGGRESSIVE_RANGE = 10
    DEFENSIVE_RANGE = 5
    BERSERK_THRESHOLD = 0.2  # 20% of max health
    LOW_HEALTH_THRESHOLD = 0.3  # 30% of max health

    def calc_proximity(self, player_position: int, enemy_position: int) -> int:
        return abs(player_position - enemy_position)

    def choose_strategy(self, enemy, player: Player) -> Strategy:
        # Berserk mode overrides everything
        if enemy.health / enemy.max_health < self.BERSERK_THRESHOLD:
            return BerserkAttack()

        proximity = self.calc_proximity(player.position, enemy.position)

        # Low health: play defensively
        if enemy.health / enemy.max_health < self.LOW_HEALTH_THRESHOLD:
            return DefensiveAttack()
        elif proximity <= self.AGGRESSIVE_RANGE:
            return AggressiveAttack()
        else:
            return HitAndRunAttack()

# ----------------------
# Enemy Class
# ----------------------
class Enemy:
    def __init__(self, name: str, health: int, position: int):
        self.name = name
        self.health = health
        self.max_health = health
        self.position = position
        self.strategy: Strategy = DefensiveAttack()

    def attack(self, player: Player, orchestrator: Orchestrator) -> None:
        # Choose and execute strategy based on current health and proximity
        self.strategy = orchestrator.choose_strategy(self, player)
        self.strategy.execute_attack(self, player)

    def take_damage(self, damage: int) -> None:
        self.health = max(0, self.health - damage)
        print(f"{self.name} takes {damage} damage! Health now {self.health}.")

# ----------------------
# Example Combat Simulation
# ----------------------
if __name__ == "__main__":
    player = Player(health=100, position=0)
    enemy = Enemy(name="Orc Warrior", health=100, position=5)
    orchestrator = Orchestrator()

    # Simulate combat rounds
    rounds = 10
    for i in range(rounds):
        print(f"\n--- Round {i+1} ---")
        enemy.attack(player, orchestrator)

        # Player attacks back
        if player.health > 0:
            damage = random.randint(8, 20)
            print(f"Player strikes back for {damage} damage!")
            enemy.take_damage(damage)

        if player.health <= 0:
            print("Player has been defeated!")
            break
        if enemy.health <= 0:
            print(f"{enemy.name} has been defeated!")
            break
