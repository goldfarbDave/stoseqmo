import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from pathlib import Path
import argparse
try:
    srcdir = Path(__file__).resolve().parent
except NameError:
    srcdir = Path("./")
parser = argparse.ArgumentParser()
parser.add_argument("-svg", type=Path,
                    default=srcdir / "calgdemo.svg")
args = parser.parse_args()
svgpath = args.svg
svgpath = "calgdemo.svg"
csvpath = srcdir/"../harness/calgary.csv"
df = pd.read_csv(csvpath)
# Add b/B
df = df.join((df["Entropy"]/df["FSize"]).to_frame(name="b/B"))

fns = np.unique(df["File"])
wanted_fns = ["bib", "book1", "obj1", "pic", "progp"]

shape = (3, 2)
fig, axs = plt.subplots(*shape, sharex='col')
dpi = fig.get_dpi()

fig.set_size_inches(6, 9)
# fig.set_size_inches(1920/dpi, 1200/dpi)
# fig.set_size_inches(11.69,8.27)
ax_l = [(0,0),
        (1,0),
        (2,0),
        (0,1),
        (2,1)]
ax = axs[1,1]
ax.clear()
ax.set_axis_off()

coords = np.arange(np.prod(shape)).reshape(*shape)
to_ax = lambda idx: axs[np.where(coords==idx)[0][0], np.where(coords==idx)[1][0]]
light_color_map = {
    "red": "lightcoral",
    "blue": "lightblue",
    "green": "lightgreen"
}
binary_data_fns = {"obj1", "obj2", "pic", "geo"}
for idx, fn in enumerate(wanted_fns):
    # ax = to_ax(idx)
    ax = axs[ax_l[idx][0], ax_l[idx][1]]
    ndf = df[df["File"] == fn]
    for meth, color in [("PPMDP", 'red'), ("SMUKN", "blue"), ("CTW", 'black')]:
        baseline = ndf[ndf["Meth"] == meth]
        hash_meths = ndf[ndf["Meth"] == f"Hash{meth}"].sort_values(by='MSize')
        fnv_meths = ndf[ndf["Meth"] == f"FNVHash{meth}"].sort_values(by='MSize')
        amn_meths = ndf[ndf["Meth"] == f"Amnesia{meth}"].sort_values(by='MSize')
        ax.plot(hash_meths["MSize"].values, hash_meths["b/B"], color=color, label=f"Hash-{meth} Compression Ratio")
        ax.axhline(y=baseline["b/B"].values[0], linestyle='dashed', color=color, label=f"{meth} Compression Ratio")
    ax.axvline(x=baseline["MSize"].values[0], linestyle='dotted', color=color, label=f"Unbounded Histograms used")
    ax.set_xscale("log", base=2)
    if fn in binary_data_fns:
        ax.set_facecolor("grey")
    ax.set_title(fn)
xlabel = "Num of Histograms"
ylabel = "Compressed bits/Uncompressed Byte"
title = "Compression ratio vs Num of Histograms: Calgary"
# ax.legend()
handles,labels = ax.get_legend_handles_labels()
ax = axs[1,1]
ax.legend(handles, labels, loc="center")
fig.supxlabel(xlabel)
fig.supylabel(ylabel)
fig.suptitle(title)
# plt.savefig(pgfpath, format='pgf')
# plt.tight_layout()
plt.savefig(svgpath, format='svg')
# plt.show()
