#==========================
# Product
#==========================
class Pizza:
    def __init__(self):
        self.size = None
        self.crust = None
        self.toppings = []

    def display(self):
        size = self.size or "Default size"
        crust = self.crust or "Default crust"
        toppings = ", ".join(self.toppings) if self.toppings else "No toppings"

        return (
            f"Pizza details:\n"
            f"- Size: {size}\n"
            f"- Crust: {crust}\n"
            f"- Toppings: {toppings}"
        )

#==========================
# Builder
#==========================
class PizzaBuilder:
    VALID_SIZES = {"Small", "Medium", "Large"}
    VALID_CRUSTS = {"Thin", "Thick", "Stuffed"}

    def __init__(self):
        self.reset()

    def reset(self):
        self._pizza = Pizza()
        
        # Returning self allows method chaining like:
        # builder.set_size(...).set_crust(...).add_topping(...)
        return self

    def set_size(self, size):
        if size not in self.VALID_SIZES:
            raise ValueError(f"Invalid size: {size}")

        self._pizza.size = size
        return self

    def set_crust(self, crust):
        if crust not in self.VALID_CRUSTS:
            raise ValueError(f"Invalid crust: {crust}")

        self._pizza.crust = crust
        return self

    def add_topping(self, topping):
        if not topping or not isinstance(topping, str):
            raise ValueError("Topping must be a non-empty string")

        self._pizza.toppings.append(topping)
        return self

    def build(self):
        pizza = self._pizza
        self.reset()
        return pizza

#==========================
# Director
#==========================
class PizzaDirector:
    def __init__(self, builder):
        self.builder = builder

    def margherita(self):
        return (
            self.builder
            .set_size("Medium")
            .set_crust("Thin")
            .add_topping("Mozzarella")
            .add_topping("Basil")
            .build()
        )

    def pepperoni(self):
        return (
            self.builder
            .set_size("Large")
            .set_crust("Thick")
            .add_topping("Mozzarella")
            .add_topping("Pepperoni")
            .build()
        )

#==========================
# Usage
#==========================
builder = PizzaBuilder()
director = PizzaDirector(builder)

pizza = director.margherita()
print(pizza.display())
