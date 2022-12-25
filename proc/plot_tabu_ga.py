import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd
import numpy as np
import os
from functools import reduce
from scipy import optimize
from decimal import Decimal

def get_df(fpath):
    def arg2df(fpath):
        # print(fpath)
        with open(fpath, 'r') as f:
            lines = [line[:-1].split(' ') for line in f.readlines()]
            columns = [ele.split('=')[0] for ele in lines[0]]
            rows = [[ele.split('=')[1] for ele in line] for line in lines]
            df = pd.DataFrame(rows, columns=columns)
            return df.apply(pd.to_numeric)

    def purify(fpath):
        ga = fpath.split('/')[-1]
        fname = fpath[fpath.rfind('/') + 1:].split('_')
        fname[-1] = fname[-1].split('.')[0]

        while 'edge' in fname:
            idx = fname.index('edge')
            fname[idx] = 'edge_recomb'
            del fname[idx + 1]
        while 'roulette' in fname:
            idx = fname.index('roulette')
            fname[idx] = 'roulette_wheel'
            del fname[idx + 1]

        if ga == 'ss':
            fname.insert(1, 'x')
        pur = ' '.join([ga] + fname)
        # print(pur)
        return pur

    # v to comment
    # np.random.shuffle(flist)
    # flist = flist[:10]
    # ^ to comment

    return fpath.split('/')[-1].split('.')[0].replace('_', ' ').replace('ga', ''), arg2df(fpath)

def diversity(df, fname = '../figure/tabu/test.png'):
    x = df['T']
    colors = list(mpl.rcParams['axes.prop_cycle'])
    colors = [x['color'] for x in colors]
    # print(colors)

    col_y = 'diversity'
    # col_2y = 'hit'
    col_x = 'T'

    # plot col_y
    fig, ax1 = plt.subplots()
    y = df[col_y].to_numpy() / 50 / 500
    y[0] = np.average(y[1:])
    ax1.set_xlabel(col_x)
    ax1.set_ylabel('Diversity')
    ax1.plot(x, y)

    # plot col_2y
    # y = df[col_2y] / (50 * 60) * 100
    # ax2 = ax1.twinx()
    # ax2.set_ylabel('Tabu Hit Rate(%)', color = colors[1])
    # ax2.plot(x, y, color=colors[1])
    # ax2.tick_params(axis ='y', labelcolor = colors[1])

    plt.savefig(fname)

def best_max_min_avg(df, fname):
    x = df['T']
    colors = list(mpl.rcParams['axes.prop_cycle'])
    colors = [x['color'] for x in colors]

    # plot col_y
    fig, ax1 = plt.subplots()
    ax1.set_xlabel('Generation')
    ax1.set_ylabel('Fitness')

    col_ys = ['best', 'mx', 'avg', 'mn']
    clr_idxs = [0, 1, 2, 3]
    lbs = ['Best', 'Max', 'Avg', 'Min']
    for col, clr_idx, lb in zip(col_ys, clr_idxs, lbs):
        # print(col, clr_idx, lb)
        y = df[col].to_numpy()
        y[0] = y[1]
        # print(y)
        ax1.plot(x, y, color=colors[clr_idx], label=lb)
    ax1.legend()
    
    # plot col_2y
    # col_2y = 'hit'
    # y = df[col_2y] / (50 * 60) * 100
    # ax2 = ax1.twinx()
    # ax2.set_ylabel('Tabu Hit Rate(%)', color = colors[1])
    # ax2.plot(x, y, color=colors[1])
    # ax2.tick_params(axis ='y', labelcolor = colors[1])

    plt.savefig(fname)

