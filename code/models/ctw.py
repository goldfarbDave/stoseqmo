from itertools import accumulate
import math
from numpy import logaddexp, log1p
from scipy.special import logsumexp
import numpy as np
class Node:
    def __init__(self, num_children=2):
        self.counts = [0]*num_children
        self.lpe = 0
        self.lpw = 0
        self.children = [None]*num_children
        self.child_lws = 0.0
    def _make_child(self):
        return Node(num_children=len(self.children))
    def update_lpw(self):
        if all(c is None for c in self.children):
            self.lpw = self.lpe
            return
        self.child_lws = sum(c.lpw if c else 0.0 for c in self.children)
        self.lpw = logaddexp(self.lpe, self.child_lws) - np.log(2)
    def _lkt_factor(self, sym):
        # (self.counts[bit]+.5)/(sum(self.counts) + 1)
        # From above, we have a/d + 1/(2d) -> (2ad+d)/(2d^2)
        # Taking logs, we get:
        # return log1p(2*self.counts[sym]) - log1p(1) - log1p(sum(self.counts))
        return np.log(self.counts[sym] + (1/len(self.children))) - np.log(sum(self.counts) + 1)
    def insert(self, ctx, sym):
        if len(ctx):
            lc = ctx[-1]
            if self.children[lc] is None:
                self.children[lc] = self._make_child()
            self.children[lc].insert(ctx[:-1],sym)
        self.lpe += self._lkt_factor(sym)
        self.counts[sym] += 1
        self.update_lpw()
    def remove(self, ctx, sym):
        if len(ctx):
            lc = ctx[-1]
            self.children[lc].remove(ctx[:-1],sym)
            if self.children[lc].counts == [0,0]:
                self.children[lc] = None
        self.counts[sym] -= 1
        self.lpe -= self._lkt_factor(sym)
        self.update_lpw()
    def _children_not_at_idx(self, ex_idx):
        for idx, c in enumerate(self.children):
            if idx == ex_idx:
                continue
            yield c
    def theoretic_insert(self, ctx, sym):
        # Delegate, then bubble up lpw as if self.insert(ctx, sym) had been called, but don't modify state
        child_lws = 0.0
        if len(ctx):
            lc = ctx[-1]
            if self.children[lc] is None:
                self.children[lc] = self._make_child()
            active_child_lw = self.children[lc].theoretic_insert(ctx[:-1], sym)
            dormant_child_lw = sum(c.lpw if c else 0.0 for c in self._children_not_at_idx(lc))
            if all(c==0 for c in self.children[lc].counts):
                self.children[lc] = None
            child_lws = active_child_lw + dormant_child_lw
        new_lpe = self.lpe + self._lkt_factor(sym)
        if all(c is None for c in self.children):
            return new_lpe
        return logaddexp(new_lpe, child_lws) - np.log(2)

    def read_from_leaf(self, binstr):
        child_counts = []
        if len(binstr):
            # Return counts in traveling down the tree
            lc = binstr[-1]
            # Silently permit traversal towards novel path:
            if self.children[lc] is None:
                self.children[lc] = self._make_child()
            child_counts = self.children[lc].read_from_leaf(binstr[:-1])

        return [(self.counts, self.lpe, self.lpw)] + child_counts
    def read_from_root(self, binstr):
        return self.read_from_leaf(list(reversed(binstr)))

class MemoryDeque:
    # List with a view window
    def __init__(self, maxlen):
        self.data = []
        self.maxlen = maxlen
    def append(self, d):
        self.data.append(d)
    def view(self):
        return self.data[-self.maxlen:]
    def pop(self):
        return self.data.pop()

