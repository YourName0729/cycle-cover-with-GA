# cycle-cover-with-GA
Solve a classic cycle cover problem with genetic algorithms

## Usage

Test dummy constructor and dummy solver

```
make
./cycle_cover --constructor="T=1000 solver='T=1000'"
```

Test GA
```
make
./cycle_cover --constructor="T=1 problem=min-max solver='name=ga m=10 T=2'"
./cycle_cover --constructor="min-deploy k=30 problem=min-max solver='name=tabu-ga m=10 T=2'"
```

Test ES

```
make
./cycle_cover --constructor="name=es T=100 demo=1 n=100 k=20 problem=min-max solver='name=elitism-ga m=100 T=200'"
./cycle_cover --constructor="name=es T=100 demo=1 n=100 k=20 problem=min-max solver='name=tour-ga m=100 tour_k=7 T=200'"
./cycle_cover --constructor="name=es T=100 demo=1 n=100 k=20 problem=min-max solver='name=trun-ga m=100 trun_k=0.5 T=200'"
```

Save ES 

```
Saving graph data :
./cycle_cover --constructor="name=es T=1000 demo=0 sigma=1 mu=1 lambda=1 repeat=5 save_graph=test.txt n=10 k=2 problem=min-max solver='name=elitism-ga m=100 T=1'"

Saving ins & sols 
./cycle_cover --constructor="name=es T=1000 demo=0 sigma=0.1 mu=1 lambda=1 save_solution=test.txt n=10 k=2 problem=min-max solver='name=elitism-ga m=100 T=1'"

Both
./cycle_cover --constructor="name=es T=1000 demo=0 k=30 sigma=1 mu=1 lambda=1 repeat=5 save_graph=test.txt save_solution=test.txt n=10 k=2 problem=min-max solver='m=100 T=10000 name=ss-ga selection=random crossover=edge_recomb mutation=swap'"
```

Test min-deploy instance
```
make
./cycle_cover --constructor="name=min-deploy k=30 problem=min-max solver='name=ga m=10 T=2'"
```


Test mccp instance // set demo=1 to see the process of finding cycles 
``` 
make
./cycle_cover --constructor="name=min-deploy B=1200 n=100 problem=mccp solver='name=mccp demo=0'"
```

Test mmccp instance 
``` 
make
./cycle_cover --constructor="name=min-deploy k=5 n=100 problem=min-max solver='name=min-max demo=0'"
```

Test Comparison
```
./cycle_cover --constructor="name=comparison n=25 k=5 sigma=0.2 save_graph=data/es3/graph/ save=data/es3/es.txt"
```

Test ES
```
make
./cycle_cover --constructor="name=es2 demo=1"
```

## GA parameters

- name
    - `elitism-ga`
    - `standard-ga`
    - `ss-ga`
- selection
    - `dummy`
    - `random`
    - `elitism`
    - `roulette_wheel`
    - `tournament`
    - `elitism-ga` has no selection (default `elitism`)
- replacement
    - `dummy`
    - `random`
    - `elitism`
    - `roulette_wheel`
    - `tournament`
    - `elitism-ga` has no replacement
    - `fast-ga` has no replacement
- crossover
    - `dummy`
    - `pmx`
    - `ox`
    - `cycle`
    - `edge_recomb`
- mutation
    - `dummy`
    - `insert`
    - `swap`
    - `inverse`
    - `scramble`

### Example

```
./cycle_cover --result --constructor="name=min-deploy k=2 n=10 problem=min-max solver='name=elitism-ga crossover=cycle mutation=swap m=10 T=100'"
./cycle_cover --result --constructor="name=min-deploy k=2 n=10 problem=min-max solver='name=standard-ga selection=roulette_wheel replacement=elitism crossover=cycle mutation=inverse m=10 T=100'"
./cycle_cover --result --constructor="name=min-deploy k=2 n=10 problem=min-max solver='name=fast-ga selection=random crossover=cycle mutation=inverse m=10 T=100'"
./cycle_cover  --constructor="name=min-deploy k=30 n=500 problem=min-max solver='name=tabu-ga selection=elitism crossover=edge_recomb mutation=swap perent_ratio=1 mutation_rate=0.2 m=100 T=30000 save=data/tabu_ga3.txt block=50'"
./cycle_cover  --constructor="name=min-deploy k=30 n=500 problem=min-max solver='name=tabu-ga selection=roulette_wheel replacement=elitism crossover=edge_recomb mutation=swap parent_ratio=0.8 mutation_rate=0.2 m=100 T=50000 save=data/tabu_ga3.txt block=50'"
```

## Grid Search

- search all solvers to find the best one

```
name: standard-ga
selection: random elitism roulette_wheel tournament
replacement: elitism roulette_wheel tournament

name: ss-ga
selection: random elitism roulette_wheel tournament
no replacement

crossover: pmx ox cycle edge_recomb
mutation: insert swap invert scramble
```

```
./cycle_cover --constructor="name=grid-search k=30 demo=1 thread_size=14"
./cycle_cover --constructor="name=grid-search problem=min-max-dis dir=data/min-max-dis k=30 demo=1 thread_size=14"
```

### Result

Top 5 solver

```
m=100 T=30000 name=ss-ga selection=random crossover=edge_recomb mutation=swap
m=100 T=30000 name=ss-ga selection=elitism crossover=edge_recomb mutation=insert
m=100 T=30000 name=ss-ga selection=roulette_wheel crossover=edge_recomb mutation=swap
m=100 T=30000 name=ss-ga selection=random crossover=edge_recomb mutation=insert
m=100 T=30000 name=standard-ga selection=random crossover=ox mutation=swap replacement=elitism
```