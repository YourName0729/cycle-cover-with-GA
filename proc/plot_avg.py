import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import matplotlib as mpl
import os
from functools import reduce

def arg2df(fpath):
    # print(fpath)
    with open(fpath, 'r') as f:
        lines = [line.split(' ')[:-1] for line in f.readlines()]
        columns = [ele.split('=')[0] for ele in lines[0]]
        rows = [[ele.split('=')[1] for ele in line] for line in lines]
        df = pd.DataFrame(rows, columns=columns)
        return df.apply(pd.to_numeric)

def unique(arr):
    re = []
    for e in arr:
        if e not in re:
            re.append(e)
    return sorted(re)

def main():
    pfx = '../data/comp/'
    fpaths = [pfx + 'tabu-ga min-max.txt', pfx + 'tagu-ga_min-max_2.txt']
    dest = '../figure/comp_avg.png'

    colors = list(mpl.rcParams['axes.prop_cycle'])
    colors = [x['color'] for x in colors]
    plt.rcParams['text.usetex'] = True

    dfs = [arg2df(fpaths[0]), arg2df(fpaths[1])]
    df = pd.concat(dfs)

    ns = unique(df['n'].to_numpy())
    obj1 = [df[df['n'] == n]['obj1'].sum() / np.count_nonzero((df['n'] == n).to_numpy()) for n in ns]
    obj2 = [df[df['n'] == n]['obj2'].sum() / np.count_nonzero((df['n'] == n).to_numpy()) for n in ns]

    fig, ax1 = plt.subplots()
    ax1.set_xlabel('$n$')
    ax1.plot(ns, -np.array(obj1), label='Fitness of GA')
    ax1.plot(ns, -np.array(obj2), label='Fitness of approAlgNoNei')
    ax1.set_ylabel('Fitness')
    ax1.legend(loc='upper center')

    ax2 = ax1.twinx()
    ax2.plot(ns, np.array(obj2) / np.array(obj1), color=colors[2])
    xv, yv = 110, 1
    ax2.plot([xv, xv], [0, yv], color=colors[2], linestyle='dashed')
    ax2.plot([xv, 500], [yv, yv], color=colors[2], linestyle='dashed')
    ax2.set_ylabel('Ratio', color = colors[2])
    ax2.tick_params(axis ='y', labelcolor = colors[2])

    plt.savefig(dest)

if __name__ == '__main__':
    main()