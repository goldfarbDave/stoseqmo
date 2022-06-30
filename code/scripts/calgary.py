import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df = pd.read_csv("../harness/calgary.csv")
# Add b/B
df = df.join((df["Entropy"]/df["FSize"]).to_frame(name="b/B"))

# Top 3 for each file
# for fn in np.unique(df["File"]):
#     print(fn)
#     ndf = df[df["File"] == fn]
#     print(ndf.sort_values(by='b/B'))
    #print(ndf.sort_values(by='b/B').head(2))
fns = np.unique(df["File"])
shape = (3,6)
fig, axs = plt.subplots(*shape)
coords = np.arange(18).reshape(3,6)
to_coords = lambda idx: (np.where(coords==idx)[0][0], np.where(coords==idx)[1][0])
light_color_map = {
    "red": "lightcoral",
    "blue": "lightblue",
    "green": "lightgreen"
}
for idx, fn in enumerate(fns):
    r,c = to_coords(idx)
    ax = axs[r,c]
    ndf = df[df["File"] == fn]
    #for meth,color in [ ("SM1PF", 'red'),  ("SMUKN", 'blue'),("CTW", 'green'),]:
    for meth, color in [("PPMDP", 'red'), ("SMUKN", "blue"), ("PPMDPFull", 'green')]:
        baseline = ndf[ndf["Meth"] == meth]
        hash_meths = ndf[ndf["Meth"] == f"Hash{meth}"].sort_values(by='MSize')
        amn_meths = ndf[ndf["Meth"] == f"Amnesia{meth}"].sort_values(by='MSize')
        lb_meths = ndf[ndf["Meth"] == f"LengthBucketHash{meth}"].sort_values(by='MSize')
        ax.plot(hash_meths["MSize"].values, hash_meths["b/B"], color=color, label=f"Stochastic-{meth} Compression Ratio")
        # ax.plot(amn_meths["MSize"].values, amn_meths["b/B"], color=color, linestyle='dashdot', label=f"Amnesia-{meth} Compression Ratio")
        # ax.plot(lb_meths["MSize"].values, lb_meths["b/B"], color="purple", label=f"Length Bucket Stochastic {meth} Compression Ratio")
        ax.axhline(y=baseline["b/B"].values[0], linestyle='dashed', color=color, label=f"{meth} Compression Ratio")
        ax.axvline(x=baseline["MSize"].values[0], linestyle='dotted', color=color, label=f"{meth} Histograms used")
    ax.set_xscale("log", base=2)
    # ax.set_xlabel("(# of Histograms)")
    # ax.set_ylabel("Compressed bits/Uncompressed Byte")
    # ax.set_title("Compression ratio vs # of Histograms: bib")
    ax.set_title(fn)
ax.legend()
plt.show()
