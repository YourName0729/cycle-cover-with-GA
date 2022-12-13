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
./cycle_cover --constructor="name=es T=1 n=15 k=4 problem=min-max solver='name=ga m=10 T=2'"
```

Test min-deploy instance
```
make
./cycle_cover --constructor="name=min-deploy k=30 problem=min-max solver='name=ga m=10 T=2'"
```


Test mccp instance // set demo=1 to see the process of finding cycles 
``` 
make
./cycle_cover --constructor="name=mccp k=1800 n=500 problem=mccp solver='name=mccp demo=1'"
```