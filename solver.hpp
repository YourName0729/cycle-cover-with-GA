#pragma once

#include <random>
#include <algorithm>
#include <utility>
#include <array>
#include <climits>
#include <stack>
#include <iomanip>

#include "agent.hpp"
#include "problem.hpp"
#include "dsu.hpp"
#include "match.hpp"


using MWM = MaximumWeightedMatching<float>;
using Edge = MWM::InputEdge;

class solver : public agent {
public:
    solver(const std::string& args = ""): agent(args + " role=solver") {}

public:
    virtual solution solve(const problem& ins) = 0;
};

class DummySolver : public solver {
public:
    DummySolver(const std::string& args = ""): solver(args + " name=dummy"), T(100) {
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());
        if (meta.find("T") != meta.end()) T = static_cast<unsigned>(meta["T"]);
    }

    virtual solution solve(const problem& ins) override {
        solution best = generate(ins);
        unsigned t = T;
        while (t--) {
            solution nsol = generate(ins);
            if (ins.objective(nsol) < ins.objective(best)) best = nsol;
        }
        return best;
    }

private:
    solution generate(const problem& ins) {
        solution sol(ins.get_k());
        std::uniform_int_distribution<unsigned> dis(0, ins.get_k() - 1);
        for (unsigned i = 0; i < ins().size(); ++i) {
            sol[dis(gen)].push_back(i);
        }
        return sol;
    }

private:
    std::default_random_engine gen;

    unsigned T;
};

class GeneticAlgorithm : public solver {
public:
    GeneticAlgorithm(const std::string& args = "") : solver("name=ga" + args), m(6), T(500) {
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());
        if (meta.find("m") != meta.end()) m = static_cast<unsigned>(meta["m"]);
        if (meta.find("T") != meta.end()) T = static_cast<unsigned>(meta["T"]);
    }

protected:
    using gene = std::vector<unsigned>;

