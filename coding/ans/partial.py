import pickle
with open("symprobs.pkl", 'rb') as f:
    symprobs = pickle.load(f)
precision = 16
denom = 2**precision
ansnumers = {k: int(denom*v) for k, v in symprobs.items()}
ansprobs = {k: v/denom for k,v in ansnumers.items()}
scaled_probs = list(ansnumers.values())
let_to_sym = {k: i for i, k in enumerate(ansnumers.keys())}
sym_to_let = {i: k for i, k in enumerate(ansnumers.keys())}
class UniformCoder:
    def __init__(self):
        self.compressed = 1
    def encode(self, sym, base):
        self.compressed = (self.compressed * base) + sym
    def decode(self, base):
        sym = self.compressed % base
        self.compressed //= base
        return sym
    def size(self):
        return len(f"{self.compressed:b}")

class ANSC:
    def __init__(self, prec):
        self.prec = prec
        self.compressed = 1
        self.prec_mask = (1 << self.prec) - 1
    def encode(self, symbol, scaled_probs):
        # Decode Z given posterior.
        base = scaled_probs[symbol]
        z = self.compressed % base + sum(scaled_probs[:symbol])
        self.compressed //= base
        # Encode Z
        self.compressed = (self.compressed << self.prec) + z
    def decode(self, scaled_probs):
        # Decode Z
        z = self.compressed & self.prec_mask
        self.compressed >>= self.prec
        # Find corresponding symbol
        for i, p in enumerate(scaled_probs):
            if z < p:
                sym = i
                break
            else:
                z -= p
        # Encode Z
        self.compressed = self.compressed * scaled_probs[sym] + z
        return sym
    def size(self):
        return len(f"{self.compressed:b}")

ansc = ANSC(prec=precision)

# s = "hellothisissometext"
# from glob import iglob
# fn = sorted(iglob("/home/etna/proj/camwork/**/*.org", recursive=True))[0]
# with open(fn) as f:
#     content = f.read().lower()
# alpha = {chr(x) for x in range(ord('a'), ord('z')+1)} | {' '}
# filtc = "".join([c for c in content if c in alpha])
# with open("testcontent.pkl", 'wb') as f:
#     pickle.dump(filtc, f)

with open("testcontent.pkl", 'rb') as f:
    filtc = pickle.load(f)
for c in reversed(filtc):
    ansc.encode(let_to_sym[c], scaled_probs)
si = ansc.size()
res = "".join([sym_to_let[ansc.decode(scaled_probs)] for _ in range(len(filtc))])
print(f"improvement {len(filtc)*5}->{si}")

assert res== filtc
