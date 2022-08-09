from common import plt as plt
import common
baseline_df = common.get_csv_df("shakespeare.csv")
for meth in ["CTW", "SMUKN", "SM1PF", "PPMDP", "PPMDPFull"]:
    df = baseline_df.copy()
    ctw_entropy=df[df["Meth"] == meth]["Entropy"].values[0]
    best_meth = df[df["Meth"] == "SM1PF"]
    best_entropy = best_meth["Entropy"].values[0]
    best_size = best_meth["MSize"].values[0]
    df = df.join((2-(df["Entropy"]/best_entropy)).to_frame(name="PropBestEnt"))
    df = df.join((((df["MSize"]/best_size))).to_frame(name="PropBestSize"))
    hmstr = f"Hash{meth}"
    amnstr = f"Amnesia{meth}"
    hash_meths = df[df["Meth"] == hmstr]
    amn_meths = df[df["Meth"] == amnstr]
    baseline=df[df["Meth"] == meth]
    hash_bpc = hash_meths["Entropy"]/hash_meths["FSize"]
    baseline_bpc = baseline["Entropy"]/baseline["FSize"]
    baseline_tsize = baseline["MSize"]
    hash_meths = hash_meths.sort_values(by="PropBestSize")
    plt.plot(hash_meths["PropBestSize"].values, hash_meths["PropBestEnt"].values,
             label=f"{common.to_nmeth(meth)}",
             **common.get_style_dict(hmstr))
    amn_meths = amn_meths.sort_values(by="PropBestSize")
    plt.plot(amn_meths["PropBestSize"].values, amn_meths["PropBestEnt"].values,
             label=f"{common.to_amnesia(meth)}",
             **common.get_style_dict(amnstr))
plt.axhline(y=1,
            label="SM1PF",
            **common.get_style_dict("SM1PF"))
plt.xlim(-.01,1)
plt.ylim(.7, 1.01)
# plt.xlim(.01,.1)
# plt.ylim(0, 1)
plt.xlabel("Proportion of Unbounded SM1PF Size")
plt.ylabel("Compression Proportion of SM1PF Entropy")
plt.title("Compression Proportion Against Unbounded SM1PF")
fig = plt.gcf()
fig.legend(loc="center right")
fig.subplots_adjust(right=.8)
common.do_plot()
