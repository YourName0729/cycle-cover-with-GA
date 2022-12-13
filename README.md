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