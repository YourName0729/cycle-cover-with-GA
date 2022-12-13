#pragma once 

#include <vector>
class dsu {

public :

    dsu() { ;}

    dsu( size_t size ) {
        for ( size_t i = 0 ; i < size ; i++ ) {
            p.push_back(i) ;
            rank.push_back(0) ;
        }
    }

    size_t find(size_t x ) { return x == p[x] ? x : ( p[x] = find(p[x]) ) ; }
 
    void unions(size_t x, size_t y ) {
        p[x] = y ;
    }

    void union_byrank(size_t x, size_t y ) {
        size_t xp = find(x), yp = find(y) ;
        if ( rank[xp] < rank[yp] ) {
            p[xp] = yp ;
        }
        else if ( rank[xp] > rank[yp] ){
            p[yp] = xp ;
        }
        else {
            p[yp] = xp ;
            rank[xp]++ ;
        }
    }

    std::vector<size_t> p ;
    std::vector<size_t> rank ;
};