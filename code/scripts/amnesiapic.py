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
                    default=srcdir / "amnesia_pic.svg")
args = parser.parse_args()
svgpath = args.svg
csvpath = srcdir/"../harness/calgary.csv"
df = pd.read_csv(csvpath)
# Add b/B
df = df.join((df["Entropy"]/df["FSize"]).to_frame(name="b/B"))

fns = np.unique(df["File"])
ndf = df[df["File"] == "pic"]
for meth, color in [("PPMDP", 'red'), ("CTW", 'black')]: #("SMUKN", 'blue'),
    baseline = ndf[ndf["Meth"] == meth]
    hash_meths = ndf[ndf["Meth"] == f"Hash{meth}"].sort_values(by='MSize')
    amn_meths = ndf[ndf["Meth"] == f"Amnesia{meth}"].sort_values(by='MSize')
    plt.plot(hash_meths["MSize"].values, hash_meths["b/B"], color=color, label=f"Stochastic-{meth} Compression Ratio")
    plt.plot(amn_meths["MSize"].values, amn_meths["b/B"], color=color, linestyle='dashdot', label=f"Amnesia-{meth} Compression Ratio")
    plt.axhline(y=baseline["b/B"].values[0], linestyle='dashed', color=color, label=f"{meth} Compression Ratio")
plt.axvline(x=baseline["MSize"].values[0], linestyle='dotted', color=color, label=f"Unbounded Histograms used")
plt.xscale("log", base=2)
# plt.axis_bgcolor("grey")
    # plt.title(fn)
xlabel = "Num of Histograms"
ylabel = "Compressed bits/Uncompressed Byte"
title = "Compression ratio vs Num of Histograms: pic. Amnesia and Hashing"
plt.legend()
plt.xlabel(xlabel)
plt.ylabel(ylabel)
plt.title(title)
# plt.savefig(pgfpath, format='pgf')
plt.tight_layout()
plt.savefig(svgpath, format='svg')
# plt.show()
