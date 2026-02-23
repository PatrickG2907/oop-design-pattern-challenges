from abc import ABC, abstractmethod
from dataclasses import dataclass, replace
from typing import List

# --- PriceBreakdown ---
@dataclass(frozen=True)
class PriceBreakdown:
    discounted: float = 0.0
    tax: float = 0.0
    shipping: float = 0.0

    @property
    def total(self) -> float:
        return self.discounted + self.tax + self.shipping

# --- Abstract Order Item ---
@dataclass
class OrderItem(ABC):
    name: str
    base_price: float

    def __post_init__(self):
        if not self.name:
            raise ValueError("Name cannot be empty!")
        if self.base_price < 0:
            raise ValueError("Price cannot be negative!")

    @property
    def discount_rate(self) -> float:
        return 0.10

    @property
    def tax_rate(self) -> float:
        return 0.07

    @property
    def shipping_cost(self) -> float:
        return 0.0

    # Unified accept method for all visitors
    def accept(self, visitor: "Visitor", breakdown: PriceBreakdown) -> PriceBreakdown:
        return visitor.visit(self, breakdown)

# --- Concrete Items ---
@dataclass
class PhysicalProduct(OrderItem):
    @property
    def shipping_cost(self) -> float:
        return 5.0

@dataclass
class DigitalProduct(OrderItem):
    pass

@dataclass
class Service(OrderItem):
    @property
    def discount_rate(self) -> float:
        return 0.05

# --- Visitor Base ---
class Visitor(ABC):
    @abstractmethod
    def visit(self, item: OrderItem, breakdown: PriceBreakdown) -> PriceBreakdown:
        pass

# --- Concrete Visitors ---
class DiscountApplier(Visitor):
    def visit(self, item: OrderItem, breakdown: PriceBreakdown) -> PriceBreakdown:
        discounted = item.base_price * (1 - item.discount_rate)
        return replace(breakdown, discounted=discounted)

class TaxCalculator(Visitor):
    def visit(self, item: OrderItem, breakdown: PriceBreakdown) -> PriceBreakdown:
        tax = breakdown.discounted * item.tax_rate
        return replace(breakdown, tax=tax)

class ShippingCostCalculator(Visitor):
    def visit(self, item: OrderItem, breakdown: PriceBreakdown) -> PriceBreakdown:
        return replace(breakdown, shipping=item.shipping_cost)

# --- Client code ---
items: List[OrderItem] = [
    PhysicalProduct("1984", 20.0),
    DigitalProduct("Inception", 15.0),
    Service("Halo Coaching", 50.0)
]

visitors: List[Visitor] = [DiscountApplier(), TaxCalculator(), ShippingCostCalculator()]

final_breakdowns = {}
for item in items:
    breakdown = PriceBreakdown()
    for visitor in visitors:
        breakdown = item.accept(visitor, breakdown)
    final_breakdowns[item.name] = breakdown

# --- Final Summary ---
print("\nFinal order summary:")
for item in items:
    b = final_breakdowns[item.name]
    print(f"{item.name}: Base={item.base_price:.2f}, "
          f"Discounted={b.discounted:.2f}, "
          f"Tax={b.tax:.2f}, "
          f"Shipping={b.shipping:.2f}, "
          f"Total={b.total:.2f}")
