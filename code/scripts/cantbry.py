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
dpi = fig.get_dpi()


fig.set_size_inches(1920/dpi, 1200/dpi)
coords = np.arange(np.prod(shape)).reshape(*shape)
to_coords = lambda idx: (np.where(coords==idx)[0][0], np.where(coords==idx)[1][0])
for idx, fn in enumerate(fns):
    r,c = to_coords(idx)
    ax = axs[r,c]
    ndf = df[df["File"] == fn]
    for meth in ["PPMDP", "SMUKN","CTW"]:
        baseline = ndf[ndf["Meth"] == meth]
        hmstr = f"Hash{meth}"
        hash_meths = ndf[ndf["Meth"] == hmstr].sort_values(by='MSize')
        ax.plot(hash_meths["MSize"].values, hash_meths["b/B"],
                label=f"{common.to_nmeth(meth)}",
                **common.get_style_dict(hmstr))
        ax.axhline(y=baseline["b/B"].values[0], label=f"{meth}",
                   **common.get_style_dict(meth))
    ax.axvline(x=baseline["MSize"].values[0],
               label=f"Unbounded Histograms Used",
               **common.get_style_dict("Unbounded"))
    ax.set_xscale("log", base=2)
    if fn in common.BIN_DATA_FNS:
        ax.set_facecolor(common.GREY)
    ax.set_title(fn)
xlabel = "Num of Histograms"
ylabel = "Compressed bits/Uncompressed Byte"
title = "Compression ratio vs Num of Histograms: Canterbury"
handles,labels = ax.get_legend_handles_labels()
sg_ax.clear()
sg_ax.set_axis_off()
sg_ax.legend(handles,labels, loc='center')
# bbox = sg_ax.get_position()
# pos = (bbox.min+bbox.max)/2
# pos[0] -= .05
# fig.legend(handles, labels, loc=tuple(pos))
fig.supxlabel(xlabel)
fig.supylabel(ylabel)
fig.suptitle(title)
common.do_plot()
