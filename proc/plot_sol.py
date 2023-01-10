import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
import sys 
import matplotlib as mpl
from matplotlib import colors
from functools import reduce

class instance:
    def __init__(self) -> None:
        self.n = 0
        self.k = 0
        self.pos = []
        self.dat = []

    def read(self, fname):
        with open(fname, 'r') as f:
            self.n, self.k = list(map(int, f.readline().split()))
            self.pos = [
                list(map(float, f.readline().split())) for _ in range(self.n) 
            ]
            self.dat = list(map(float, f.readline().split()))
        return self
        # print(self.n, self.k)
        # print(self.pos)
        # print(self.dat)
    def draw(self, ax, color='k'):
        # ax.add_collection(mpl.collections.PatchCollection(bd, zorder=2))
        # [ax.add_patch(plt.Circle((p[0], p[1]), 10 * d, color=color)) for p, d in zip(self.pos, self.dat)]
        [ax.add_patch(plt.Circle((p[0] / 10, p[1] / 10), 2.5 * (d - 3), fill = False, linewidth=1.5)) for p, d in zip(self.pos, self.dat)]
        return self

class solution:
    def __init__(self) -> None:
        self.n = 0
        self.cycs = []

    def read(self, fname):
        with open(fname, 'r') as f:
            self.n = int(f.readline())
            self.cycs = [
                list(map(int, f.readline().split())) for _ in range(self.n)
            ]
        # print(self.cycs)
        return self

    def draw(self, ax, ins):
        for cyc in self.cycs:
            if len(cyc) == 0:
                continue
            if len(cyc) == 1:
                ax.add_patch(plt.Circle((ins.pos[cyc[0]][0] / 10, ins.pos[cyc[0]][1] / 10), 4.5))
            else:
                x = [ins.pos[v][0] for v in cyc] + [ins.pos[cyc[0]][0]]
                y = [ins.pos[v][1] for v in cyc] + [ins.pos[cyc[0]][1]]
                x = np.array(x) / 10
                y = np.array(y) / 10
                ax.plot(x, y, linewidth=2)

def draw_ins_sol(ax, ins, sol):
    ins.draw(ax)
    sol.draw(ax, ins)

def main():
    ins = instance().read('../data/es5/graph/202.txt')
    sol1 = solution().read('../data/sol3.txt')
    sol2 = solution().read('../data/sol4.txt')

    fig, ax = plt.subplots()
    ax.set_xlim([0, 500])
    ax.set_ylim([0, 500])
    ax.set_aspect(1)

    ins.draw(ax)
    plt.savefig('../figure/min-max/tabu2/ins1.png', bbox_inches = 'tight')

    # draw_ins_sol(ax, ins, sol1)
    # plt.savefig('../figure/min-max/tabu2/sol3.png', bbox_inches = 'tight')

    # draw_ins_sol(ax, ins, sol2)
    # plt.savefig('../figure/min-max/tabu2/sol4.png', bbox_inches = 'tight')

    # plt.show()

if __name__ == '__main__':
    main()