# Adapted from https://github.com/omerktz/VMMPredictor/blob/master/vmms/vmm/vmm/algs/ctw/VolfNode.java
import numpy as np
RESCALE_THRESHOLD_COUNT = 255
DEFAULT_ALPHA=16
class VolfNode:
    def __init__(self, syms, alpha_factor):
        self.syms = syms
        self.counts = np.zeros(len(syms),dtype=np.int32)#[0]*len(syms)
        self.children = [None]*len(syms)
        self.alpha = alpha_factor
        self.alphainv = 1/self.alpha
        self.absizedivalpha = len(syms)/self.alpha
        self.beta = 1.0
        self.i = 0
    def _make_child(self):
        return VolfNode(self.syms, self.alpha)
    def get_child(self, idx):
        if self.children[idx] is None:
            self.children[idx] = self._make_child()
        return self.children[idx]
    def get_probs(self, ctx):
        if len(ctx):
            lc = ctx[-1]
            child = self.get_child(lc)
            child_probs = child.get_probs(ctx[:-1])
            cond_probs = self._weight_probs(child_probs)
            return cond_probs
        return self._pe_arr()
    def _weight_probs(self, probs):
        beta_tag = self.beta / (self.absizedivalpha + self.counts.sum())
        child_probs = probs + (self.counts+self.alphainv)*beta_tag
        return child_probs/child_probs.sum()
    def _pe_arr(self):
        return (self.counts +self.alphainv)/(self.counts.sum() + self.absizedivalpha)
    def _incr_count(self, sym):
        self.counts[sym] += 1
        # Rescale if needed
        if self.counts[sym] < RESCALE_THRESHOLD_COUNT:
            return
        # div by 2, and round up
        self.counts = (self.counts >> 1) + (self.counts&1)

    def learn(self, ctx, sym):
        if len(ctx):
            lc = ctx[-1]
            child = self.get_child(lc)
            child_probs = child.learn(ctx[:-1], sym)
            cond_probs = self._weight_probs(child_probs)
            # update beta, consts are from thesis
            if (self.beta > 1500000) or (self.beta < (1.0/1500000)):
                self.beta /= 2
            else:
                child_pw = child_probs[sym]
                beta_tag = self.beta / (self.counts.sum() + self.absizedivalpha)
                self.beta = (beta_tag * (self.counts[sym] + self.alphainv))/child_pw
            ret = cond_probs
        else:
            ret = self._pe_arr()
        self._incr_count(sym)
        return ret

    def prob(self, ctx, sym):
        probarr = self.get_probs(ctx)
        return probarr[sym]

from ctw import MemoryDeque
from itertools import accumulate
class VolfModel:
    def __init__(self, syms, depth, alpha=DEFAULT_ALPHA):
        self.syms = syms
        self.sym_to_idx = {s:i for i,s in enumerate(syms)}
        self.idx_to_sym = {i:s for i,s in enumerate(syms)}
        self.root = VolfNode(list(range(len(syms))), alpha)
        self.past_idxs = MemoryDeque(maxlen=depth)
    def learn(self, sym):
        idx = self.sym_to_idx[sym]
        self.root.learn(ctx=self.past_idxs.view(), sym=idx)
        self.past_idxs.append(idx)
    def probs(self):
        return self.root.get_probs(self.past_idxs.view())
    def pmf(self, sym):
        return self.probs()[self.sym_to_idx[sym]]
    def exclusive_cmf(self, sym):
        return sum(self.probs()[:self.sym_to_idx[sym]])
    def find_sym_from_cum_prob(self, cum_prob):
        probs = self.probs()
        cdfs = list(accumulate(probs, initial=0))
        ranges = [(a,b) for a,b in zip(cdfs, cdfs[1:])]
        idx = [i for i, (a,b) in enumerate(ranges) if cum_prob >= a and cum_prob < b][0]
        return self.idx_to_sym[idx]


import sys
sys.path.insert(0,"../")
from corpus.corpus import get_sum, bit_iter, byte_iter
from scripts.order0test import ac_correctness, entropy_stats

# ctor = lambda syms: VolfModel(syms=syms, depth=2)
# ac_correctness(ctor)
# entropy_stats(ctor)
# sum_text = get_sum()
bits = list(bit_iter(get_sum()))

# vm = VolfModel([0,1], 5)
from tqdm import tqdm
vm = VolfModel([0, 1], 5)
for b in tqdm(bits):
    vm.learn(b)
    #print(f"Learned: {b} pmf: {vm.pmf(b):g}")
print(vm.pmf(0))
