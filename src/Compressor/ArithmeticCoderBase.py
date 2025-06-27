class ArithmeticCoderBase:
    def __init__(self, num_bits):
        if num_bits < 1:
            raise ValueError("State size too small")
        self.num_state_bits = num_bits
        self.full_range = 1 << self.num_state_bits
        self.half_range = self.full_range >> 1
        self.quarter_range = self.half_range >> 1
        self.minimum_range = self.quarter_range + 2
        self.maximum_total = self.minimum_range
        self.state_mask = self.full_range - 1
        self.low = 0
        self.high = self.state_mask