protected:
    gene random_gene() {
        std::uniform_int_distribution<> dis(0, n - 1);
        gene re;
        for (unsigned i = 0; i < n; ++i) {
            re.push_back(i);
            unsigned idx = std::uniform_int_distribution<unsigned>(0, i)(gen);
            std::swap(re[i], re[idx]);
        }
        // for (unsigned i = 0; i < n; ++i) re.push_back(dis(gen));
        for (unsigned i = n; i < len; ++i) {
            re.push_back(i - n);
        }
        return re;
    }

    virtual std::vector<gene> initialize_pool() {
        std::vector<gene> pool(m);
        for (unsigned i = 0; i < m; ++i) {
            pool[i] = random_gene();
        }
        return pool;
    }

    solution decode(const gene& gn) const {
        auto g = gn;
        std::sort(g.begin() + n, g.end());
        solution sol(len - n + 1);
        unsigned l = 0;
        for (unsigned i = n; i < len; ++i) {
            for (unsigned j = l; j < g[i]; ++j) {
                sol[i - n].push_back(g[j]);
            }
            l = g[i];
        }
        for (unsigned j = l; j < n; ++j) {
            sol.back().push_back(g[j]);
        }
        return sol;
    }

    virtual std::vector<gene> selection(const std::vector<gene>& org, const problem& ins) {
        std::vector<problem::obj_t> score;
        problem::obj_t sum = 0;
        for (auto& g : org) {
            // score.push_back(std::exp(-ins.objective(decode(g))));
            // std::cout << ins.objective(decode(g)) << '\n';
            score.push_back(1 / ins.objective(decode(g)));
            sum += score.back();
            // std::cout << score.back() << ' ';
        }
        // std::cout << " sum " << sum << '\n';

        std::uniform_real_distribution<problem::obj_t> dis(0, sum);
        std::vector<problem::obj_t> trials;
        for (unsigned i = 0; i < org.size(); ++i) {
            trials.push_back(dis(gen));
        }
        std::sort(trials.begin(), trials.end());

        std::vector<gene> re;
        problem::obj_t acc = 0;
        unsigned l = 0;
        for (unsigned i = 0; i < trials.size(); ++i) {
            while (acc + score[l] < trials[i]) acc += score[l++];
            re.push_back(org[i]);
        }

        std::shuffle(re.begin(), re.end(), gen);
        return re;
    }

    virtual std::vector<gene> crossover(const std::vector<gene>& pool) {
        std::vector<gene> re;
        for (unsigned i = 0; i + 1 < pool.size(); i += 2) {
            // std::cout << "-> " << i << ' ' << pool.size() << '\n';
            // for (auto g : pool) {
            //     for (auto v : g) {
            //         std::cout << v << ' ';
            //     }
            //     std::cout << '\n';
            // }
            // cycle crossover
            gene ch[2] = {gene(n), gene(n)};
            std::vector<unsigned> vst(len, 0);

            std::vector<unsigned> inv(n);
            for (unsigned j = 0; j < n; ++j) {
                inv[pool[i][j]] = j;
            }

            unsigned alt = 1;
            for (unsigned j = 0; j < n; ++j) {
                if (!vst[j]) alt ^= 1u;
                while (!vst[j]) {
                    ch[alt][j] = pool[i][j], ch[alt ^ 1u][j] = pool[i + 1][j];
                    vst[j] = 1;
                    j = inv[pool[i + 1][j]];
                }
            }

            std::vector<unsigned> cuts;
            for (unsigned j = n; j < len; ++j) {
                cuts.push_back(pool[i][j]);
                cuts.push_back(pool[i + 1][j]);
            }
            std::shuffle(cuts.begin(), cuts.end(), gen);


            std::fill(vst.begin(), vst.end(), false);
            for (unsigned j = 0; j < cuts.size() && ch[0].size() < len; ++j) {
                if (vst[cuts[j]]) continue;
                vst[cuts[j]] = true;
                ch[0].push_back(cuts[j]);
            }

            std::fill(vst.begin(), vst.end(), 0);
            std::reverse(cuts.begin(), cuts.end());
            for (unsigned j = 0; j < cuts.size() && ch[1].size() < len; ++j) {
                if (vst[cuts[j]]) continue;
                vst[cuts[j]] = true;
                ch[1].push_back(cuts[j]);
            }

            re.push_back(ch[0]);
            re.push_back(ch[1]);
        }

        std::shuffle(re.begin(), re.end(), gen);
        return re;
    }

    virtual std::vector<gene> mutation(const std::vector<gene>& pool) {
        
        std::vector<gene> re;
        std::uniform_int_distribution<unsigned> dis2n(0, n - 1);
        std::uniform_int_distribution<unsigned> dis2nm1(0, n - 2);

        std::uniform_int_distribution<unsigned> dis2k(0, len - n - 1);
        std::uniform_int_distribution<unsigned> dis2nmk(0, n - (len - n) - 1);
        for (auto& ge : pool) {
            re.push_back(ge);
            auto& g = re.back();

            // swap mutaion
            auto x = dis2n(gen), y = (x + dis2nm1(gen)) % n;
            std::swap(g[x], g[y]);

            // choose one cut and change it
            unsigned idx = dis2k(gen); // index to change
            std::swap(g[n], g[n + idx]);
            std::sort(g.begin() + n + 1, g.end());
            unsigned cut_idx = dis2nmk(gen);
            for (unsigned j = n + 1; j < len && g[j] <= cut_idx; ++j, ++cut_idx);
            g[n] = cut_idx;
        }

        return re;        
    }

public:
    virtual solution solve(const problem& ins) override {
        // std::cout << "solve\n";
        n = ins().size();
        len = n + ins.get_k() - 1;

        auto pool = initialize_pool();
        // std::cout << "---\n";
        auto best_gene = pool[0];
        problem::obj_t best_score = ins.objective(decode(best_gene));

        unsigned t = T;
        while (t--) {
            // std::cout << "t = " << t << '\n';
            // std::cout << "selection\n";
            pool = selection(pool, ins);
            // std::cout << "crossover\n";
            pool = crossover(pool);
            // std::cout << "mutation\n";
            pool = mutation(pool);
            

        
            // for (auto g : pool) {
            //     for (auto v : g) {
            //         std::cout << v << ' ';
            //     }
            //     std::cout << " = " << ins.objective(decode(g)) << '\n';
            // }

            for (auto& g : pool) {
                problem::obj_t nobj = ins.objective(decode(g));
                if (nobj < best_score) best_gene = g, best_score = nobj;
            }
        }

        return decode(best_gene);
    }

protected:
    unsigned m; // size of gene pool
    unsigned T; // # of iterations


    unsigned n; // # of nodes
    unsigned len; // length of a gene <permutation of # of nodes, k-1 cut on # of nodes>

    std::default_random_engine gen;
};

