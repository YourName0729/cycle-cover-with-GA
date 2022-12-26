import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
import sys 
from functools import reduce





    

    

      


def get_df(dst):
    def arg2df(fpath):
        # print(fpath)
        columns = ['T','t','best_ratio','best_solv4_obj','best_GAs','best_X', 'best_Y', 'best_data', 'best_solv4_solution', 'best_GAs_solution']
        with open(fpath, 'r') as f:
            rows = []
            lines = f.readlines() 
            for i in range(len(lines)) :
                line = lines[i]
                ele = line.split(' ')
                row = []
                n = 0
                X = [];data = []
                Y = []
                best_GA = []
                best_solv4 = []
                for j in range(len(ele)) :
                
                    e = ele[j]
                    e = e.split('=')
                    if len(e) > 1 and e[0] != 'best_instance' :
                        row.append(e[1])
                    elif len(e) > 1 and e[0] == 'best_instance' :
                        n = int(e[1])
                        j += 1 

                        e = ele[j].split('=')[1]
                        X.append(e)
                        X += ele[j+1:j+n]
                        j += n
                        
                        e = ele[j].split('=')[1]
                        Y.append(e)
                        Y += ele[j+1:j+n]
                        j += n
           

                        e = ele[j].split('=')[1]
                        data.append(e)
                        data += ele[j+1:j+n]
                        j += n
                       
                        e = ele[j].split('=')[1]
                        for k in range(int(e)) :
                            i+=1 
                            ele = lines[i].split(' ')[:-1]
                            best_GA.append(ele)
                        i+=1 
                        e = lines[i].split('=')[1]
                        for k in range(int(e)) :
                            i+=1 
                            ele = lines[i].split(' ')[:-1]
                            best_solv4.append(ele)
                        
                        row += [X,Y,data,best_solv4,best_GA] 
                    
                        rows.append(row)
                        break 
            df = pd.DataFrame(rows, columns=columns)
 
         
            return df

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

    def plot_line(x,y,color=None) :
        plt.plot([x[0],y[0]],[x[1],y[1]], color=color, linestyle="-")
        
    def save_GA(dfs, fname, x = 'T' ):
        # print(avg_dfs)
   
        

        # print(names)
        for name,df in dfs.items():
            index = [0,len(df.index)-1]
            for i in index :
                plt.xlim(-50,5050)
                plt.ylim(-50,5050)
           
                X = [ float(x) for x in df['best_X'][i]]
                Y = [ float(y) for y in df['best_Y'][i]]
                data = [ (float(d)-5)*15 for d in df['best_data'][i]]

                plt.scatter(X,Y,s=data)

                for j in range(len(df['best_GAs_solution'][i])) :
                    tour = df['best_GAs_solution'][i][j]
                    for k in range(len(tour)-1) :
                        plot_line( (X[int(tour[k])],Y[int(tour[k])]), (X[int(tour[k+1])],Y[int(tour[k+1])]),color='red')
                    if len(tour) > 1 :
                     
                        plot_line( (X[int(tour[0])],Y[int(tour[0])]), (X[int(tour[-1])],Y[int(tour[-1])]),color='red')

            
                if x == 'T' :
                    plt.title('GA solution when T ='+ df['T'][i] )
                else : 
                    plt.title('GA solution when t ='+ df['t'][i])
             
                plt.xlabel('x-axis', fontsize=12)
                plt.ylabel('y-axis', fontsize=12)
                plt.savefig(fname+x+'='+df[x][i]+'.png')
                plt.clf()
                # plt.show()

    def save_solv4(dfs, fname, x = 'T' ):
        # print(avg_dfs)
   
        

        # print(names)
        for name,df in dfs.items():
  
            index = [0,len(df.index)-1]
            for i in index :
                plt.xlim(-50,5050)
                plt.ylim(-50,5050)
           
                X = [ float(x) for x in df['best_X'][i]]
                Y = [ float(y) for y in df['best_Y'][i]]
                data = [ (float(d)-5)*15 for d in df['best_data'][i]]

                plt.scatter(X,Y,s=data)
        

                for j in range(len(df['best_solv4_solution'][i])) :
                    tour = df['best_solv4_solution'][i][j]
                    for k in range(len(tour)-1) :
                        plot_line( (X[int(tour[k])],Y[int(tour[k])]), (X[int(tour[k+1])],Y[int(tour[k+1])]),color='blue')

       
                if x == 'T' :
                    plt.title('solv4 solution when T ='+ df['T'][i] )
                else : 
                    plt.title('solv4 solution when t ='+ df['t'][i])
         

                plt.xlabel('x-axis', fontsize=12)
                plt.ylabel('y-axis', fontsize=12)
                plt.savefig(fname+x+'='+df[x][i]+'.png')
                plt.clf()
        # plt.show()


    print("Reading and Parsing Data...")
    dfs = get_df(['data/min-max/es/solution/'])
   
    # saves
    print("Analyzing Data and Saving Figures...")
    pfx = 'figure/min-max/es/solution/'

    save_GA(dfs, pfx + 'GA_solution', 'T')
    save_solv4(dfs, pfx + 'solv4_solution', 't')


if __name__ == '__main__':

    main()
