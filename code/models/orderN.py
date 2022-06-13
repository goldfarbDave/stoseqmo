from collections import deque
from itertools import chain
def bitlist_to_idx(bitlist):
    out = 0
    for bit in bitlist:
        out = (out << 1) | bit
    return out
class OrderNLaplace:
    def __init__(self, syms, N=6):
        assert syms == [0, 1]
        self.counts = [1]*(2**N)
        self.past_bits = deque([0]*(N-1), maxlen=N-1)
    def learn(self, bit):
        idx = bitlist_to_idx(chain(self.past_bits, [bit]))
        self.counts[idx] += 1
        self.past_bits.append(bit)
    def _probs(self):
        # Count number of 1s predicted. If we end in 1, it's an odd number
        evens = sum(self.counts[i] for i in range(0,len(self.counts), 2))
        odds = sum(self.counts[i] for i in range(1,len(self.counts), 2))
        tot = evens+odds
        return [evens/tot, odds/tot]
    def pmf(self, sym):
        return self.probs()[sym]
    def exclusive_cmf(self, sym):
        return self.probs()[0] if sym else 0.0
    def find_sym_from_cum_prob(self, cum_prob):
        probs = self.probs()
        return 1 if cum_prob > probs[0] else 0
