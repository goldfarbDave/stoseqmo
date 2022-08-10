import common
from common import plt as plt
df = common.get_csv_df("calgary.csv")

steppingmap = {k:v for k,v in zip(range(3), ["blue", "black", "purple"])}
widthmap =  {k:v for k,v in zip(range(3), [3, 2, 1])}
container = common.get_large_items("calgary")
for ax, fn in zip(container.ax_ar, container.fns):
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
                    color=steppingmap[pur], linestyle="dashdot", linewidth=widthmap[pur])

common.do_plot(container=container,
               title = "Pure Low-Order Context Histograms over Full Calgary Corpus")
