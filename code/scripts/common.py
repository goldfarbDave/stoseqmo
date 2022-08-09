import matplotlib as mpl
import matplotlib.font_manager as font_man
import matplotlib.pyplot as plt
# For some reason CMR can't be found on a standard ubuntu install, so using the below instead
# plt.rcParams.update({
#     "font.family": "serif",
#     "font.serif": ["Computer Modern Roman"]})
_cmrfont = font_man.FontProperties(fname=mpl.get_data_path() + "/fonts/ttf/cmr10.ttf")
plt.rcParams.update({
    "font.family": "serif",
    "font.serif": _cmrfont.get_name(),
    "mathtext.fontset":'cm',
    "axes.unicode_minus":False
})
import argparse
from pathlib import Path
import pandas as pd


#GREY= "#B4B4B4"
GREY="#909090"


BIN_DATA_FNS= {
    # Calgary
    "obj1", "obj2", "pic", "geo",
    # Cantbry
    "kennedy.xls", "ptt5", "sum"}

meth_cmap = {
    "PPMDP": "red",
    "SMUKN": "blue",
    "SM1PF": "teal",
    "CTW": "black",
    "PPMDPFull": "green",
}

def to_nmeth(meth_name):
    return f"Hash{meth_name}"
def to_amnesia(meth_name):
    return f"Amnesia{meth_name}"
def to_fnv(meth_name):
    return f"FNVHash{meth_name}"
def get_style_dict(meth):
    if meth in meth_cmap.keys():
        return {"linestyle": "dashed",
                "color": meth_cmap[meth]}
    if meth == "Unbounded":
        return {"linestyle": "dotted",
                "color": "black"}
    if meth.startswith("Hash"):
        mname = meth[len("Hash"):]
        return {"linestyle": "solid",
                "color": meth_cmap[mname]}
    if meth.startswith("FNVHash"):
        mname = meth[len("FNVHash"):]
        return {"linestyle": (0, (1,1)), #"densely dotted",
                "color": meth_cmap[mname]}

    if meth.startswith("Amnesia"):
        mname = meth[len("Amnesia"):]
        return {"linestyle": (0, (5,1)), #"densely dashed",
                "color": meth_cmap[mname]}

    else:
        import sys
        print("UNRESOLVED STYLE FOR: " + meth)
        sys.exit(1)


def do_plot():
    if _is_interactive():
        plt.show()
    else:
        parser = argparse.ArgumentParser()
        parser.add_argument("-svg", type=Path)
        args = parser.parse_args()
        plt.savefig(args.svg, format='svg')


def _is_interactive():
    import sys
    try:
        sys.ps1
        return True
    except AttributeError:
        return False

def get_csv_df(csv_fn):
    import inspect
    # Highest thing on the stack which is in this project (works for interactive and other usecases)
    caller_file_path = Path([frame for frame in inspect.stack()
                             if "proj/thes/" in frame.filename][-1].filename)
    srcdir = caller_file_path.resolve().parent
    csvpath = (srcdir/"../harness")/csv_fn
    return pd.read_csv(csvpath)