def wams_miss(df, fname):
    x = df['T']
    colors = list(mpl.rcParams['axes.prop_cycle'])
    colors = [x['color'] for x in colors]

     # plot col_y
    col_y = 'wams_miss'
    fig, ax1 = plt.subplots()
    y = df[col_y] / (50 * 60) * 100
    ax1.set_xlabel('Generation')
    ax1.set_ylabel('WAMS Miss Rate(%)')
    ax1.plot(x, y)

    # plot col_2y
    # col_2y = 'hit'
    # y = df[col_2y] / (50 * 60) * 100
    # ax2 = ax1.twinx()
    # ax2.set_ylabel('Tabu Hit Rate(%)', color = colors[1])
    # ax2.plot(x, y, color=colors[1])
    # ax2.tick_params(axis ='y', labelcolor = colors[1])

    plt.savefig(fname)

def tabu_hit(df, fname):
    x = df['T']
    colors = list(mpl.rcParams['axes.prop_cycle'])
    colors = [x['color'] for x in colors]

     # plot col_y
    col_y = 'hit'
    fig, ax1 = plt.subplots()
    y = df[col_y] / (50 * 60) * 100
    ax1.set_xlabel('Generation')
    ax1.set_ylabel('Tabu Hit Rate(%)')
    ax1.plot(x, y)

    plt.savefig(fname)

def fitted(x, y):
    log = optimize.curve_fit(lambda t, a, b, c, d, e: a + t * (b + t * (c + t * (d + e * t))), x, y)
    para = np.array(log[0])
    # y_log = para[1] * np.log(x + 1e-6) + para[0]
    a, b, c, d, e = para
    fit = a + x * (b + x * (c + x * (d + x * e)))
    return fit

def fitted_log(x, y):
    log = optimize.curve_fit(lambda t, a, b: a + b * np.log(t), x, y)
    para = np.array(log[0])
    # y_log = para[1] * np.log(x + 1e-6) + para[0]
    a, b = para
    fit =  a + b * np.log(x)
    return fit

def fitted_order(y, vis=100):
    y2 = np.zeros(len(y))
    for i in range(len(y2)):
        ws = 50
        win = y[max(0, i - ws):min(len(y) - 1, i + ws)]
        mx, mn = np.max(win), np.min(win)
        y2[i] = (y[i] - mn) / (mx - mn + 1e-6)
        y2[i] = np.power(vis, y2[i]) / vis * 100
    return y2

