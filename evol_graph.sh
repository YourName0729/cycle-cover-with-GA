# ./cycle_cover --constructor="name=es2 n=25 k=5 sigma=0.025 save_graph=data/es3/graph/ save=data/es3/es.txt demo=1" &
# ./cycle_cover --constructor="name=es2 n=25 k=5 sigma=0.05 save_graph=data/es4/graph/ save=data/es4/es.txt demo=1" &
# ./cycle_cover --constructor="name=es2 n=25 k=5 sigma=0.1 save_graph=data/es5/graph/ save=data/es5/es.txt demo=1" &
# ./cycle_cover --constructor="name=es2 n=25 k=5 sigma=0.2 save_graph=data/es6/graph/ save=data/es6/es.txt demo=1" &
# ./cycle_cover --constructor="name=es2 n=25 k=5 sigma=0.4 save_graph=data/es7/graph/ save=data/es7/es.txt demo=1" &
# wait
./cycle_cover --constructor="name=es2 n=25 k=5 sigma=0.0125 save_graph=data/es8/graph/ save=data/es8/es.txt demo=1" &
./cycle_cover --constructor="name=es2 n=25 k=5 sigma=0.00625 save_graph=data/es9/graph/ save=data/es9/es.txt demo=1" &
wait