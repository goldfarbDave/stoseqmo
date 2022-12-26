from itertools import accumulate
import sys
sys.path.append("../")
from structures.FTree import FTree

class LinearOrder0Model:
    def __init__(self, syms, init_count=1):
        self.count_dict = {s:init_count for s in syms}
        self.total = init_count*len(syms)
        self.idx_to_sym = {i:s for i, s in enumerate(syms)}
        self.sym_to_idx = {s:i for i, s in enumerate(syms)}
    def learn(self, sym):
        self.count_dict[sym] += 1
        self.total +=1
    def unlearn(self, sym):
        self.count_dict[sym] -= 1
        self.total -= 1
    def _point_count(self, sym):
        return self.count_dict[sym]
    def _partial_counts(self):
        return list(accumulate(self.count_dict.values(), initial=0))
    def pmf(self, sym):
        return self._point_count(sym)/self.total
    def exclusive_cmf(self, sym):
        # P(X < x) , rather than the typical P(X <= x)
        # This distinction is necessary because scale(x + y) != scale(x) + scale(y).
        # (Thus, scaled_cmf - scaled_pmf != scale(cum_count - point_count)).
        idx = self.sym_to_idx[sym]
        count = self._partial_counts()[idx]
        return count / self.total
    def find_sym_from_cum_prob(self, cum_prob):
        cum_count = cum_prob*self.total
        cum_counts = self._partial_counts()
        ranges = [range(a, b) for a, b in zip(cum_counts, cum_counts[1:])]
        idx = [i for i, r in enumerate(ranges) if cum_count >= r.start and cum_count < r.stop][0]
        sym = self.idx_to_sym[idx]
        return sym

class UniformModel(LinearOrder0Model):
    def learn(self, sym):
        pass
    def unlearn(self, sym):
        pass

class Order0KT(LinearOrder0Model):
    def __init__(self, syms):
        super().__init__(syms, 0)
    def pmf(self, sym):
        c = self._point_count(sym)
        num_syms = len(self.sym_to_idx)
        return (c + 1/num_syms)/(self.total + 1)
    def exclusive_cmf(self, sym):
        idx = self.sym_to_idx[sym]
        count = self._partial_counts()[idx]
        num_syms = len(self.sym_to_idx)
        return (count + idx/num_syms)/(self.total + 1)
    def find_sym_from_cum_prob(self, cum_prob):
        assert False
        # Re-use parent's logic, we just need to twiddle cum_prob:
        # numerator = cum_prob * (self.total + 1)
        # cum_count = int(numerator)
        # return self.idx_to_sym[int((numerator-cum_count)*len(self.sym_to_idx))]
        # if numerator < 1:
        #     return self.idx_to_sym[int(numerator*len(self.sym_to_idx))]
        # cum_count = int(numerator)
        # cum_counts = self._partial_counts()
        # try:
        #     sidx = cum_counts.index(cum_count)
        #     eidx = len(cum_counts)-1-cum_counts[::-1].index(cum_count)
        #     if eidx - sidx > 1:
        #         return self.idx_to_sym[int((numerator-cum_count)*len(self.sym_to_idx))]
        #     if eidx == sidx:
        #         return self.idx_to_sym[eidx]
        # except:
        #     pass
        # ranges = [range(a, b) for a, b in zip(cum_counts, cum_counts[1:])]
        # idx = [i for i, r in enumerate(ranges) if cum_count >= r.start and cum_count < r.stop][0]
        # sym = self.idx_to_sym[idx]
        # return sym

class LogOrder0Model:
    def __init__(self, syms, init_count=1):
        self.tree = FTree(len(syms))
        self.total = init_count*len(syms)
        self.idx_to_sym = {i:s for i, s in enumerate(syms)}
        self.sym_to_idx = {s:i for i, s in enumerate(syms)}
    def learn(self, sym):
        self.tree.update(self.sym_to_idx[sym], 1)
        self.total += 1
    def unlearn(self, sym):
        self.tree.update(self.sym_to_idx[sym], -1)
        self.total -= 1
    def _point_count(self, sym):
        pos = self.sym_to_idx[sym]
        return self.tree.range_sum(pos, pos+1)
    def _ex_cum_count(self, sym):
        return self.tree.range_sum(self.sym_to_idx[sym])
    def pmf(self, sym):
        return self._point_count(sym) / self.total
    def exclusive_cmf(self, sym):
        return self._ex_cum_count(sym) / self.total
    def find_sym_from_cum_prob(self, cum_prob):
        cum_count = cum_prob*self.total
        idx = self.tree.val_to_idx(cum_count)
        sym = self.idx_to_sym[idx]
        return sym
