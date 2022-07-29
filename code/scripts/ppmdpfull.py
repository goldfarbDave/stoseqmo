import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import argparse
try:
    srcdir = Path(__file__).resolve().parent
except NameError:
    srcdir = Path("./")
parser = argparse.ArgumentParser()
parser.add_argument("-svg", type=Path,
                    default=srcdir / "ppmdpfull.svg")
args = parser.parse_args()
svgpath = args.svg
csvpath = srcdir/"../harness/calgary.csv"
df = pd.read_csv(csvpath)
# Add b/B
df = df.join((df["Entropy"]/df["FSize"]).to_frame(name="b/B"))

fns = np.unique(df["File"])
shape = (3,6)
fig, axs = plt.subplots(*shape)
plt.subplots_adjust(
    top=0.88,
bottom=0.11,
left=0.11,
right=0.9,
hspace=0.2,
wspace=0.2
)
dpi = fig.get_dpi()

# fig.set_size_inches(11.69, 7.27)
fig.set_size_inches(1920/dpi, 1200/dpi)
coords = np.arange(18).reshape(3,6)
to_coords = lambda idx: (np.where(coords==idx)[0][0], np.where(coords==idx)[1][0])
light_color_map = {
    "red": "lightcoral",
    "blue": "lightblue",
    "green": "lightgreen"
}
binary_data_fns = {"obj1", "obj2", "pic", "geo"}
for idx, fn in enumerate(fns):
    r,c = to_coords(idx)
    ax = axs[r,c]
    ndf = df[df["File"] == fn]
    #for meth,color in [ ("SM1PF", 'red'),  ("SMUKN", 'blue'),("CTW", 'green'),]: ,("PPMDPFull", 'green'),
    for meth, color in [("PPMDP", 'red'), ("PPMDPFull", 'green')]:
        baseline = ndf[ndf["Meth"] == meth]
        hash_meths = ndf[ndf["Meth"] == f"Hash{meth}"].sort_values(by='MSize')
        fnv_meths = ndf[ndf["Meth"] == f"FNVHash{meth}"].sort_values(by='MSize')
        amn_meths = ndf[ndf["Meth"] == f"Amnesia{meth}"].sort_values(by='MSize')
        ax.plot(hash_meths["MSize"].values, hash_meths["b/B"], color=color, label=f"Stochastic-{meth} Compression Ratio")
        # ax.plot(fnv_meths["MSize"].values, fnv_meths["b/B"], color=color, linestyle="dashdot", label=f"FNV-{meth} Compression Ratio")
        # ax.plot(amn_meths["MSize"].values, amn_meths["b/B"], color=light_color_map[color], label=f"Amnesia-{meth} Compression Ratio")
        # ax.plot(amn_meths["MSize"].values, amn_meths["b/B"], color=color, linestyle='dashdot', label=f"Amnesia-{meth} Compression Ratio")
        # ax.plot(lb_meths["MSize"].values, lb_meths["b/B"], color="purple", label=f"Length Bucket Stochastic {meth} Compression Ratio")
        ax.axhline(y=baseline["b/B"].values[0], linestyle='dashed', color=color, label=f"{meth} Compression Ratio")
    ax.axvline(x=baseline["MSize"].values[0], linestyle='dotted', color=color, label=f"Unbounded Histograms used")

    ax.set_xscale("log", base=2)
    if fn in binary_data_fns:
        ax.set_facecolor("grey")
    ax.set_title(fn)
xlabel = "Num of Histograms"
ylabel = "Compressed bits/Uncompressed Byte"
title = "Compression ratio vs Num of Histograms: Calgary. Full updates"
ax.legend()
fig.supxlabel(xlabel)
fig.supylabel(ylabel)
fig.suptitle(title)
# plt.savefig(pgfpath, format='pgf')
plt.savefig(svgpath, format='svg')
# plt.show()
