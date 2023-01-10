import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
import sys 
import matplotlib as mpl
from matplotlib import colors
from functools import reduce


def get_dfs(dst):
    def arg2df(fpath):
        # print(fpath)
        with open(fpath, 'r') as f:
            lines = [line[:-1].split(' ') for line in f.readlines()]
            # print(lines)
            columns = [ele.split('=')[0] for ele in lines[0]]
            # print(columns)
            # print(lines[0])
            # print(lines)
            # [print(ele) if len(ele.split('=')) < 2 else 0 for line in lines for ele in line ]
            rows = [[ele.split('=')[1] for ele in line] for line in lines]
            df = pd.DataFrame(rows, columns=columns)
            return df.apply(pd.to_numeric)

    flist = [fpath + 'es.txt' for fpath in dst]

    dfs = {int(fpath.split('/')[-2][-1]) - 3:arg2df(fpath) for fpath in flist}
    return dfs

def sigma():
    pfx = '../data/'
    fpaths = [pfx + 'es3/', pfx + 'es4/', pfx + 'es5/', pfx + 'es6/', pfx + 'es7/', pfx + 'es8/', pfx + 'es9/']
    dfs = get_dfs(fpaths)
    print(dfs)

    plt.rcParams['text.usetex'] = True

    sig = [0.4, 0.2, 0.1, 0.05, 0.025, 0.0125, 0.00625]
    # sig = [0.00625, 0.0125, 0.025, 0.05, 0.1, 0.2, 0.4]
    idx = [4, 3, 2, 1, 0, 5, 6]
    x = dfs[0]['T'].to_numpy()

    fig, ax1 = plt.subplots()
    for i, v in enumerate(idx):
        ax1.plot(x, dfs[v]['sigma'], label='inital $\sigma=' + str(sig[i]) + '$')
        print(i, sig[i], dfs[v]['sigma'][0])
    ax1.legend()
    ax1.set_xlabel('Generation')
    ax1.set_ylabel('$\sigma$')
    plt.savefig('../figure/min-max/tabu2/sigma.png')
    # plt.show()
    # print(x)

def ratio():
    pfx = '../data/'
    fpaths = [pfx + 'es3/', pfx + 'es4/', pfx + 'es5/', pfx + 'es6/', pfx + 'es7/', pfx + 'es8/', pfx + 'es9/']
    dfs = get_dfs(fpaths)
    print(dfs)

    plt.rcParams['text.usetex'] = True

    sig = [0.4, 0.2, 0.1, 0.05, 0.025, 0.0125, 0.00625]
    idx = [4, 3, 2, 1, 0, 5, 6]
    # idx = [2]

    x = dfs[0]['T'].to_numpy()
    fig, ax1 = plt.subplots()
    for i, v in enumerate(idx):
        ax1.plot(x, dfs[v]['obj1'] / dfs[v]['obj2'], label='inital $\sigma=' + str(sig[i]) + '$')
    ax1.legend()
    ax1.set_xlabel('Generation')
    ax1.set_ylabel('Ratio')
    plt.savefig('../figure/min-max/tabu2/ratio_all.png')

def main():
    # sigma()
    ratio()

if __name__ == '__main__':
    main()