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
import numpy as np

# Lightest Grey?
GREY="#E0E0E0"
# Lighter Grey
# GREY= "#B4B4B4"
# GREY="#909090"


BIN_DATA_FNS= {
    # Calgary
    "obj1", "obj2", "pic", "geo",
    # Cantbry
    "kennedy.xls", "ptt5", "sum"}

tolBrightGreen="#228833"
tolBrightCyan="#66CCEE"
tolBrightGrey="#BBBBBB"
tolBrightBlue="#4477AA"
tolBrightPurple="#AA3377"
tolBrightRed="#EE6677"
tolBrightYellow="#CCBB44"

meth_cmap = {
    "PPMDP": tolBrightRed,
    "SMUKN": tolBrightBlue,
    "SM1PF": tolBrightCyan,
    "CTW": tolBrightGreen,
    "PPMDPFull": tolBrightPurple,
}

def to_nmeth(meth_name):
    return f"{meth_name} (with Hashing)"
def to_amnesia(meth_name):
    return f"{meth_name} (with Amnesia)"
def to_fnv(meth_name):
    return f"{meth_name} (with FNV Hashing)"
def get_style_dict(meth):
    if meth in meth_cmap.keys():
        return {"linestyle": "dashed",
                "color": meth_cmap[meth],
                "label": meth}
    if meth == "Unbounded":
        return {"linestyle": "dotted",
                "color": "black",
                "label": "Unbounded Histograms Used"}
    if meth.startswith("Hash"):
        mname = meth[len("Hash"):]
        return {"linestyle": "solid",
                "color": meth_cmap[mname],
                "label": to_nmeth(mname)}
    if meth.startswith("FNVHash"):
        mname = meth[len("FNVHash"):]
        return {"linestyle": (0, (1,1)), #"densely dotted",
                "color": meth_cmap[mname],
                "label": to_fnv(mname)}
    if meth.startswith("Amnesia"):
        mname = meth[len("Amnesia"):]
        return {"linestyle": (0, (5,1)), #"densely dashed",
                "color": meth_cmap[mname],
                "label": to_amnesia(mname)}
    else:
        import sys
        print("UNRESOLVED STYLE FOR: " + meth)
        sys.exit(1)


def do_plot(*,container=None, xlabel=None, ylabel=None, title=None):
    if container:
        if not isinstance(container, DemoPlottingItems) and not isinstance(container, LargePlottingItems):
            print("PANIC")
            sys.exit(1)
        ref, leg = container.ax_ar[-1], container.legend_ax
        handles,labels = ref.get_legend_handles_labels()
        leg.legend(handles,labels, loc="center")
        for ax, fn in zip(container.ax_ar, container.fns):
            ax.set_xscale("log", base=2)
            if fn in BIN_DATA_FNS:
                ax.set_facecolor(GREY)
            ax.set_title(fn)
        fig = container.fig
        xlabel = xlabel if xlabel else "Number of Histograms"
        ylabel = ylabel if ylabel else "Compressed bits/Uncompressed Byte"
        fig.supxlabel(xlabel)
        fig.supylabel(ylabel)
        fig.suptitle(title)
        fig.tight_layout()
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
    df = pd.read_csv(csvpath)
    # Add b/B
    df = df.join((df["Entropy"]/df["FSize"]).to_frame(name="b/B"))
    return df

from dataclasses import dataclass
from typing import List
@dataclass
class DemoPlottingItems:
    ax_ar: List[mpl.axes.Axes]
    legend_ax: mpl.axes.Axes
    fig: mpl.figure.Figure
    fns: List[str]
@dataclass
class LargePlottingItems:
    ax_ar: List[mpl.axes.Axes]
    legend_ax: mpl.axes.Axes
    fig: mpl.figure.Figure
    fns: List
def get_demo_items(dataset):
    shape = (3, 2)
    fig, axs = plt.subplots(*shape, sharex='col')
    fig.set_size_inches(6, 7)
    ax_l = [(0,0),
            (1,0),
            (2,0),
            (0,1),
            (2,1)]
    ax_ar = [axs[idx] for idx in ax_l]
    #scapegoat axis for legend
    sg_ax = axs[1,1]
    sg_ax.clear()
    sg_ax.set_axis_off()
    if dataset == "calgary":
        wanted_fns = ["bib", "book1", "obj1", "pic", "progp"]
    elif dataset == "cantbry":
        wanted_fns = ["alice29.txt", "kennedy.xls", "sum", "fields.c", "xargs.1"]
    else:
        import sys
        print(f"{dataset} not found")
        sys.exit(1)
    return DemoPlottingItems(
        ax_ar=ax_ar,
        legend_ax=sg_ax,
        fig=fig,
        fns=wanted_fns
    )



def get_large_items(dataset):
    if dataset == "calgary":
        shape = (3,6)
        fig, axs = plt.subplots(*shape)
        plt.subplots_adjust(top=0.88, bottom=0.11, left=0.11,
                            right=0.9, hspace=0.2, wspace=0.2)
        fns = ["bib", "book1", "book2", "geo", "news", "obj1", "obj2",
               "paper1", "paper2", "paper3", "paper4", "paper5",
               "paper6", "pic", "progc", "progl", "progp", "trans"]
        legend_ax = axs[2,5]
    elif dataset == "cantbry":
        shape = (2,6)
        fig, axs = plt.subplots(*shape)
        legend_ax = axs[1,5]
        legend_ax.clear()
        legend_ax.set_axis_off()
        fns = ["alice29.txt", "asyoulik.txt", "cp.html", "fields.c",
               "grammar.lsp", "kennedy.xls", "lcet10.txt",
               "plrabn12.txt", "ptt5", "sum", "xargs.1",]
        coords = np.arange(np.prod(shape)).reshape(*shape)
        to_coords = lambda idx: (np.where(coords==idx)[0][0], np.where(coords==idx)[1][0])
    else:
        import sys
        print(f"{dataset} not found")
        sys.exit(1)
    dpi = fig.get_dpi()
    fig.set_size_inches(1920/dpi, 1200/dpi)
    coords = np.arange(np.prod(shape)).reshape(*shape)
    to_coords = lambda idx: (np.where(coords==idx)[0][0], np.where(coords==idx)[1][0])
    ax_ar = [axs[to_coords(idx)] for idx in range(len(fns))]
    return LargePlottingItems(
        ax_ar=ax_ar,
        fig=fig,
        fns=fns,
        legend_ax=legend_ax)
