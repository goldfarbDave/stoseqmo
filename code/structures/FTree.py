class FTree:
    # Largely ported from Wikipedia's c++ reference implementation
    def __init__(self, numel, init_value=1):
        # Initialize Fenwick tree of uniform array values
        self.ar = [0]*(numel+1) # 1-based array
        for i in range(len(self.ar)):
            self.update(i, init_value)
    def update(self, idx, val=1):
        idx += 1 # To 1-s based array
        while idx < len(self.ar):
            self.ar[idx] += val
            idx =self.next_idx(idx)
    def total(self):
        return self.exclusive_sum(len(self.ar)-1)
    def inclusive_sum(self, idx):
        return self.exclusive_sum(idx+1)
    def val_to_idx(self, val):
        # Binary search for lower_bound
        l = 0
        r = len(self.ar)-2
        while l != r:
            mid = (l+r) // 2
            if self.inclusive_sum(mid) <= val:
                l = mid+1
            else:
                r = mid
        return l
    def to_flat_cum(self):
        return [self.exclusive_sum(i) for i in range(len(self.ar)-1)]
    def to_flat_point(self):
        return [self.inclusive_sum(i)-self.exclusive_sum(i) for i in range(len(self.ar)-1)]

    def range_sum(self, send, end=None):
        # Use same semantics as =range=, so exclusive
        if not end:
            start = 0
            end = send
        else:
            start = send
        return self.exclusive_sum(end) - self.exclusive_sum(start)
    def exclusive_sum(self, idx):
        acc = 0
        while idx != 0:
            acc += self.ar[idx]
            idx = self.parent_idx(idx)
        return acc
    def parent_idx(self, idx):
        # Clear least-significant set bit (LSSB).
        # (idx & (-idx)) is effectively a bit-scan-forward + mask (all zeros, one at LSSB)
        return idx - (idx & (-idx))
    def next_idx(self, idx):
        # This has the result of traversing all right siblings of the node, and then proceeding to the parent's right siblings
        # Add masked least bit set
        return idx + (idx & (-idx))

# Should be fine to run tests on import for now
#def ftree_test():
tr = FTree(8, init_value=1)
assert tr.inclusive_sum(2) - tr.exclusive_sum(2) == 1
assert tr.inclusive_sum(2) == 3
assert tr.exclusive_sum(2) == 2
assert tr.range_sum(2, 3) == 1
assert tr.range_sum(0, 8) == tr.range_sum(8) == 8
assert tr.range_sum(8) == tr.inclusive_sum(7)
assert tr.total() == 8
# BST test:
assert tr.val_to_idx(5) == tr.to_flat_cum()[tr.val_to_idx(5)]
tr.update(5)
assert tr.val_to_idx(5) == tr.to_flat_cum()[tr.val_to_idx(5)]
assert tr.val_to_idx(0) == tr.to_flat_cum()[0]


#ftree_test()
