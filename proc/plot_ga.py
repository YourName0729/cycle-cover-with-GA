import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
from functools import reduce

def get_df():
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
        
    dst = ['../data/ss/', '../data/standard/']
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
        for col, df in re.items():
            plt.plot(df[x], df['best'], label=col)
        xlabel = ''
        if x == 'T':
            xlabel = 'Generation'
        else:
            xlabel = 'Time(ms)'
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
            plt.plot(avg_dfs[name][x], avg_dfs[name]['best'], label=name)
        xlabel = ''
        if x == 'T':
            xlabel = 'Generation'
        else:
            xlabel = 'Time(ms)'
        plt.xlabel(xlabel)
        plt.ylabel('Fitness')
        plt.title(f'Best GAs over {xlabel}')
        plt.legend()
        plt.savefig(fname)
        plt.clf()
        # plt.show()

    print("Reading and Parsing Data...")
    dfs = get_df()
    avg_dfs = avg_instance(dfs)

    # saves
    print("Analyzing Data and Saving Figures...")
    pfx = '../figure/'

    save_bests(avg_dfs, pfx + 'best_T.png', 'T', 5)
    save_bests(avg_dfs, pfx + 'best_t.png', 't', 5)
    save_avg_by_index(avg_dfs, 0, pfx + 'ga_t.png', 't')
    save_avg_by_index(avg_dfs, 0, pfx + 'ga_T.png', 'T')
    save_avg_by_index(avg_dfs, 1, pfx + 'selection_t.png', 't')
    save_avg_by_index(avg_dfs, 1, pfx + 'selection_T.png', 'T')
    save_avg_by_index(avg_dfs, 2, pfx + 'replacement_t.png', 't')
    save_avg_by_index(avg_dfs, 2, pfx + 'replacement_T.png', 'T')
    save_avg_by_index(avg_dfs, 3, pfx + 'crossover_t.png', 't')
    save_avg_by_index(avg_dfs, 3, pfx + 'crossover_T.png', 'T')
    save_avg_by_index(avg_dfs, 4, pfx + 'mutation_t.png', 't')
    save_avg_by_index(avg_dfs, 4, pfx + 'mutation_T.png', 'T')

if __name__ == '__main__':
    main()
    # unite_dataframe('../data/ss/')
    # arg2df('../data/ss/elitism_cycle_insert_0.txt')