class CTW:
    def __init__(self, D=5, past=None, syms=[0,1]):
        self.ct = Node(num_children=len(syms))
        self.past_idxs = MemoryDeque(maxlen=D)
        self.syms = syms
        self.sym_to_idx = {s:i for i,s in enumerate(syms)}
        self.idx_to_sym = {i:s for i,s in enumerate(syms)}
        self.pcache=None
        if past is None:
            return
        for p in past:
            self.past_idxs.append(self.sym_to_idx[p])
    def learn(self, sym):
        self.pcache=None
        idx = self.sym_to_idx[sym]
        self.ct.insert(ctx=self.past_idxs.view(), sym=self.sym_to_idx[sym])
        self.past_idxs.append(idx)
    def unlearn(self, sym):
        self.pcache=None
        idx = self.sym_to_idx[sym]
        assert self.past_idxs.pop() == idx
        self.ct.remove(ctx=self.past_idxs.view(), sym=idx)
    def probs(self):
        if self.pcache is not None:
            return self.pcache
        start = self.ct.lpw
        # Note, joints don't sum to one as we're only considering our specific past history
        log_joint_probs = np.zeros(len(self.syms))
        for i in range(len(self.syms)):
            log_joint_probs[i] = self.ct.theoretic_insert(ctx=self.past_idxs.view(), sym=i)
        # un_norm = np.exp(log_joint_probs)
        # cond_probs = un_norm/np.sum(un_norm)
        norm = logsumexp(log_joint_probs)
        cond_probs = np.exp(log_joint_probs - norm)
        assert math.isclose(sum(cond_probs),1)
        if any(cp == 0 for cp in cond_probs):
            breakpoint()
        self.pcache = cond_probs
        return cond_probs
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



from dataclasses import dataclass
from typing import List, Tuple
from fractions import Fraction
@dataclass
class CTWTestCase:
    depth:int
    past:List[int]
    source:List[int]
    coord_counts:Tuple[List[int], List[List[int]]]
    end_pw:Fraction
test_cases = [
    # Reflections, Fig6
    CTWTestCase(
        depth=3,
        past=[1,1,0],
        source=[0,1,0,0,1,1,0],
        coord_counts=[
            ([0, 0, 0], [[4, 3], [2, 2], [0, 2], [0, 0]]),
            ([0, 0, 1], [[4, 3], [2, 2], [0, 2], [0, 2]]),
            ([0, 1, 0], [[4, 3], [2, 2], [2, 0], [1, 0]]),
            ([0, 1, 1], [[4, 3], [2, 2], [2, 0], [1, 0]]),
            ([1, 0, 0], [[4, 3], [2, 1], [1, 1], [1, 1]]),
            ([1, 0, 1], [[4, 3], [2, 1], [1, 1], [0, 0]]),
            ([1, 1, 0], [[4, 3], [2, 1], [1, 0], [1, 0]]),
            ([1, 1, 1], [[4, 3], [2, 1], [1, 0], [0, 0]]),
        ],
        end_pw=Fraction(7,2048)
    ),
    # 1995, Fig2
    CTWTestCase(
        depth=3,
        past=[0,1,0],
        source=[0,1,1,0,1,0,0],
        coord_counts=[
            ([0, 0, 0], [[4, 3], [2, 2], [0, 1], [0, 0]]),
            ([0, 0, 1], [[4, 3], [2, 2], [0, 1], [0, 1]]),
            ([0, 1, 0], [[4, 3], [2, 2], [2, 1], [2, 0]]),
            ([0, 1, 1], [[4, 3], [2, 2], [2, 1], [0, 1]]),
            ([1, 0, 0], [[4, 3], [2, 1], [1, 1], [0, 1]]),
            ([1, 0, 1], [[4, 3], [2, 1], [1, 1], [1, 0]]),
            ([1, 1, 0], [[4, 3], [2, 1], [1, 0], [1, 0]]),
            ([1, 1, 1], [[4, 3], [2, 1], [1, 0], [0, 0]]),
        ],
        end_pw=Fraction(95,32768)
    )]
def run_test(tc):
    st = CTW(D=tc.depth, past=tc.past)
    for bit in tc.source:
        st.learn(bit)
    # Ensure learning and unlearning doesn't bork things
    for bit in reversed(tc.source):
        st.unlearn(bit)
    for bit in tc.source:
        st.learn(bit)
    assert math.isclose(math.exp(st.ct.lpw),tc.end_pw)
    for tree_coord, counts in tc.coord_counts:
        emp_counts = [c[0] for c in st.ct.read_from_root(tree_coord)]
        assert counts == emp_counts
for test in test_cases:
    run_test(test)
