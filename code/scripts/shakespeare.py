import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
df = pd.read_csv("../harness/shakespeare.csv")
ctw_entropy=df[df["Meth"] == "CTW"]["Entropy"].values[0]
df = df.join((((df["Entropy"]/ctw_entropy)-1)*100).to_frame(name="PerCTWEnt"))
ctw_size = (df[df["Meth"] == "CTW"]["MSize"].values[0])
df = df.join((((df["MSize"]/ctw_size))*100).to_frame(name="PerCTWSize"))
hash_meths = df[df["Meth"].str.startswith("Hash")]
baseline=df[df["Meth"] == "CTW"]
hash_bpc = hash_meths["Entropy"]/hash_meths["FSize"]
baseline_bpc = baseline["Entropy"]/baseline["FSize"]
baseline_tsize = baseline["MSize"]

plt.plot(hash_meths["PerCTWSize"].values, hash_meths["PerCTWEnt"].values)
plt.xlim(-1, 80)
plt.ylim(0, 20)
plt.xlabel("% of CTW Histograms")
plt.ylabel("% Larger Information Content")
plt.show()


# fig, ax = plt.subplots(figsize=[5,4])
# ax.plot(hash_meths["MSize"].tolist(), hash_bpc.to_list(), color="blue")
# ax.axhline(y=baseline_bpc.tolist()[0], linestyle='dashed', color="red")
# ax.axvline(x=baseline_tsize.tolist()[0], linestyle='dashed', color="red")
# ax.set_xscale("log", base=2)
# ax.set_xlabel("(# of Histograms)")
# ax.set_ylabel("bits/Byte")
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
# plt.show()
