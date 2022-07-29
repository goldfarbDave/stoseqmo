import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

baseline_df = pd.read_csv("../harness/shakespeare.csv")
for meth,color in [("CTW", "black"), ("SMUKN", "blue"), ("SM1PF", "teal"), ("PPMDP", "red")]:
    df = baseline_df.copy()
    ctw_entropy=df[df["Meth"] == meth]["Entropy"].values[0]
    df = df.join((((df["Entropy"]/ctw_entropy)-1)*100).to_frame(name="PerCTWEnt"))
    ctw_size = (df[df["Meth"] == meth]["MSize"].values[0])
    df = df.join((((df["MSize"]/ctw_size))*100).to_frame(name="PerCTWSize"))
    hash_meths = df[df["Meth"] == f"Hash{meth}"]
    baseline=df[df["Meth"] == meth]
    hash_bpc = hash_meths["Entropy"]/hash_meths["FSize"]
    baseline_bpc = baseline["Entropy"]/baseline["FSize"]
    baseline_tsize = baseline["MSize"]
    hash_meths = hash_meths.sort_values(by="PerCTWSize")
    plt.plot(hash_meths["PerCTWSize"].values, hash_meths["PerCTWEnt"].values, color=color, label=meth)
# plt.xlim(-1, 80)
# plt.ylim(0, 20)
plt.xlabel("Percent of Unbounded Histograms")
plt.ylabel("Percent Larger Information Content Under Model")
plt.title("Percent of Unbounded Memory Usage vs. Percent Worse Compression effectiveness")
plt.legend()
plt.show()

# .plot(hash_meths["MSize"].tolist(), hash_bpc.to_list(), color="blue", label="Stochastic-CTW Compression Ratio")
# ax.axhline(y=baseline_bpc.tolist()[0], linestyle='dashed', color="red", label="CTW Compression Ratio")
# ax.axvline(x=baseline_tsize.tolist()[0], linestyle='dotted', color="red", label="CTW Histograms used")
# ax.set_xscale("log", base=2)
# ax.set_xlabel("(# of Histograms)")
# ax.set_ylabel("Compressed bits/Uncompressed Byte")
# ax.set_title("Compression ratio vs # of Histograms: Shakespeare")
# ax.legend()
# axins = ax.inset_axes([.5, .5, .47, .47])
# axins.plot(hash_meths["MSize"].tolist(), hash_bpc.to_list(), color="blue")
# axins.axhline(y=baseline_bpc.tolist()[0], linestyle='dashed', color="red")
# axins.axvline(x=baseline_tsize.tolist()[0], linestyle='dashed', color="red")
# axins.set_xscale("linear")
# #axins.set_xscale("log", base=2)
# axins.set_xlim(1<<17, 2**20+2**17)
# axins.set_ylim(baseline_bpc.tolist()[0]-.1, 2.5)
# axins.set_xticklabels([])
# #axins.set_yticklabels([])
# ax.indicate_inset_zoom(axins, edgecolor="k")
# plt.savefig("shakespeare.svg", format='svg')
plt.show()
