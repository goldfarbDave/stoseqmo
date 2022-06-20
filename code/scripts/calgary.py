import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df = pd.read_csv("../harness/calgary.csv")
# Add b/B
df = df.join((df["Entropy"]/df["FSize"]).to_frame(name="b/B"))

# Top 3 for each file
for fn in np.unique(df["File"]):
    print(fn)
    ndf = df[df["File"] == fn]
    print(ndf.sort_values(by='b/B'))
    #print(ndf.sort_values(by='b/B').head(2))
