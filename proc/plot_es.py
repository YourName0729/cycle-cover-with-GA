import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
import sys 
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

    
        
    def save_bests(dfs, fname, x = 'T' ):
        # print(avg_dfs)
   
    

        # print(names)
        for name,df in dfs.items():
            if x == 'T':
                plt.plot(df['T'], df['best_ratio'], label=name)
            else:
                plt.plot(df['t']/ 1000.0, df['best_ratio'], label=name)
            # plt.plot(avg_dfs[name][x], avg_dfs[name]['best'], label=name)
        xlabel = ''
        if x == 'T':
            xlabel = 'Generation'
        else:
            xlabel = 'Time(s)'
        plt.xlabel(xlabel)
        plt.ylabel('Objective')
        plt.title(f'Compare of different step size')
        plt.legend()
        plt.savefig(fname)
        plt.clf()
        # plt.show()

    print("Reading and Parsing Data...")
    dfs = get_df(['data/min-max/es/'])

    # saves
    print("Analyzing Data and Saving Figures...")
    pfx = 'figure/min-max/es/'

    save_bests(dfs, pfx + 'step_size_T.png', 'T')
    save_bests(dfs, pfx + 'step_size_t.png', 't')


if __name__ == '__main__':

    main()
    # change_name()
    # unite_dataframe('../data/ss/')
    # arg2df('../data/ss/elitism_cycle_insert_0.txt')