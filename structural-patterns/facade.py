from abc import ABC, abstractmethod

# ===== Subsystems =====

class ElectricDevice(ABC):

    @abstractmethod
    def on(self) -> None:
        pass

    @abstractmethod
    def off(self) -> None:
        pass

class Lights(ElectricDevice):
    def on(self) -> None:
        print("Lights ON")

    def off(self) -> None:
        print("Lights OFF")

class AC(ElectricDevice):
    def on(self) -> None:
        print("AC ON")

    def off(self) -> None:
        print("AC OFF")

    def set_temperature(self, temp: int) -> None:
        print(f"AC temperature set to {temp}°C")

class Music(ElectricDevice):
    def on(self) -> None:
        print("Music system ON")

    def off(self) -> None:
        print("Music system OFF")

    def play_song(self, song: str) -> None:
        print(f"Playing: {song}")

# ===== Facade =====

class SmartHomeFacade:
    """
    Facade that hides all smart home subsystems and exposes
    simple, high-level operations to the client.
    """

    def __init__(self):
        self._lights = Lights()
        self._ac = AC()
        self._music = Music()

    def start_party(self) -> None:
        print("\n--- Party Mode ---")
        self._lights.on()
        self._ac.on()
        self._ac.set_temperature(21)
        self._music.on()
        self._music.play_song("Party Anthem")

    def end_party(self) -> None:
        print("\n--- Party Over ---")
        self._music.off()
        self._ac.off()
        self._lights.off()

    def vacation_mode(self) -> None:
        print("\n--- Vacation Mode ---")
        print("Simulating presence while away...")
        self._lights.on()
        self._ac.on()
        self._ac.set_temperature(26)
        self._lights.off()
        self._ac.off()

# ===== Client Code =====

home = SmartHomeFacade()
home.start_party()
home.end_party()
home.vacation_mode()
