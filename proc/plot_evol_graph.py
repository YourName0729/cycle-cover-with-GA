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
        def singleCmap(from_rgb):
            if isinstance(from_rgb, str):
                from_rgb = colors.to_rgba(from_rgb)
            print(from_rgb)
            r1,g1,b1, _ = from_rgb
            r2,g2,b2, _ = from_rgb

            cdict = {'red': ((0, r1, r1),
                        (1, r2, r2)),
                'green': ((0, g1, g1),
                            (1, g2, g2)),
                'blue': ((0, b1, b1),
                        (1, b2, b2))}

            cmap = colors.LinearSegmentedColormap('custom_cmap', cdict)
            return cmap
        # [print(p, d) for p, d in zip(self.pos, self.dat)]
        # print(color)
        circ = [plt.Circle((p[0], p[1]), 10 * d) for p, d in zip(self.pos, self.dat)]
        bd = [plt.Circle((p[0], p[1]), 10 * d, fill = False, linewidth=0.2) for p, d in zip(self.pos, self.dat)]
        ax.add_collection(mpl.collections.PatchCollection(circ, cmap=singleCmap(color), zorder=1))
        # ax.add_collection(mpl.collections.PatchCollection(bd, zorder=2))
        # [ax.add_patch(plt.Circle((p[0], p[1]), 10 * d, color=color)) for p, d in zip(self.pos, self.dat)]
        # [ax.add_patch(plt.Circle((p[0], p[1]), 10 * d, fill = False, linewidth=0.2)) for p, d in zip(self.pos, self.dat)]
        return self

    

def test_draw_graph():
    fin = '../data/es2/graph/2.txt'
    fout = '../figure/graph.png'

    g = instance()
    g.read(fin)
    fig, ax = plt.subplots()
    ax.set_xlim([0, 5000])
    ax.set_ylim([0, 5000])
    ax.set_aspect(1)
    g.draw(ax)
    
    plt.savefig(fout)

def read_graphs(path):
    flist = sorted([fname for fname in os.listdir(path)], key=lambda x: int(x.split('.')[0]))
    gs = [instance().read(path + fname) for fname in flist]
    # print(len(gs))
    return gs

def draw_diff(ax, g1, g2, color='b', linewidth=1):
    for p1, p2 in zip(g1.pos, g2.pos):
        ax.plot([p1[0], p2[0]], [p1[1], p2[1]], color=color, linewidth=linewidth, alpha=0.5)
        # print(p1, p2)

def main():
    path = '../data/es2/graph/'
    fout = '../figure/graph.png'
    gs = read_graphs(path)

    clrs = list(mpl.rcParams['axes.prop_cycle'])
    clrs = [np.array(colors.to_rgba(x['color'])) for x in clrs][:2]
    # cmid = (clrs[0] + clrs[1]) / 2

    fig, ax = plt.subplots()
    ax.set_aspect(1)
    for i in range(len(gs) - 1):
        # draw_diff(ax, gs[i], gs[i + 1], linewidth=30 * ((i + 1) / len(gs)))
        # draw_diff(ax, gs[i], gs[i + 1], linewidth=np.log(100 * ((i + 1) / len(gs)) + 1))
        draw_diff(ax, gs[i], gs[i + 1], color=(clrs[0] * i + clrs[1] * (len(gs) - i - 1)) / (len(gs) - 1), linewidth=np.log(100 * ((i + 1) / len(gs)) + 1))
    # gs[0].draw(ax, color=clrs[0])
    # gs[-1].draw(ax, color=clrs[1])
    plt.savefig(fout)

if __name__ == '__main__':
    main()