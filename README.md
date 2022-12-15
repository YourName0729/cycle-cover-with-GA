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
```

Test min-deploy instance
```
make
./cycle_cover --constructor="name=min-deploy k=30 problem=min-max solver='name=ga m=10 T=2'"
```


Test mccp instance // set demo=1 to see the process of finding cycles 
``` 
make
./cycle_cover --constructor="name=min-deploy k=1200 n=100 problem=mccp solver='name=mccp demo=0'"
```

Test mmccp instance 
``` 
make
./cycle_cover --constructor="name=min-deploy k=25 n=400 problem=min-max solver='name=min-max demo=0'"
```

Test gaip

```
make
./cycle_cover --constructor="name=min-deploy k=10 n=100 problem=min-max solver='name=gaip demo=1'"
```