from abc import ABC, abstractmethod

# ---------------- Observer ----------------
class Observer(ABC):
    @abstractmethod
    def update(self, temperature: int = None, humidity: int = None): pass

# ---------------- Subject ----------------
class Subject(ABC):
    @abstractmethod
    def attach(self, observer: Observer): pass

    @abstractmethod
    def detach(self, observer: Observer): pass

# ---------------- Concrete Subject ----------------
class WeatherStation(Subject):
    def __init__(self, temperature: int, humidity: int):
        self._observers = []
        self._temperature = temperature
        self._prev_temperature = temperature
        self._humidity = humidity

    def attach(self, observer: Observer):
        if observer not in self._observers:
            self._observers.append(observer)

    def detach(self, observer: Observer):
        if observer in self._observers:
            self._observers.remove(observer)

    def update_weather(self, temperature: int = None, humidity: int = None):
        """Update weather and notify observers based on their preferences."""
        temp_change = None
        if temperature is not None:
            temp_change = abs(temperature - self._prev_temperature)
            self._prev_temperature = self._temperature
            self._temperature = temperature

        if humidity is not None:
            self._humidity = humidity

        for obs in self._observers:
            notify_temp = False
            notify_humidity = False

            # Check if observer cares about temperature and threshold is met
            if hasattr(obs, "min_temp_change") and temp_change is not None:
                if temp_change >= obs.min_temp_change:
                    notify_temp = True

            # Check if observer cares about humidity
            if hasattr(obs, "watch_humidity") and obs.watch_humidity and humidity is not None:
                notify_humidity = True

            # Only notify if either condition is met
            if notify_temp or notify_humidity:
                obs.update(
                    temperature=self._temperature if notify_temp else None,
                    humidity=self._humidity if notify_humidity else None
                )

# ---------------- Concrete Observers ----------------
class PhoneDisplay(Observer):
    def __init__(self, name: str, min_temp_change: int = 0, watch_humidity: bool = True):
        self.name = name
        self.min_temp_change = min_temp_change
        self.watch_humidity = watch_humidity

    def update(self, temperature: int = None, humidity: int = None):
        if temperature is not None:
            print(f"Phone display {self.name} -> Temperature: {temperature}")
        if humidity is not None:
            print(f"Phone display {self.name} -> Humidity: {humidity}")

class WindowDisplay(Observer):
    def __init__(self, name: str, min_temp_change: int = 0, watch_humidity: bool = True):
        self.name = name
        self.min_temp_change = min_temp_change
        self.watch_humidity = watch_humidity

    def update(self, temperature: int = None, humidity: int = None):
        if temperature is not None:
            print(f"Window display {self.name} -> Temperature: {temperature}")
        if humidity is not None:
            print(f"Window display {self.name} -> Humidity: {humidity}")

# ---------------- Usage ----------------
station = WeatherStation(10, 50)
station.attach(PhoneDisplay("A", min_temp_change=2, watch_humidity=False))  # Only temp > 2°C
station.attach(WindowDisplay("B", min_temp_change=0, watch_humidity=True))  # Always watches humidity

station.update_weather(11)           # Only WindowDisplay updates (PhoneDisplay threshold not met)
station.update_weather(15)           # Both displays update (temp > 2°C)
station.update_weather(humidity=55)  # Only WindowDisplay updates (PhoneDisplay doesn't watch humidity)
station.update_weather(16, 60)       # PhoneDisplay updates (temp change 1°C no, threshold not met), WindowDisplay updates both
