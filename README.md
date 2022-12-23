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
./cycle_cover --constructor="name=es T=1000 demo=0 sigma=1 mu=1 lambda=1 save_graph=test.txt n=10 k=2 problem=min-max solver='name=elitism-ga m=100 T=1'"
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