from itertools import accumulate

# General utilities
def flipbit(bit):
    assert bit in range(2)
    return 0 if bit else 1

class BitStream:
    # Makes subsequent pops return 0
    def __init__(self, bitstring):
        self.data = bitstring
        self.idx = 0
    def left(self):
        return len(self.data) - self.idx - 1 if self.idx < len(self.data) else 0
    def pop(self):
        if self.idx >= len(self.data):
            return 0
        c = self.data[self.idx]
        self.idx += 1
        return c


class StreamingAC:
    def __init__(self, prec, compressed=None):
        self.whole_b = prec - 2
        self.q1_b = 1 << (self.whole_b - 2)
        self.q2_b = 1 << (self.whole_b - 1)
        self.q4_b = (1 << self.whole_b) - 1
        self.mask = self.q4_b
        self.L = 0
        self.R = self.q4_b
        if compressed:
            self.in_bits = BitStream(compressed)
            self.D = 0
            # Decoder mode
            for _ in range(self.whole_b):
                self.D <<= 1
                self.D += self.in_bits.pop()
        else:
            self.waiting = 0
            self._outbits=[]
    def get_compressed(self):
        return self._outbits
    def _narrow_region(self, scaled_excmf, scaled_pmf):
        self.L += scaled_excmf
        self.R = scaled_pmf
    def _out_all(self, bit):
        res = [bit] + [flipbit(bit)]*self.waiting
        self.waiting = 0
        self._outbits.extend(res)
    def _output_bits(self):
        while self.R <= self.q1_b:
            if self.R+self.L <= self.q2_b:
                self._out_all(0)
            elif self.L >= self.q2_b:
                self._out_all(1)
                self.L -= self.q2_b
            else:
                self.waiting += 1
                self.L -= self.q1_b
            self.L <<= 1
            self.R <<= 1
    # Helpers for range-based (rather than fixed precision as in ANS) scaling
    def _scale_prob(self, prob):
        return int(prob * self.R)
    def _inv_scale_prob(self, prob):
        return prob / self.R
    def encode(self, sym, model):
        pmf = self._scale_prob(model.pmf(sym))
        assert pmf > 0
        excmf = self._scale_prob(model.exclusive_cmf(sym))
        self._narrow_region(excmf, pmf)
        self._output_bits()
    def finish_encode(self):
        while True:
            if self.L + (self.R >> 1) >= self.q2_b:
                self._out_all(1)
                if self. L < self.q2_b:
                    self.R -= self.q2_b - self.L
                    self.L = 0
                else:
                    self.L -= self.q2_b
            else:
                self._out_all(0)
                if self.L+self.R > self.q2_b:
                    self.R = self.q2_b - self.L
            if (self.R == self.q2_b):
                break
            self.L <<= 1
            self.R <<= 1
    def _get_target(self):
        return self.D-self.L

    def _discard_bits(self):
        while self.R <= self.q1_b:
            if self.L >= self.q2_b:
                self.L -= self.q2_b
                self.D -= self.q2_b
            elif self.L+self.R <= self.q2_b:
                pass # lower half, do nothing
            else:
                self.L -= self.q1_b
                self.D -= self.q1_b
            self.L <<= 1
            self.R <<= 1
            self.D <<= 1
            self.D &= self.mask
            self.D += self.in_bits.pop()

    def decode(self, model):
        target = self._get_target()
        sym = model.find_sym_from_cum_prob(self._inv_scale_prob(target))
        pmf = self._scale_prob(model.pmf(sym))
        excmf = self._scale_prob(model.exclusive_cmf(sym))
        self._narrow_region(excmf, pmf)
        self._discard_bits()
        return sym
import sys
sys.path.append("../..")
from models.order0 import LinearOrder0Model

def do_flow(src):
    Model = LinearOrder0Model
    model = Model([0,1])
    ac = StreamingAC(64)
    for c in src:
        ac.encode(c, model)
        model.learn(c)
    ac.finish_encode()
    model = Model([0,1])
    decoded_str = []
    x = ""
    ac = StreamingAC(64, compressed=ac.get_compressed())
    while len(decoded_str) != len(src):
        x = ac.decode(model)
        model.learn(x)
        decoded_str += [x]
    assert decoded_str == src
do_flow([1,1,1,1,0,0,0,1,1,1,1,1,0])
