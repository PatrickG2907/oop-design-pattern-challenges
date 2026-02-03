from abc import ABC, abstractmethod

#======================================================================================================
# Static Factory
#======================================================================================================

# === Abstract Products ===
class Button(ABC):
    @abstractmethod
    def render(self):
        pass

class Checkbox(ABC):
    @abstractmethod
    def render(self):
        pass

class Slider(ABC):
    @abstractmethod
    def render(self):
        pass

# === Concrete Products ===
class LightButton(Button):
    def render(self):
        return "Light Button rendered"

class DarkButton(Button):
    def render(self):
        return "Dark Button rendered"

class LightCheckbox(Checkbox):
    def render(self):
        return "Light Checkbox rendered"

class DarkCheckbox(Checkbox):
    def render(self):
        return "Dark Checkbox rendered"

class LightSlider(Slider):
    def render(self):
        return "Light Slider rendered"

class DarkSlider(Slider):
    def render(self):
        return "Dark Slider rendered"

# === Abstract Factory ===
class UIFactory(ABC):
    @abstractmethod
    def create_button(self) -> Button:
        pass

    @abstractmethod
    def create_checkbox(self) -> Checkbox:
        pass

    @abstractmethod
    def create_slider(self) -> Slider:
        pass

# === Concrete Factories ===
class LightFactory(UIFactory):
    def create_button(self) -> Button:
        return LightButton()
    
    def create_checkbox(self) -> Checkbox:
        return LightCheckbox()
    
    def create_slider(self) -> Slider:
        return LightSlider()

class DarkFactory(UIFactory):
    def create_button(self) -> Button:
        return DarkButton()
    
    def create_checkbox(self) -> Checkbox:
        return DarkCheckbox()
    
    def create_slider(self) -> Slider:
        return DarkSlider()

# === Client ===
def build_ui(factory: UIFactory):
    components = [
        factory.create_button(),
        factory.create_checkbox(),
        factory.create_slider()
    ]
    for component in components:
        print(component.render())

# === Usage ===
if __name__ == "__main__":
    
    print("========= Static Factory =========")
    
    print("\nLight Mode UI:")
    build_ui(LightFactory())

    print("\nDark Mode UI:")
    build_ui(DarkFactory())

#======================================================================================================
# Dynamic Factory
#======================================================================================================

from abc import ABC, abstractmethod

# === Abstract Product ===
class UIComponent(ABC):
    @abstractmethod
    def render(self):
        pass

# === Concrete Products ===
class LightButton(UIComponent):
    def render(self): return "Light Button rendered"

class DarkButton(UIComponent):
    def render(self): return "Dark Button rendered"

class LightCheckbox(UIComponent):
    def render(self): return "Light Checkbox rendered"

class DarkCheckbox(UIComponent):
    def render(self): return "Dark Checkbox rendered"

class LightSlider(UIComponent):
    def render(self): return "Light Slider rendered"

class DarkSlider(UIComponent):
    def render(self): return "Dark Slider rendered"

# === Abstract Factory ===
class UIFactory:
    def __init__(self):
        self._creators = {}  # registry for component creators

    def register_component(self, name: str, creator):
        self._creators[name] = creator

    def create(self, name: str) -> UIComponent:
        if name not in self._creators:
            raise ValueError(f"No component registered for '{name}'")
        return self._creators[name]()

# === Concrete Factories as Classes ===
class LightFactory(UIFactory):
    def __init__(self):
        super().__init__()
        self.register_component("button", LightButton)
        self.register_component("checkbox", LightCheckbox)
        self.register_component("slider", LightSlider)

class DarkFactory(UIFactory):
    def __init__(self):
        super().__init__()
        self.register_component("button", DarkButton)
        self.register_component("checkbox", DarkCheckbox)
        self.register_component("slider", DarkSlider)

# === Client ===
def build_ui(factory: UIFactory, components):
    for name in components:
        component = factory.create(name)
        print(component.render())

# === Usage ===
if __name__ == "__main__":
    
    print("\n========= Dynamic Factory =========")
    
    component_list = ["button", "checkbox", "slider"]

    print("\nLight Mode UI:")
    build_ui(LightFactory(), component_list)

    print("\nDark Mode UI:")
    build_ui(DarkFactory(), component_list)
