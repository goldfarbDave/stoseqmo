from collections import Counter
from glob import glob, iglob
import os
files = list(iglob("/home/etna/proj/camwork/**/*.org", recursive=True))
content = ""
for fn in files:
    with open(fn) as f:
        content += f.read()
lc = content.lower()
counts = Counter(lc)
chars = {chr(o) for o in range(ord('a'), ord('z') +1)}
chars.add(" ")
filtered = Counter({k:v for k,v in counts.items() if k in chars})
total = sum(filtered.values())
probs = {}
for k in sorted(filtered.keys()):
    probs[k] = filtered[k]/total
import pickle
with open("symprobs.pkl", 'wb') as f:
    pickle.dump(probs, f)
