from collections import Counter, defaultdict
from itertools import accumulate, chain
def flipbit(bit):
    assert bit in range(2)
    return 0 if bit else 1
class BitStream:
    # Makes subsequent pops return 0
    def __init__(self, bitstring):
        self.rev_bitstring = list(reversed(bitstring))
    def pop(self):
        try:
            return self.rev_bitstring.pop()
        except IndexError:
            return 0

class Model:
    def __init__(self, sym_range):
        self.sym_to_counts = {s:1 for s in sym_range}
        self.sym_to_idx = {s:idx for idx, s in enumerate(sym_range)}
        self.idx_to_sym = {idx:s for idx, s in enumerate(sym_range)}
    def learn(self, sym):
        self.sym_to_counts[sym] += 1
    def total(self):
        return sum(self.sym_to_counts.values())
    def stats(self, sym):
        return (self.sym_to_counts[sym], self.total())
    def region_starts(self):
        return list(accumulate(chain([0], self.sym_to_counts.values())))
    def ranges(self):
        starts = self.region_starts()
        return list(zip(starts, starts[1:]))
    def range_of_sym(self, sym):
        return self.ranges()[self.sym_to_idx[sym]]
    def containing_range_and_sym(self, num):
        rs = self.ranges()
        for idx, r in enumerate(rs):
            if num in range(*r):
                break
        # idx_r = [(idx,r) for idx, r in rs if num in range(*r)]
        # assert len(idx_r) == 1
        # idx, r = idx_r[0]
        sym = self.idx_to_sym[idx]
        return r, sym
class AC:
    whole_b = 64 - 2
    q1_b = 1 << (whole_b - 2)
    q2_b = 1 << (whole_b - 1)
    q4_b = (1 << whole_b) - 1
    mask = q4_b
    def __init__(self, model, bit_stream=None):
        self.model=model
        self.L = 0
        self.R = self.q4_b
        if bit_stream:
            self.in_bits = bit_stream
            self.D = 0
            # Decoder mode
            for _ in range(self.whole_b):
                self.D <<= 1
                self.D += self.in_bits.pop()
        else:
            self.waiting = 0
            self.outbits=[]
    def _narrow_region(self, lo, hi, total):
        r = self.R // total
        self.L += r*lo
        if hi == total:
            self.R -= r*lo
        elif hi < total:
            self.R = r * (hi-lo)
        else:
            print("PANIC")
    def _out_all(self, bit):
        res = [bit] + [flipbit(bit)]*self.waiting
        #print(f"Outputting {bit} waiting {self.waiting} total {1+self.waiting}")
        self.waiting = 0
        self.outbits.extend(res)

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

    def encode(self, sym):
        #print("Encoding " + sym)
        lo, hi = self.model.range_of_sym(sym)
        total = self.model.total()
        self._narrow_region(lo, hi, total)
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
        total = self.model.total()
        r = self.R // total
        dr = (self.D-self.L)//r
        return min(total-1, dr)

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

        
    def decode(self):
        target = self._get_target()
        total = self.model.total()
        (lo, hi), sym = self.model.containing_range_and_sym(target)
        self._narrow_region(lo, hi, total)
        self._discard_bits()
        return sym




def pad_to_byte(bits):
    num_zeros = 8 - (len(bits) - 8)
    return bits + [0]*num_zeros
def do_enc(src):
    model = Model(["A", "B", "C"])
    ac = AC(model)
    for c in src:
        ac.encode(c)
        model.learn(c)
    ac.finish_encode()
    return ac.outbits
def do_dec(bit_string):
    model = Model(["A", "B", "C"])
    decoded_str = ""
    x = ""
    ac = AC(model, bit_stream=BitStream(bit_string))
    while x != "C":
        x = ac.decode()
        model.learn(x)
        decoded_str += x
    return decoded_str
def do_flow(src):
    model = Model(["A", "B", "C"])
    ac = AC(model)
    for c in src:
        ac.encode(c)
        model.learn(c)
    ac.finish_encode()
    bit_string = pad_to_byte(ac.outbits)

    model = Model(["A", "B", "C"])
    decoded_str = ""
    x = ""
    ac = AC(model, bit_stream=BitStream(bit_string))
    while x != "C":
        x = ac.decode()
        model.learn(x)
        decoded_str += x
    assert decoded_str == src
    # print(decoded_str)

model = Model(["A", "B", "C"])
in_str = "BBBBABBBC"
precision = 64
whole = 2**precision
mask = whole-1
half = whole >> 1
quarter = whole >> 2
narrowing_start = 0
narrowing_end = whole
waiting = 0
outbits = []
def emit(bit):
    global waiting
    global outbits
    outbits.extend([bit] + [flipbit(bit)] * waiting)
    waiting = 0
for char in in_str:
    # Update start and end
    point_start, point_end = model.range_of_sym(char)
    total = model.total()
    model.learn(char)
    width = narrowing_end-narrowing_start
    narrowing_end = narrowing_start + width*point_end//total
    narrowing_start += width*point_start//total
    while narrowing_start > half or narrowing_end < half:
        if narrowing_end <= half:
            emit(0)
        elif narrowing_start > half:
            emit(1)
            narrowing_start -= half
            narrowing_end -= half
        narrowing_start <<= 1
        narrowing_end <<= 1
    while narrowing_start > quarter and narrowing_end < 3*quarter:
        waiting+=1
        narrowing_start = (narrowing_start-quarter)<<1
        narrowing_end = (narrowing_end-quarter)<<1
waiting += 1
if narrowing_start <= quarter:
    emit(0)
else:
    emit(1)

# print(outbits)
# print(do_enc(in_str))
inbits = BitStream(outbits)
model = Model(["A", "B", "C"])
narrowing_start = 0
narrowing_end = whole
out_str = ""
# init D
D = 0
for _ in range(precision):
    D <<= 1
    D += inbits.pop()
do_quit=False
i = 0
sym = None
while sym != "C":
    width = narrowing_end - narrowing_start
    total = model.total()
    for idx, (start,end) in enumerate(model.ranges()):
        candidate_start = narrowing_start + width*start//total
        candidate_end = narrowing_start + width*end//total
        if D in range(candidate_start, candidate_end):
            break
    sym = model.idx_to_sym[idx]
    model.learn(sym)
    out_str += sym
    narrowing_start = candidate_start
    narrowing_end = candidate_end
    # Rescale
    while narrowing_start > half or narrowing_end <= half:
        if narrowing_end <= half:
            pass
        elif narrowing_start > half:
            narrowing_start -= half
            narrowing_end -= half
            D -= half
        narrowing_start <<= 1
        narrowing_end <<= 1
        D <<= 1
        D &= mask
        D += inbits.pop()
    while narrowing_start > quarter and narrowing_end < 3*quarter:
        narrowing_start = (narrowing_start-quarter)<<1
        narrowing_end = (narrowing_end-quarter)<<1
        D = (D-quarter)<<1
        D &= mask
        D += inbits.pop()

# import numpy as np
# from numpy.random import default_rng
# rng = default_rng(1998)
# lens = rng.geometric(p=1/1000, size=5)
# for l in lens:
#     flips = rng.binomial(1, .1, l)
#     st = "".join(["A" if flip else "B" for flip in flips]) + "C"
#     do_flow(st)
