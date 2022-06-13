import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
# with open("../models/hashing_calgary.csv") as f:
#     lines = [l.strip().split(',') for l in f.readlines()]
#     rows = lines[1:]
#     for row in rows:
#         fn, _, ts, depth, ent = row
with open("../models/ctw_sizemap.csv") as f:
    lines = [l.strip().split(',') for l in f.readlines()]
    ctw_size = {l[0]: int(l[1]) for l in lines[1:]}
df = pd.read_csv("../models/calgary.csv")
fns = np.unique(df["File"])

from pathlib import Path
fig, axs = plt.subplots(4,4)
for idx,fn in enumerate(fns):
    p = Path(f"../data/calgary/")/fn
    bytesize = p.stat().st_size
    narrowed = df[(df['File'] ==fn) & (df["Depth"] == 8)]
    # method = narrowed["Meth"]
    # entropy = narrowed["Entropy"]
    # print(method, entropy)
    #bpc = narrowed["Entropy"]/bytesize
    hash_ones = df[df["Meth"].str.startswith("Hash")]
    narrowed = hash_ones[(hash_ones['File'] == fn) & (hash_ones["Depth"] == 8)]
    baseline = df[(df['File'] == fn) & (df["Depth"] == 8) & (df["Meth"] == "CTW")]

    narrowed_bpc = narrowed["Entropy"]/bytesize
    baseline_bpc = baseline["Entropy"]/bytesize
    tsizes = narrowed["Meth"].str.slice(len("Hash"))
    dfn = tsizes.to_frame(name="TableSize").join(narrowed_bpc.to_frame(name="b/B"))
    ax = axs[idx//4, idx%4]
    ax.plot([int(i) for i in dfn["TableSize"].tolist()], dfn["b/B"].tolist(), color="blue")
    ax.axhline(y=baseline_bpc.tolist()[0], linestyle='dashed', color="red")
    ax.axvline(x=ctw_size[fn], linestyle='dashed', color="red")
    ax.set_xscale("log", base=2)
    ax.set_title(f"{fn}")

plt.xlabel("Table Size (# of Histograms)")
plt.ylabel("bits/Byte")
plt.show()