class ElitismGA : public GeneticAlgorithm {
public:
    ElitismGA(const std::string& args = "") : GeneticAlgorithm("name=elitism-ga " + args), elite_rate(0.1) {
        if (meta.find("elite_rate") != meta.end()) elite_rate = static_cast<float>(meta["elite_rate"]);
    }

protected:
    virtual std::vector<gene> selection(const std::vector<gene>& org, const problem& ins) override {
        std::vector<gene> re = org;
        std::sort(re.begin(), re.end(), [&](const gene& a, const gene& b) {
            return ins.objective(decode(a)) < ins.objective(decode(b));
        });

        re.erase(re.begin() + static_cast<unsigned>(re.size() * elite_rate), re.end());
        std::shuffle(re.begin(), re.end(), gen);
        return re;
    }

public:
    virtual solution solve(const problem& ins) override {
        n = ins().size();
        len = n + ins.get_k() - 1;

        auto pool = initialize_pool();
        // std::cout << "---\n";
        auto best_gene = pool[0];
        problem::obj_t best_score = ins.objective(decode(best_gene));
        unsigned t = T;
        while (t--) {
            // std::cout << "t = " << t << '\n';
            // std::cout << "selection\n";
            auto elite = selection(pool, ins);
            // std::cout << "crossover\n";
            auto child = crossover(elite);
            while (elite.size() + child.size() < m) {
                std::shuffle(elite.begin(), elite.end(), gen);
                auto new_child = crossover(elite);
                child.insert(child.end(), new_child.begin(), new_child.end());
            }
            while(elite.size() + child.size() > m) child.pop_back();
            // pool = crossover(pool);
            // std::cout << "mutation\n";
            child = mutation(child);
            pool = elite;
            pool.insert(pool.end(), child.begin(), child.end());
        
            // for (auto g : pool) {
            //     for (auto v : g) {
            //         std::cout << v << ' ';
            //     }
            //     std::cout << " = " << ins.objective(decode(g)) << '\n';
            // }

            for (auto& g : pool) {
                problem::obj_t nobj = ins.objective(decode(g));
                if (nobj < best_score) best_gene = g, best_score = nobj;
            }
        }

        return decode(best_gene);
    } 

protected:
    float elite_rate;
};



