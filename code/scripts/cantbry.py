import numpy as np
import common
from common import plt as plt
df = common.get_csv_df("cantbry.csv")
# Add b/B
df = df.join((df["Entropy"]/df["FSize"]).to_frame(name="b/B"))

fns = np.unique(df["File"])
shape = (2,6)
fig, axs = plt.subplots(*shape)
sg_ax = axs[1,5]
sg_ax.clear()
sg_ax.set_axis_off()

dpi = fig.get_dpi()


fig.set_size_inches(1920/dpi, 1200/dpi)
coords = np.arange(np.prod(shape)).reshape(*shape)
to_coords = lambda idx: (np.where(coords==idx)[0][0], np.where(coords==idx)[1][0])
for idx, fn in enumerate(fns):
    r,c = to_coords(idx)
    ax = axs[r,c]
    ndf = df[df["File"] == fn]
    ax.axvline(x=ndf[ndf["Meth"] == "CTW"]["MSize"].values[0],
               **common.get_style_dict("Unbounded"))
    for meth in ["PPMDP", "SMUKN","CTW"]:
        baseline = ndf[ndf["Meth"] == meth]
        ax.axhline(y=baseline["b/B"].values[0],
                   **common.get_style_dict(meth))
        hmstr = f"Hash{meth}"
        hash_meths = ndf[ndf["Meth"] == hmstr].sort_values(by='MSize')
        ax.plot(hash_meths["MSize"].values, hash_meths["b/B"],
                **common.get_style_dict(hmstr))
    ax.set_xscale("log", base=2)
    if fn in common.BIN_DATA_FNS:
        ax.set_facecolor(common.GREY)
    ax.set_title(fn)
xlabel = "Num of Histograms"
ylabel = "Compressed bits/Uncompressed Byte"
title = "Compression ratio vs Num of Histograms: Canterbury"
handles,labels = ax.get_legend_handles_labels()
sg_ax.legend(handles,labels, loc='center')
fig.supxlabel(xlabel)
fig.supylabel(ylabel)
fig.suptitle(title)
common.do_plot()
