class SimpleFrequencyTable:
    def __init__(self, freqs):
        self.freqs = list(freqs)
        self.cumulative = None
        self.total = None
        self._build_cumulative()

    def _build_cumulative(self):
        self.cumulative = [0]
        total = 0
        for f in self.freqs:
            total += f
            self.cumulative.append(total)
        self.total = total

    def get_symbol_limit(self):
        return len(self.freqs)

    def get(self, symbol):
        return self.freqs[symbol]

    def get_total(self):
        return self.total

    def get_low(self, symbol):
        return self.cumulative[symbol]

    def get_high(self, symbol):
        return self.cumulative[symbol + 1]