class MCCPSolver : public solver {
public:
    MCCPSolver(const std::string& args = ""): solver(args + " name=MCCP"), T(100), demo(0) {
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());
        if (meta.find("T") != meta.end()) T = static_cast<unsigned>(meta["T"]);
        if (meta.find("demo") != meta.end()) demo = static_cast<bool>(meta["demo"]);
    }

    std::vector< std::vector< std::pair<int,int> > > find_MSTs( problem::graph_t graph ) {
        size_t n = graph.size() ;
        std::vector< std::pair<float,int> > distance(n, std::make_pair(INT_MAX,-1) ) ;
        std::vector<bool> visited(n,false) ;
        std::vector< std::vector< std::pair<int,int> > > MSTs ;
        int src = 0 ; 

        while ( src != -1 ) {
            auto cur = src ;
            src = -1 ;
          
            std::vector< std::pair<int,int> > mst ;
            visited[cur] = true ;
            for ( size_t i = 0 ; i < n ;i++ ) {
                if ( distance[i].first > graph(cur,i) && graph(cur,i) > 0 && !visited[i] )
                    distance[i] = std::make_pair(graph(cur,i),cur) ;
            }

            float min_weight = INT_MAX ;
            int nextN = -1 ;
            for ( size_t i = 0 ; i < n ; i++ ) {
                if ( distance[i].first < min_weight && !visited[i] ) {
                    min_weight = distance[i].first ;
                    nextN = i ;
                }
            }

            while ( nextN != -1 ) {
                auto p = distance[nextN] ;
                auto minN = p.second ;
                mst.push_back(std::make_pair(minN,nextN)) ;
                cur = nextN ;
                visited[cur] = true ;
                min_weight = INT_MAX ;
                nextN = -1 ;
                for ( size_t i = 0 ; i < n ;i++ ) {
                    if ( distance[i].first > graph(cur,i) && graph(cur,i) > 0 && !visited[i] )
                        distance[i] = std::make_pair(graph(cur,i),cur) ;
                }

                for ( size_t i = 0 ; i < n ; i++ ) {
                    if ( distance[i].first < min_weight && !visited[i] ) {
                        min_weight = distance[i].first ;
                        nextN = i ;
                    }
                }
            }

            if ( mst.size() == 0 ) 
                mst.push_back(std::make_pair(cur,cur)) ;

            MSTs.push_back(mst) ;

            for ( size_t i = 0 ; i < n ; i++ ) {
                if ( !visited[i] ) {
                    src = i ;
                    break ;
                }
            } 
        }
        return MSTs ;
    }

    std::vector< std::pair<int,int> > FindEC( std::vector< std::pair<int,int> > edges, size_t n ) {
     
        if ( edges.size() == 1 ) return edges ;

        problem::graph_t g(n) ;
        std::vector< std::pair<int,int> > ec ;
        std::vector<int> degs ;
        for ( auto e : edges ) {
            g(e.first,e.second) = g(e.first,e.second)+1 ;
            g(e.second,e.first) = g(e.second,e.first)+1 ;
        }

         
        for ( size_t i = 0 ; i < n ; i++ ) {
            int deg = 0 ;
            for ( size_t j = 0 ; j < n ; j++ ) {
                deg += g(i,j) ;
            }
            degs.push_back(deg) ;
        }

        std::stack<int> vertex_stack ;
        vertex_stack.push(edges[0].first) ;
        int last_vertex = -1 ;

        while ( ! vertex_stack.empty() ) {
            auto cur = vertex_stack.top() ;
        
            if ( degs[cur] == 0 ) {
                if ( last_vertex != -1 ) {
                    ec.push_back(std::make_pair(last_vertex,cur)) ;
                }

                last_vertex = cur ;
                vertex_stack.pop() ;
            }    
            else {
                int nextV ;
                for ( nextV = 0 ; nextV < n ; nextV++ ) {
                    if ( g(cur,nextV) )
                        break ;
                }

                vertex_stack.push(nextV) ;
                g(cur,nextV) = g(cur,nextV)-1 ;
                g(nextV,cur) = g(nextV,cur)-1 ;
                degs[cur]-- ;
                degs[nextV]-- ;

            }
        }


        return ec ; 
    }

    solution christofideMethod( float threshold, const problem& ins ) {

        solution Ci ;
        

        // construct a graph from G_prime by removing the large edges
        problem::graph_t graph = ins.copy() ; 
        for ( size_t i = 0 ; i < graph.size(); i++ ) {
            for ( size_t j = i+1 ; j < graph.size(); j++ ) {
                if ( ins.copy()(i,j) > threshold ) {
                    graph(i,j) = graph(j,i) = 0 ;
                }
            }
        }

        // find MSTs of each connected component 
        std::cout << "\n----------Step1 : Find MSTs for each connected components ----------\n\n" ;
        auto MSTs = find_MSTs(graph) ;

        std::cout << "The number of minimum spanning trees is : " << MSTs.size() ;
     
        std::cout << "\n----------Step1 : Find MSTs for each connected components ----------\n\n" ; 

        std::cout << "\n----------Step2 : Find minimum weighted perfect mathcing  ----------\n\n" ; 


        // Find minimum weighted perfect matching 

        // find odd degree nodes V_o 
        std::vector<int> V_o ;
        std::map<int,int> V_o_mapping ; // reverse mapping store the info of the mathcing in which MSTs

        for ( size_t i = 0 ; i < MSTs.size() ;i++ ) {
            auto mst = MSTs[i] ;
            std::map<int,int> deg ;
            for ( auto p : mst ) {
                deg[p.first]++ ;
                deg[p.second]++ ;
            }

            for ( auto node : deg ) {
                if ( node.second % 2 ) {
                    V_o.push_back(node.first) ;
                    V_o_mapping[node.first] = i ;
                }
            }
        }

        std::cout << "The number of odd degree nodes : " << V_o.size() << std::endl ;
        // construct G_o complete graph 
   
        const int n_vertices = V_o.size() ;
 

        // use maxixmum matching algorithm to compute minimum matching 
        // set each edge : new_weight = (max_weight + 1) - edge_weight
        std::vector< std::pair<int,int > > matchings ;


        if ( n_vertices > 0 ) {
            std::vector<Edge> edges ; 

            float max_w = 0 ;

            for ( int i = 0 ; i < n_vertices ; i++ ) {
                for ( int j = i+1 ; j < n_vertices ; j++ ) {
                    if ( max_w < ins.copy()(V_o[i],V_o[j]) ) max_w = ins.copy()(V_o[i],V_o[j]) ;
                }
            }

            for ( int i = 0 ; i < n_vertices ; i++ ) {
                for ( int j = i+1 ; j < n_vertices ; j++ ) {
                    edges.push_back( { V_o[i]+1, V_o[j]+1, max_w+1-ins.copy()(V_o[i],V_o[j]) } );
                    edges.push_back( { V_o[j]+1, V_o[i]+1, max_w+1-ins.copy()(V_o[i],V_o[j]) } );
                }
            }

            auto matches = MWM(graph.size(), edges).maximum_weighted_matching().second ;

            for ( auto i = 0 ; i <= graph.size() ; i++ ) {
                if ( matches[i] && matches[i] < i ) matchings.push_back( std::make_pair(i-1, matches[i]-1) ) ;
            }

    

        }
        // form eulerian circuit 

        std::cout << "\n----------Step2 : Find minimum weighted perfect mathcing  ----------\n" ;


        std::cout << "\n----------Step3 : Form Eulerian Circuit  ---------------------------\n\n" ;



        std::vector< std::vector< std::pair<int,int > > > ECs ;
        dsu trees(MSTs.size()) ;

        for ( auto m : matchings ) {
            trees.union_byrank(V_o_mapping[m.first],V_o_mapping[m.second]) ;
        }

      

        std::map<int,int > ECs_mapping ;

   

        for ( size_t i = 0 ; i < MSTs.size(); i++ ) {
            
            auto group = trees.find(i) ;
            if ( ECs_mapping.find(group) == ECs_mapping.end() ) {
                ECs_mapping[group] = ECs.size() ;
                ECs.push_back(MSTs[i]) ;
            }
            else ECs[ECs_mapping[group]].insert(ECs[ECs_mapping[group]].end(), MSTs[i].begin(), MSTs[i].end() ) ;
        }

        for ( auto m : matchings ) {
            ECs[ECs_mapping[trees.find(V_o_mapping[m.first])]].push_back(m) ;
        }

        std::cout << "The number of ECs is : " << ECs.size() << std::endl ;

        std::cout << "\n----------Step3 : Form Eulerian Circuit  ---------------------------\n" ;
       

        std::cout << "\n----------Step4 : Obtain tour from EC and tour division  -----------\n\n" ; 

        // Obtain tour from EC and tour division 
        for ( size_t i = 0 ; i < ECs.size() ; i++ ) {
            
            auto EC = FindEC(ECs[i], ins.copy().size() ) ;
            float tour_cost = 0 ;
            std::vector<unsigned> EC_visit ;
 
            for ( auto p : EC ) {
                tour_cost += ins.copy()(p.first,p.second) ;
                EC_visit.push_back(p.first) ;
            }
            EC_visit.push_back(EC[0].first) ;

            std::map<int,int> visit ;
            std::vector<unsigned> tour_visit ;

            // obtain tour by shortcutting the repeated node 
            for ( size_t j = 0 ; j < EC_visit.size()-1 ; j++ ) {
                if ( visit.find(EC_visit[j]) == visit.end() ) {
                    visit[EC_visit[j]] = 1 ;
                    tour_visit.push_back(EC_visit[j]) ;
                }
                else {
                    tour_cost += ins.copy()(EC_visit[j-1],EC_visit[j+1])-ins.copy()(EC_visit[j-1],EC_visit[j])-ins.copy()(EC_visit[j],EC_visit[j+1]) ;
                }
            }
            tour_visit.push_back(EC_visit.back()) ;

          
            if ( tour_cost <= ins.get_k() ) {
                Ci.push_back(tour_visit) ;
                std::cout << "Add new tour to Ci , size = " << Ci.back().size() << "\n" ; 
            }
            else {
                auto split = tour_visit ;
                auto residual_cost = tour_cost ;
                while ( residual_cost > ins.get_k() ) {
                    std::cout << "Tour cost exceed limit : " << residual_cost << "\n" ;
                    split.pop_back() ;
                    float newPath_cost = 0 ;
                    std::vector<unsigned> newPath ; 
                    newPath.push_back(split[0]) ;
                    size_t l = split.size()-1, r = 1 ;
                    while ( newPath_cost+std::min(ins.copy()(split[l],split[l+1]),ins.copy()(split[r],split[r-1]) ) <= ins.get_k()/2 ) {
                        newPath_cost += std::min(ins.copy()(split[l],split[l+1]),ins.copy()(split[r],split[r-1]) ) ;
                        if ( ins.copy()(split[l],split[l+1]) <= ins.copy()(split[r],split[r-1]) ) {
                            newPath.insert(newPath.begin(), split[l]) ;
                            l-- ;
                        }
                        else {
                            newPath.push_back(split[r]) ;
                            r++ ;
                        }
                    }
                
                    newPath.push_back(newPath[0]) ;
                    std::cout << "sub-path cost " << newPath_cost << "\n" ;
                    std::cout << "The number of node in the tour sub-path " << newPath.size()-1 << "\n" ;
              
                    newPath_cost += ins.copy()(split[r-1],split[l+1]) ;
                    if ( l == split.size()-1 ) {
                        split = std::vector<unsigned>(split.begin()+r,split.end()) ;
                    }
                    else {
                        split = std::vector<unsigned>(split.begin()+r,split.begin()+l+1) ;
                    }

                    std::cout << "The number of node in the residual_tour " << split.size()-1 << "\n" ;

                    residual_cost = 0 ;
                    for ( size_t j = 0 ; j < split.size()-1 ; j++ ) {
                        residual_cost += ins.copy()(split[j],split[j+1]) ;
                    }
                    residual_cost += ins.copy()(split[0],split.back()) ;

                    split.push_back(split[0]) ;
                    Ci.push_back(newPath) ;

                    std::cout << "Add new tour to Ci , size = " << Ci.back().size() << "\n" ; 
                }

                if ( residual_cost != 0 or split.size() != 0 ) {
                    residual_cost += ins.copy()(split[0],split.back()) ; 
                    Ci.push_back(split) ;

                    std::cout << "Add new tour to Ci , size = " << Ci.back().size() << "\n" ; 
                }
            }

        
        }

        std::cout << "\n----------Step4 : Obtain tour from EC and tour division  -----------\n" ; 

        return Ci ;
    }

    virtual solution solve(const problem& ins) override {
        solution best ;
        size_t n = ins.copy().size() ;
        best.resize(n) ; 
        if ( ! demo ) std::cout.setstate(std::ios_base::failbit);
        for ( size_t i = 2 ; i < n ; i++ ) {
            
            float threshold = 1.0*ins.get_k()/i ;
            std::cout << "----------Iteration " << i << " Edge weight threshold = " << threshold << " ------------\n\n" ; 
            solution Ci = christofideMethod(threshold, ins ) ;

            std::cout << "----------Iteration " << i << " Ci has " << Ci.size() << " cycles  ------------\n\n" ; 
            if ( Ci.size() < best.size() ) 
                best = Ci ;

            // early return
            if ( Ci.size() > best.size() ) break ;
        }

        std::cout.clear();
        return best ;
    }



