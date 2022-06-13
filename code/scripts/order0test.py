import sys
sys.path.append("../")
from coding.ans.StreamingANS import StreamingANS
from coding.ac.StreamingAC import StreamingAC
from models.order0 import LinearOrder0Model, LogOrder0Model, UniformModel, Order0KT
from corpus import corpus
from tqdm import tqdm

def ans_encode(ans, model, string):
    for s in string:
        model.learn(s)
    for s in reversed(string):
        model.unlearn(s)
        ans.encode(s, model)

def ac_encode(ac, model, string):
    for s in tqdm(string):
        ac.encode(s, model)
        model.learn(s)
    ac.finish_encode()

def general_decode(coder, model, EOF):
    out_buf = []
    sym = ""
    while sym != EOF:
        sym = coder.decode(model)
        model.learn(sym)
        out_buf.append(sym)
    return "".join(out_buf)
def ac_correctness(model_ctor):
    precision = 64
    EOF = '\0'
    alphabet =  corpus.SPACE_LOWERALPHA + EOF
    model = model_ctor(syms=alphabet)
    ac = StreamingAC(precision)
    in_str = corpus.get_shakespeare()[:10000] + EOF
    ac_encode(ac, model, in_str)
    # Clear model
    model = model_ctor(syms=alphabet)
    ac = StreamingAC(precision, compressed=ac.get_compressed())
    out_str = general_decode(ac, model, EOF)
    assert out_str == in_str
def model_correctness_test(model_ctor):
    precision = 32
    EOF = '\0'
    alphabet =  corpus.SPACE_LOWERALPHA + EOF
    model = model_ctor(alphabet)
    ans = StreamingANS(precision)
    in_str = corpus.get_shakespeare()[:10000] + EOF
    ans_encode(ans, model, in_str)
    # Clear model
    model = model_ctor(alphabet)
    out_str = general_decode(ans, model, EOF)
    assert out_str == in_str
def model_stats(model_ctor):
    precision = 32
    EOF = '\0'
    alphabet =  corpus.SPACE_LOWERALPHA + EOF
    model = model_ctor(alphabet)
    ans = StreamingANS(precision)
    in_str = corpus.get_shakespeare() + EOF
    ans_encode(ans, model, in_str)
    def encstate_size_in_bits(ar):
        nonlocal precision
        return len(ar)*precision
    name = model_ctor.__name__
    print(f"{name}: {encstate_size_in_bits(ans.get_compressed())/len(in_str)}")
import math
def entropy_stats(model_ctor):
    alphabet =  corpus.SPACE_LOWERALPHA
    model = model_ctor(syms=alphabet)
    in_str = corpus.get_shakespeare()[:10000]
    entropy = 0
    for char in tqdm(in_str):
        ic = -math.log2(model.pmf(char))
        model.learn(char)
        entropy += ic
    print(f"{model_ctor.__name__}: {entropy}")


from multiprocessing import Pool, cpu_count
correctness_models = [
    LinearOrder0Model,
    LogOrder0Model,
    UniformModel,
]
from models.ctw import CTW
stats_models = [
    UniformModel,
    LinearOrder0Model,
    Order0KT,
#    CTW,
]

if __name__ == "__main__":
    pass
    #ac_correctness(CTW)
    # entropy_stats(CTW)
    # with Pool(cpu_count()) as p:
    #     p.map(model_correctness_test, correctness_models)
    #     p.map(ac_correctness, correctness_models)
    # for model in stats_models:
    #     entropy_stats(model)

# def time_ans(model_ctor):
#     pass
# with TimeSection("Linear bkwd") as lin_bkwd:
#     lin_work_times = []
#     for s in reversed(in_str):
#         with TimeSection() as wt:
#             ans.encode(s, model2)
#         lin_work_times.append(wt.elapsed())
# mu = lambda li: sum(li)/len(li)
# print(f"Linear learn {mu(lin_learn_times)} unlearn {mu(lin_unlearn_times)} work {mu(lin_work_times)}")
# with TimeSection("Log fwd") as log_fwd:
#     log_learn_times = []
#     for s in in_str:
#         with TimeSection() as lt:
#             model1.learn(s)
#         log_learn_times.append(lt.elapsed())
# with TimeSection("Log bkwd") as log_bkwd:
#     log_unlearn_times = []
#     log_work_times = []
#     for s in reversed(in_str):
#         with TimeSection() as lt:
#             model1.unlearn(s)
#         log_unlearn_times.append(lt.elapsed())
#         with TimeSection() as wt:
#             model1.scaled_pmf(EOF, precision)
#             model1.scaled_exclusive_cmf(EOF, precision)
#         log_work_times.append(wt.elapsed())
# print(f"Log learn {mu(log_learn_times)} unlearn {mu(log_unlearn_times)} work {mu(log_work_times)}")
