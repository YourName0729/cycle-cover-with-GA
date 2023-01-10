import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
from functools import reduce

def get_df(dst):
    def arg2df(fpath):
        # print(fpath)
        with open(fpath, 'r') as f:
            lines = [line[:-1].split(' ') for line in f.readlines()]
            columns = [ele.split('=')[0] for ele in lines[0]]
            rows = [[ele.split('=')[1] for ele in line] for line in lines]
            df = pd.DataFrame(rows, columns=columns)
            return df.apply(pd.to_numeric)

    def purify(fpath):
        ga = fpath.split('/')[-2]
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
        
    flist = [path + fname for path in dst for fname in os.listdir(path)]

    # v to comment
    # np.random.shuffle(flist)
    # flist = flist[:10]
    # ^ to comment

    dfs = {purify(fpath):arg2df(fpath) for fpath in flist}
    return dfs

def main():
    def avg_instance(dfs):
        re = {}
        for fname, df in dfs.items():
            fname_ni = fname[:fname.rfind(' ')]
            if fname_ni in dfs:
                continue
            inss = [f'{fname_ni} {i}' for i in range(10) if f'{fname_ni} {i}' in dfs]
            f_dfs = [dfs[name] for name in inss]
            df = reduce(lambda a, b: a.add(b, fill_value=0), f_dfs)
            # df = df.apply(pd.to_numeric)
            df = df.div(len(inss))
            re[fname_ni] = df
        return re
    
    def save_avg_by_index(dfs, idx, fname, x='T'):
        re = {}
        for name, df in dfs.items():
            col = name.split()[idx]
            if col not in re:
                re[col] = []
            re[col].append(df)
        for col, df_list in re.items():
            l = len(df_list)
            re[col] = reduce(lambda a, b: a.add(b, fill_value=0), df_list).div(l)
        items = re.items()
        ord = {'elitism': 'a', 'tournament': 'aa', 'roulette_wheel': 'aaa'}
        items = sorted(items, key=lambda x: ord[x[0]] if x[0] in ord else x[0])
        for col, df in items:
            if x == 'T':
                plt.plot(df[x], df['best'], label=col)
            else:
                plt.plot(df[x] / 1000.0, df['best'], label=col)
                # plt.plot(df[x], df['best'], label=col)
        xlabel = ''
        if x == 'T':
            xlabel = 'Generation'
        else:
            xlabel = 'Time(s)'
        plt.xlabel(xlabel)
        plt.ylabel('Fitness')
        columns = ['GA types', 'Selections', 'Replacements', 'Crossovers', 'Mutations']
        plt.title(f'Comparison of {columns[idx]}')
        plt.legend()
        # plt.show()
        plt.savefig(fname)
        plt.clf()
        # plt.show()
        
    def save_bests(avg_dfs, fname, x = 'T', k = 1):
        # print(avg_dfs)
        names = avg_dfs.keys()
        names = sorted(names, key=lambda x : -avg_dfs[x]['best'].iloc[-1])
        names = names[:k]
        # print(names)
        for name in names:
            if x == 'T':
                plt.plot(avg_dfs[name][x], avg_dfs[name]['best'], label=name)
            else:
                plt.plot(avg_dfs[name][x] / 1000.0, avg_dfs[name]['best'], label=name)
            # plt.plot(avg_dfs[name][x], avg_dfs[name]['best'], label=name)
        xlabel = ''
        if x == 'T':
            xlabel = 'Generation'
        else:
            xlabel = 'Time(s)'
        plt.xlabel(xlabel)
        plt.ylabel('Fitness')
        plt.title(f'Best GAs over {xlabel}')
        plt.legend()
        plt.savefig(fname)
        plt.clf()
        # plt.show()

    print("Reading and Parsing Data...")
    # dfs = get_df(['../data/min-max/ss/', '../data/min-max/standard/'])
    dfs = get_df(['../data/min-max/tabu2/'])
    avg_dfs = avg_instance(dfs)

    # saves
    print("Analyzing Data and Saving Figures...")
    pfx = '../figure/min-max/tabu2/'

    save_bests(avg_dfs, pfx + 'best_T.png', 'T', 5)
    save_bests(avg_dfs, pfx + 'best_t.png', 't', 5)
    # save_avg_by_index(avg_dfs, 0, pfx + 'ga_t.png', 't')
    # save_avg_by_index(avg_dfs, 0, pfx + 'ga_T.png', 'T')
    save_avg_by_index(avg_dfs, 1, pfx + 'selection_t.png', 't')
    save_avg_by_index(avg_dfs, 1, pfx + 'selection_T.png', 'T')
    save_avg_by_index(avg_dfs, 2, pfx + 'replacement_t.png', 't')
    save_avg_by_index(avg_dfs, 2, pfx + 'replacement_T.png', 'T')
    save_avg_by_index(avg_dfs, 3, pfx + 'crossover_t.png', 't')
    save_avg_by_index(avg_dfs, 3, pfx + 'crossover_T.png', 'T')
    save_avg_by_index(avg_dfs, 4, pfx + 'mutation_t.png', 't')
    save_avg_by_index(avg_dfs, 4, pfx + 'mutation_T.png', 'T')

def change_name():
    dst = ['../data/min-max/ss/', '../data/min-max/standard/']
    for path in dst:
        for fname in os.listdir(path):
            if 'invert' in fname:
                os.rename(path + fname, path + fname.replace('invert', 'inverse'))

if __name__ == '__main__':
    main()
    # change_name()
    # unite_dataframe('../data/ss/')
    # arg2df('../data/ss/elitism_cycle_insert_0.txt')