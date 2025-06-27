from .ArithmeticCoderBase import ArithmeticCoderBase

class ArithmeticDecoder(ArithmeticCoderBase):
    def __init__(self, num_bits, read_callback):
        super().__init__(num_bits)
        self.input = read_callback
        self.code = 0
        for _ in range(self.num_state_bits):
            self.code = (self.code << 1) | self._read_code_bit()

    def read(self, freqs):
        total = freqs.get_total()
        range_ = self.high - self.low + 1
        offset = self.code - self.low
        value = ((offset + 1) * total - 1) // range_
        start = 0
        end = freqs.get_symbol_limit()
        while end - start > 1:
            middle = (start + end) // 2
            if freqs.get_low(middle) > value:
                end = middle
            else:
                start = middle
        symbol = start

        self._update(freqs, symbol)
        return symbol

    def _update(self, freqs, symbol):
        low = freqs.get_low(symbol)
        high = freqs.get_high(symbol)
        total = freqs.get_total()

        range_ = self.high - self.low + 1
        self.high = self.low + (range_ * high // total) - 1
        self.low = self.low + (range_ * low // total)

        while True:
            if self.high < self.half_range:
                pass
            elif self.low >= self.half_range:
                self.low -= self.half_range
                self.high -= self.half_range
                self.code -= self.half_range
            elif self.low >= self.quarter_range and self.high < 3 * self.quarter_range:
                self.low -= self.quarter_range
                self.high -= self.quarter_range
                self.code -= self.quarter_range
            else:
                break
            self.low = (self.low << 1) & self.state_mask
            self.high = ((self.high << 1) & self.state_mask) | 1
            self.code = ((self.code << 1) & self.state_mask) | self._read_code_bit()

    def _read_code_bit(self):
        try:
            return next(self.input)
        except StopIteration:
            return 0