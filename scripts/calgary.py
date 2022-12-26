import common
from common import plt as plt
df = common.get_csv_df("calgary.csv")
container = common.get_large_items("calgary")
for ax, fn in zip(container.ax_ar, container.fns):
    ndf = df[df["File"] == fn]
    ax.axvline(x=ndf[ndf["Meth"] == "CTW"]["MSize"].values[0],
               **common.get_style_dict("Unbounded"))
    for meth in ["PPMDP", "SMUKN", "CTW"]:
        baseline = ndf[ndf["Meth"] == meth]
        ax.axhline(y=baseline["b/B"].values[0],
                   **common.get_style_dict(meth))
        hmstr = f"Hash{meth}"
        hash_meths = ndf[ndf["Meth"] == hmstr].sort_values(by='MSize')
        ax.plot(hash_meths["MSize"].values, hash_meths["b/B"],
                **common.get_style_dict(hmstr))

common.do_plot(container=container,
               title = "Random Hashing over Calgary Corpus")
