from .ArithmeticCoderBase import ArithmeticCoderBase

class ArithmeticEncoder(ArithmeticCoderBase):
    def __init__(self, num_bits, write_callback):
        super().__init__(num_bits)
        self.output = write_callback
        self.pending_bits = 0

    def write(self, freqs, symbol):
        low = freqs.get_low(symbol)
        high = freqs.get_high(symbol)
        total = freqs.get_total()

        range_ = self.high - self.low + 1
        self.high = self.low + (range_ * high // total) - 1
        self.low = self.low + (range_ * low // total)

        while True:
            if self.high < self.half_range:
                self._write_bit(0)
            elif self.low >= self.half_range:
                self._write_bit(1)
                self.low -= self.half_range
                self.high -= self.half_range
            elif self.low >= self.quarter_range and self.high < 3 * self.quarter_range:
                self.pending_bits += 1
                self.low -= self.quarter_range
                self.high -= self.quarter_range
            else:
                break
            self.low = (self.low << 1) & self.state_mask
            self.high = ((self.high << 1) & self.state_mask) | 1

    def _write_bit(self, bit):
        self.output(bit)
        for _ in range(self.pending_bits):
            self.output(1 - bit)
        self.pending_bits = 0

    def finish(self):
        self.pending_bits += 1
        if self.low < self.quarter_range:
            self._write_bit(0)
        else:
            self._write_bit(1)