import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df = pd.read_csv("../harness/calgary.csv")
# Add b/B
df = df.join((df["Entropy"]/df["FSize"]).to_frame(name="b/B"))

# Top 3 for each file
# for fn in np.unique(df["File"]):
#     print(fn)
#     ndf = df[df["File"] == fn]
#     print(ndf.sort_values(by='b/B'))
    #print(ndf.sort_values(by='b/B').head(2))
# fns = np.unique(df["File"])[:16]
# fig, axs = plt.subplots(4,4)
# for idx, fn in enumerate(fns):
#     ax = axs[idx//4, idx%4]
#     ndf = df[df["File"] == fn]
#     baseline = ndf[ndf["Meth"] == "SM"]
#     hash_meths = ndf[ndf["Meth"] == "HashSM"].sort_values(by='MSize')
#     ax.plot(hash_meths["MSize"].values, hash_meths["b/B"], color="blue", label="Stochastic-SM Compression Ratio")
#     ax.axhline(y=baseline["b/B"].values[0], linestyle='dashed', color="red", label="SM Compression Ratio")
#     ax.axvline(x=baseline["MSize"].values[0], linestyle='dotted', color="red", label="SM Histograms used")
#     ax.set_xscale("log", base=2)
#     # ax.set_xlabel("(# of Histograms)")
#     # ax.set_ylabel("Compressed bits/Uncompressed Byte")
#     # ax.set_title("Compression ratio vs # of Histograms: bib")
#     ax.set_title(fn)
# #ax.legend()
# plt.show()
