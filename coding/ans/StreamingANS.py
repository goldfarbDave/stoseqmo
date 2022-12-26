class StreamingANS:
    def __init__(self, precision, partial=None):
        self.precision = precision
        self.mask = (1 << precision) - 1
        self.bulk = []
        self.head = 1 # We could technically initialize this with zero too.
        if not partial:
            return
        if len(partial) == 1:
            self.head = partial[0]
        else:
            self.head = partial[-2] | (partial[-1] << self.precision)
            self.bulk = partial[:-2]
    # Precision-based scaling (as opposed to range-based in AC)
    def _scale_prob(self, prob):
        return int(prob * (2**self.precision))
    def _inv_scale_prob(self, scaled_prob):
        return scaled_prob / (2**self.precision)

    def encode(self, symbol, model):
        pmf = self._scale_prob(model.pmf(symbol))
        excmf = self._scale_prob(model.exclusive_cmf(symbol))
        if (self.head >> self.precision) >= pmf:
            self.bulk.append(self.head & self.mask)
            self.head >>= self.precision
        z = self.head % pmf
        self.head //= pmf
        z += excmf
        self.head = (self.head << self.precision) | z
    def decode(self, model):
        z = self.head & self.mask
        self.head >>= self.precision
        sym = model.find_sym_from_cum_prob(self._inv_scale_prob(z))
        pmf = self._scale_prob(model.pmf(sym))
        excmf = self._scale_prob(model.exclusive_cmf(sym))
        z -= excmf
        self.head = self.head * pmf + z
        # If we did corrective shift in the encoder, load from bulk now
        if (self.head & (~self.mask)) == 0 and len(self.bulk):
            self.head <<= self.precision
            self.head |= self.bulk.pop(-1)
        return sym

    def get_compressed(self):
        head_list = [self.head & self.mask]
        if (self.head >> self.precision):
            head_list.append(self.head >> self.precision)
        return self.bulk + head_list
