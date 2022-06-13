
from tqdm import tqdm
def entropy_under_model(model_ctor, bits):
    model = model_ctor()
    entropy = 0
    for cursor, bit in tqdm(enumerate(bits), total=len(bits)):
        try:
            ic = -math.log2(model.probs()[bit])
        except:
            breakpoint()
        model.learn(bit)
        entropy += ic
    return entropy, model
models = [
    (UniformModel, "UniformModel"),
    (Order0Laplace, "Order0Laplace"),
    (Order0KT, "Order0KT"),
    (SuffixTree, "SuffixTree")
]
def orderNfact(i):
    return (lambda: OrderNLaplace(i), f"Order{i}Laplace")
# for i in range(1, 10):
#     models.append(orderNfact(i))
# # debug = False
# for model, name in models:
#     e, m = entropy_under_model(model, bits)
#     print(f"{e:.2f} <- {name}")