def progress(df, fname):
    x = df['T']
    colors = list(mpl.rcParams['axes.prop_cycle'])
    colors = [x['color'] for x in colors]

     # plot col_y
    fig, ax1 = plt.subplots()   
    ax1.set_xlabel('Generation')
    ax1.set_ylabel('Exploitation (%)')
    # ax1.plot(x[:-1], fitted_order(y))

    def plot(ax, x, y, lb, vis=100):
        st = int(10000 / 50)
        ed = int(27000 / 50)
        ny = fitted_order(y, vis)[st: ed]
        print(np.average(ny[ny > 50]))
        ax.plot(x[st:ed], ny, label=lb)

    st = int(10000 / 50)
    ed = int(27000 / 50)

    ys = []
    lbs = ['Best', 'Tabu Hit', 'WAMS Miss', 'Difference', 'Diversity']

    y = df['best']
    y = [((y[i + 1] - y[i]) / -y[i] * 100) for i in range(len(y) - 1)]
    ys.append(np.array(fitted_order(y, 10000)[st: ed]))
    # plot(ax1, x, y, 'Best', 10000)

    y = df['hit'].to_numpy() / (50 * 60) * 100
    ys.append(np.array(fitted_order(y, 10000)[st: ed]))
    # plot(ax1, x, y, 'Tabu Hit', 10000)

    y = df['wams_miss'].to_numpy() / (50 * 60) * 100
    ys.append(np.array(fitted_order(y, 50)[st: ed]))
    # plot(ax1, x, y, 'WAMS Miss', 50)

    y = 1 / (df['mx'] - df['mn'])
    ys.append(np.array(fitted_order(y, 10000)[st: ed]))
    # plot(ax1, x, y, 'Difference', 10000)

    y = -df['diversity']
    ys.append(np.array(fitted_order(y, 250)[st: ed]))
    # plot(ax1, x, y, 'Diversity', 250)

    mat = np.zeros((len(ys), len(ys)))
    for i in range(len(ys)):
        for j in range(len(ys)):
            if i == j:
                continue
            # mat[i, j] = np.inner(ys[i], ys[j]) / np.linalg.norm(ys[i]) / np.linalg.norm(ys[j])
            # mat[i, j] = np.log(mat[i, j] + 1)
            mat[i, j] = np.corrcoef(ys[i], ys[j])[0][1]

    im = ax1.imshow(mat)
    ax1.set_xticks(np.arange(len(lbs)), labels=lbs)
    ax1.set_yticks(np.arange(len(lbs)), labels=lbs)

    for i in range(len(lbs)):
        for j in range(len(lbs)):
            if i == j:
                continue
            # text = ax1.text(j, i, mat[i, j], ha="center", va="center", color="w")
            text = ax1.text(j, i, "{:.2E}".format(Decimal(str(mat[i, j]))), ha="center", va="center", color="r")

    # ax1.legend()

    # col_2y = 'hit'
    # y = df[col_2y] / (50 * 60) * 100
    # ax2 = ax1.twinx()
    # ax2.set_ylabel('Tabu Hit Rate(%)', color = colors[1])
    # ax2.plot(x, y, color=colors[1])
    # ax2.tick_params(axis ='y', labelcolor = colors[1])

    # col_2y = 'wams_miss'
    # y = df[col_2y] / (50 * 60) * 100
    # ax2 = ax1.twinx()
    # ax2.set_ylabel('WAMS Miss Rate(%)', color = colors[1])
    # ax2.plot(x, y, color=colors[1])
    # ax2.tick_params(axis ='y', labelcolor = colors[1])

    # col_2y = 'hit'
    # ax2 = ax1.twinx()
    # y = df[col_2y].to_numpy() / (50 * 60) * 100
    # ax2.plot(x, fitted_order(y), color=colors[1])

    # col_2y = 'wams_miss'
    # y = df[col_2y].to_numpy() / (50 * 60) * 100
    # ax2.plot(x, fitted_order(y), color=colors[2])

    # ax2.tick_params(axis ='y', labelcolor = colors[1])

    plt.savefig(fname)

def wams_miss_adj(df, fname):
    x = df['T'].to_numpy()
    colors = list(mpl.rcParams['axes.prop_cycle'])
    colors = [x['color'] for x in colors]

     # plot col_y
    col_y = 'wams_miss'
    fig, ax1 = plt.subplots()
    y = df[col_y].to_numpy() / (50 * 60) * 100
    log = optimize.curve_fit(lambda t, a, b, c, d, e: a + t * (b + t * (c + t * (d + e * t))), x, y)
    para = np.array(log[0])
    # y_log = para[1] * np.log(x + 1e-6) + para[0]
    a, b, c, d, e = para
    fit = a + x * (b + x * (c + x * (d + x * e)))
    ax1.plot(x, y - fit, color=colors[2])

    ax1.set_xlabel('Generation')
    ax1.set_ylabel('WAMS Miss Rate(%)', color=colors[0])
    # ax1.plot(x, y, color=colors[0])

    # plot col_2y
    col_2y = 'hit'
    y = df[col_2y] / (50 * 60) * 100
    ax2 = ax1.twinx()
    ax2.set_ylabel('Tabu Hit Rate(%)', color = colors[1])
    ax2.plot(x, y, color=colors[1])
    ax2.tick_params(axis ='y', labelcolor = colors[1])

    plt.savefig(fname)

def main():
    fpath = '../data/tabu_ga.txt'
    name, df = get_df(fpath)
    # print(name)
    print(df)
    # plot2y(df, 'best')
    # plot2y(df, 'wams_miss')
    pfx = '../figure/tabu/'
    # diversity(df, pfx + 'diversity.png')
    best_max_min_avg(df, pfx + 'bmma.png')
    # wams_miss(df, pfx + 'wams_miss.png')
    # tabu_hit(df, pfx + 'tabu_hit.png')
    # progress(df, pfx + 'progress.png')

    # wams_miss_adj(df, pfx + "wams_miss_adj.png")


if __name__ == '__main__':
    main()