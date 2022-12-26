from pathlib import Path
import pickle
DATA_DIR = Path("../data/")
# CORPORA = [
#     "shakespeare",
#     "enwik8",
#     "cantrbry",
# ]

import string
SPACE_LOWERALPHA = ' ' + string.ascii_lowercase
def alphaspace_normalize(text):
    text = text.lower()
    as_set = set(SPACE_LOWERALPHA)
    text = "".join([t for t in text if t in as_set])
    return text

def get_shakespeare():
    shakespeare_path = DATA_DIR / "shakespeare/shakespeare"
    pkl_path = DATA_DIR / "shakespeare/normed.pkl"
    if not pkl_path.exists():
        with open(shakespeare_path) as f:
            text = f.read()
        text = alphaspace_normalize(text)
        with open(pkl_path, 'wb') as f:
            pickle.dump(text, f)
    with open(pkl_path, 'rb') as f:
        return pickle.load(f)

def byte_iter(byte_str):
    for byte in byte_str:
        yield [int(i) for i in f"{byte:08b}"]
def bit_iter(byte_str):
    for byte in byte_iter(byte_str):
        yield from byte
def get_sum():
    sum_path = DATA_DIR / "cantrbry/sum"
    with open(sum_path, 'rb') as f:
        return f.read()
def get_enwik8():
    enwik_path = DATA_DIR / "enwik8/enwik8"
    with open(enwik_path, 'rb') as f:
        return f.read()
