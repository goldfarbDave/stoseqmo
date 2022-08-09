from common import plt as plt
import common
import numpy as np

# baseline_df = common.get_csv_df("shakespeare.csv")
# for meth,color in [("CTW", "black"), ("SMUKN", "blue"), ("SM1PF", "orange"), ("PPMDP", "red")]:
#     df = baseline_df.copy()
#     ctw_entropy=df[df["Meth"] == meth]["Entropy"].values[0]
#     #df = df.join((((df["Entropy"]/ctw_entropy)-1)*100).to_frame(name="PerCTWEnt"))
#     df = df.join((100 -(((df["Entropy"]/ctw_entropy)-1)*100)).to_frame(name="PerCTWEnt"))
#     ctw_size = (df[df["Meth"] == meth]["MSize"].values[0])
#     df = df.join((((df["MSize"]/ctw_size))*100).to_frame(name="PerCTWSize"))
#     hmstr = f"Hash{meth}"
#     amnstr = f"Amnesia{meth}"
#     hash_meths = df[df["Meth"] == hmstr]
#     amn_meths = df[df["Meth"] == amnstr]
#     baseline=df[df["Meth"] == meth]
#     hash_bpc = hash_meths["Entropy"]/hash_meths["FSize"]
#     baseline_bpc = baseline["Entropy"]/baseline["FSize"]
#     baseline_tsize = baseline["MSize"]
#     hash_meths = hash_meths.sort_values(by="PerCTWSize")
#     plt.plot(hash_meths["PerCTWSize"].values, hash_meths["PerCTWEnt"].values,
#              label=f"{common.to_nmeth(meth)}",
#              **common.get_style_dict(hmstr))
#     amn_meths = amn_meths.sort_values(by="PerCTWSize")
#     plt.plot(amn_meths["PerCTWSize"].values, amn_meths["PerCTWEnt"].values,
#              label=f"{common.to_amnesia(meth)}",
#              **common.get_style_dict(amnstr))
# plt.xlim(-1, 100)
# plt.ylim(80, 100)
# plt.xlabel("Percent of Unbounded Histograms")
# plt.ylabel("Percent Larger Information Content Under Model")
# plt.title("Pc. Unbounded Memory Usage vs. Pc. Worse Effectiveness")
# plt.legend()
# common.do_plot()
