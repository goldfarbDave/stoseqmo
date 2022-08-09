import numpy as np
import common
from common import plt as plt
df = common.get_csv_df("calgary.csv")
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

fig.set_size_inches(1920/dpi, 1200/dpi)
coords = np.arange(18).reshape(3,6)
to_coords = lambda idx: (np.where(coords==idx)[0][0], np.where(coords==idx)[1][0])
light_color_map = {
    "red": "lightcoral",
    "blue": "lightblue",
    "green": "lightgreen"
}
steppingmap = {"red": {k:v for k, v in zip(range(3),["peachpuff", "sandybrown", "saddlebrown"])},
               "blue": {k:v for k, v in zip(range(3),["lightcyan", "cyan", "darkcyan"])},
               "black": {k:v for k, v in zip(range(3),["lightgrey", "grey", "darkgrey"])},}
widthmap =  {k:v for k,v in zip(range(3), [3, 2, 1])}
binary_data_fns = {"obj1", "obj2", "pic", "geo"}
for idx, fn in enumerate(fns):
    r,c = to_coords(idx)
    ax = axs[r,c]
    ndf = df[df["File"] == fn]
    ax.axvline(x=ndf[ndf["Meth"] == "CTW"]["MSize"].values[0],
               **common.get_style_dict("Unbounded"))
    for meth in ["PPMDP"]:
        baseline = ndf[ndf["Meth"] == meth]
        ax.axhline(y=baseline["b/B"].values[0],
                   **common.get_style_dict(meth))
        hmstr = f"Hash{meth}"
        hash_meths = ndf[ndf["Meth"] == hmstr].sort_values(by='MSize')
        ax.plot(hash_meths["MSize"].values, hash_meths["b/B"],
                linewidth=4,
                **common.get_style_dict(hmstr))
        color = common.get_style_dict(hmstr)['color']
        for pur in range(3):
            pur_meths = ndf[ndf["Meth"] == f"Pure{pur}Hash{meth}"].sort_values(by='MSize')
            ax.plot(pur_meths["MSize"].values, pur_meths["b/B"],
                    label=f"Pure{pur}-{common.to_nmeth(meth)}",
                    color=steppingmap[color][pur], linestyle="dashdot", linewidth=widthmap[pur])
    ax.set_xscale("log", base=2)
    if fn in common.BIN_DATA_FNS:
        ax.set_facecolor(common.GREY)
    ax.set_title(fn)
xlabel = "Num of Histograms"
ylabel = "Compressed bits/Uncompressed Byte"
title = "Compression ratio vs Num of Histograms: Calgary. Pure"
ax.legend()
fig.supxlabel(xlabel)
fig.supylabel(ylabel)
fig.suptitle(title)
common.do_plot()