private:
    std::default_random_engine gen;

    bool demo ; 
    unsigned T;
};



class MMCCPSolver : public solver {
public:
    MMCCPSolver(const std::string& args = ""): solver(args + " name=min-max"), T(100), demo(0) {
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());
        if (meta.find("T") != meta.end()) T = static_cast<unsigned>(meta["T"]);
        if (meta.find("demo") != meta.end()) demo = static_cast<bool>(meta["demo"]);
    }

    virtual solution solve(const problem& ins) override {
        if ( ! demo ) std::cout.setstate(std::ios_base::failbit);
        solution best ;

        float lb = 0, rb = 3600 ;
        auto minB = rb ;
        auto tar = ins.get_k() ;
        while ( lb <= rb ) {

            auto mid = (rb+lb)/2 ;
            auto mccp_ins = MinCycleProblem(ins.copy(),mid) ;

            mccpsolver = MCCPSolver() ;
            auto sol = mccpsolver.solve(mccp_ins) ;
            auto maxB = mccp_ins.max_cost(sol) ;
            if (  mccp_ins.objective(sol) <= tar ) {
                rb = mid-1 ;
                if ( minB > maxB ) minB = maxB, best = sol ;
            }
            else {
                lb = mid+1 ;
            }

            std::cout << "tour budget B = " << std::setw(8) << mid ;
            std::cout << " actual maximum cost = " << maxB  << " need " << std::setw(2) <<  mccp_ins.objective(sol) << " cycles "  ;
            if ( mccp_ins.objective(sol) <= tar ) std::cout << ", decrease budget B \n" ;
            else std::cout << ", increase budget B \n" ;
        }

        std::cout.clear();
        return best;
    }


private:
    std::default_random_engine gen;

    MCCPSolver mccpsolver ;
    bool demo ;
    unsigned T;
};