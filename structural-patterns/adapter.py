import random

# Existing e-commerce platform (UNCHANGED)
# It expects any payment processor to implement: pay(amount) -> bool
class EcommercePlatform:
    def __init__(self, payment_processor):
        self.payment_processor = payment_processor

    def checkout(self, amount):
        # Platform relies on a boolean result
        if self.payment_processor.pay(amount):
            print("Payment successful!")
        else:
            print("Payment failed!")

# New payment gateway with an incompatible interface
class NewPaymentGateway:
    def make_payment(self, total):
        # Simulate inconsistent success responses from the gateway
        # Sometimes returns "OK", sometimes True, sometimes False
        return random.choice(["OK", True, False])

# Adapter that makes the new gateway compatible with the platform
class PaymentGatewayAdapter:
    def __init__(self, gateway):
        self.gateway = gateway

    def pay(self, amount):
        # Translate the platform's expected method call
        response = self.gateway.make_payment(amount)

        # Normalize the gateway response so the platform always gets a boolean
        if response == "OK":
            return True
        return bool(response)

# ----- Test the adapter -----
if __name__ == "__main__":
    gateway = NewPaymentGateway()                 # Incompatible gateway
    adapter = PaymentGatewayAdapter(gateway)     # Adapter wraps the gateway
    platform = EcommercePlatform(adapter)        # Platform uses adapter

    # Process multiple payments to show normalization works
    for i in range(5):
        print(f"Attempt {i + 1}: ", end="")
        platform.checkout(100)
