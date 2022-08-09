import numpy as np
import common
from common import plt as plt
df = common.get_csv_df("calgary.csv")
# Add b/B
df = df.join((df["Entropy"]/df["FSize"]).to_frame(name="b/B"))

fns = np.unique(df["File"])
wanted_fns = ["bib", "book1", "obj1", "pic", "progp"]

shape = (3, 2)
fig, axs = plt.subplots(*shape, sharex='col')
dpi = fig.get_dpi()
fig.set_size_inches(6, 9)
ax_l = [(0,0),
        (1,0),
        (2,0),
        (0,1),
        (2,1)]
#scapegoat axis for legend
sg_ax = axs[1,1]
sg_ax.clear()
sg_ax.set_axis_off()

coords = np.arange(np.prod(shape)).reshape(*shape)
for idx, fn in enumerate(wanted_fns):
    ax = axs[ax_l[idx][0], ax_l[idx][1]]
    ndf = df[df["File"] == fn]
    ax.axvline(x=ndf[ndf["Meth"] == "CTW"]["MSize"].values[0],
               **common.get_style_dict("Unbounded"))
    for meth in ["PPMDP", "CTW"]:
        baseline = ndf[ndf["Meth"] == meth]
        ax.axhline(y=baseline["b/B"].values[0],
                   **common.get_style_dict(meth))
        hmstr = f"Hash{meth}"
        hash_meths = ndf[ndf["Meth"] == hmstr].sort_values(by='MSize')
        ax.plot(hash_meths["MSize"].values, hash_meths["b/B"],
                **common.get_style_dict(hmstr))
        amnstr = f"Amnesia{meth}"
        amn_meths = ndf[ndf["Meth"] == amnstr].sort_values(by='MSize')
        ax.plot(amn_meths["MSize"].values, amn_meths["b/B"],
                **common.get_style_dict(amnstr))
    ax.set_xscale("log", base=2)
    if fn in common.BIN_DATA_FNS:
        ax.set_facecolor(common.GREY)
    ax.set_title(fn)
xlabel = "Num of Histograms"
ylabel = "Compressed bits/Uncompressed Byte"
title = "Compression ratio vs Num of Histograms: Calgary. Amnesia and Hashing"
handles,labels = ax.get_legend_handles_labels()
sg_ax.legend(handles, labels, loc="center")
fig.supxlabel(xlabel)
fig.supylabel(ylabel)
fig.suptitle(title)
common.do_plot()
