import common
from common import plt as plt

baseline_df = common.get_csv_df("shakespeare.csv")
for meth in ["CTW","SMUKN", "SM1PF", "PPMDP", "PPMDPFull"]:
    df = baseline_df.copy()
    baseline_entropy=df[df["Meth"] == meth]["Entropy"].values[0]
    #df = df.join((1-((df["Entropy"]/baseline_entropy)-1)).to_frame(name="PropBaselineEnt"))
    df = df.join((2-(df["Entropy"]/baseline_entropy)).to_frame(name="PropBaselineEnt"))
    baseline_size = (df[df["Meth"] == meth]["MSize"].values[0])
    df = df.join(((df["MSize"]/baseline_size)).to_frame(name="PropBaselineSize"))
    hmstr = f"Hash{meth}"
    amnstr = f"Amnesia{meth}"
    hash_meths = df[df["Meth"] == hmstr]
    amn_meths = df[df["Meth"] == amnstr]
    baseline=df[df["Meth"] == meth]
    hash_bpc = hash_meths["Entropy"]/hash_meths["FSize"]
    baseline_bpc = baseline["Entropy"]/baseline["FSize"]
    baseline_tsize = baseline["MSize"]
    hash_meths = hash_meths.sort_values(by="PropBaselineSize")
    plt.plot(hash_meths["PropBaselineSize"].values, hash_meths["PropBaselineEnt"].values,
             **common.get_style_dict(hmstr))
    amn_meths = amn_meths.sort_values(by="PropBaselineSize")
    plt.plot(amn_meths["PropBaselineSize"].values, amn_meths["PropBaselineEnt"].values,
             **common.get_style_dict(amnstr))
sdict = common.get_style_dict("Unbounded")
sdict["label"] = "Unbounded"
plt.axhline(y=1,
            **sdict)
plt.xlim(-.01,1)
plt.ylim(.7, 1.01)
plt.xlabel("Proportion of Unbounded Model Size")
plt.ylabel("Compression Proportion against Model")
plt.title("Relative Compression Proportion per Model")
fig = plt.gcf()
fig.legend(loc="center right")
fig.subplots_adjust(right=.8)
# plt.legend()
common.do_plot()
