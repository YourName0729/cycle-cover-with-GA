import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd
import numpy as np

def main():
    colors = list(mpl.rcParams['axes.prop_cycle'])
    colors = [x['color'] for x in colors]
    clr = colors[0]

    x = np.linspace(0, 1, 100)
    r = 1000
    y = (np.power(r, x) - 1) / (r - 1)

    x2 = np.linspace(1, 1.1, 10)
    y2 = np.ones(len(x2))
    plt.plot(x, y, color=clr)
    plt.plot(x2, y2, color=clr)
    plt.xlabel('Fitness Ratio')
    plt.ylabel('Replace Prabability')
    plt.savefig('../figure/wams.png')

if __name__ == '__main__':
    main()