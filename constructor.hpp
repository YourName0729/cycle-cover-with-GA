#pragma once

#include "solver.hpp"
#include "factory.hpp"
#include "statistics.hpp"
#include <fstream>



class constructor : public agent {
public:
    constructor(const std::string& args = ""): agent(args + " role=constructor") {}

public:
    virtual std::pair<std::shared_ptr<problem>, solution> construct() = 0;
};

class DummyConstructor : public constructor {
public:
    DummyConstructor(const std::string& args = ""): constructor("name=dummy " + args), solv(SolverFactory::produce(property("solver"))), T(100), n(10), k(3),B(0) {
        if (meta.find("T") != meta.end()) T = static_cast<unsigned>(meta["T"]);
        if (meta.find("n") != meta.end()) n = static_cast<unsigned>(meta["n"]);
        if (meta.find("k") != meta.end()) k = static_cast<unsigned>(meta["k"]);
        if (meta.find("B") != meta.end()) B = static_cast<float>(meta["B"]);
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());

    }

public:
    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        unsigned t = T - 1;

        // MinSumProblem best_p(generate(n), k);
        auto best_p = ProblemFactory::produce(property("problem"), generate(n), k, B);
        solution best_s = solv->solve(*best_p);
        problem::obj_t mx = best_p->objective(best_s);
        while (t--) {
            auto pro = ProblemFactory::produce(property("problem"), generate(n), k,B);
            solution sol = solv->solve(*pro);
            problem::obj_t nmx = pro->objective(sol);
            if (nmx > mx) best_p = pro, best_s = sol, mx = nmx;
        }
        return {best_p, best_s};
    }

protected:
    virtual problem::graph_t generate(unsigned n) {
        problem::graph_t g(n);
        std::uniform_real_distribution<problem::obj_t> dis(1, 10);
        for (unsigned i = 0; i < n; ++i) {
            for (unsigned j = 0; j < n; ++j) {
                g(i, j) = i == j? 0 : dis(gen);
            }
        }
        return g;
    }

public :
    unsigned repeat ;

protected:
    // DummySolver solver;
    std::shared_ptr<solver> solv;
    std::default_random_engine gen;

    unsigned T;
    unsigned n, k; // # of nodes and # of cycles
    float B ;
};

class EvolutionStrategy : public DummyConstructor {
public:
    EvolutionStrategy(const std::string& args = ""): DummyConstructor("name=es " + args), sigma_pos(5000), sigma_dat(5), sigma(1), a(0.85), G(10), mu(10), lambda(70) {
        // default step size = 1 
        if (meta.find("sigma_pos") != meta.end()) sigma_pos = static_cast<float>(meta["sigma_pos"]);
        if (meta.find("sigma_dat") != meta.end()) sigma_dat = static_cast<float>(meta["sigma_dat"]);
        if (meta.find("sigma") != meta.end()) {
            sigma = static_cast<float>(meta["sigma"]);
            sigma_pos *= sigma, sigma_dat *= sigma;
        }
        if (meta.find("a") != meta.end()) a = static_cast<float>(meta["a"]);
        if (meta.find("pos_ub") != meta.end())     pos_ub = static_cast<float>(meta["pos_ub"]);
        if (meta.find("pos_lb") != meta.end())     pos_lb = static_cast<float>(meta["pos_lb"]);
        if (meta.find("data_ub") != meta.end())    data_ub = static_cast<float>(meta["data_ub"]);
        if (meta.find("data_lb") != meta.end())    data_lb = static_cast<float>(meta["data_lb"]);
        if (meta.find("trans_rate") != meta.end()) trans_rate = static_cast<float>(meta["trans_rate"]);
        if (meta.find("fly_speed") != meta.end())  fly_speed = static_cast<float>(meta["fly_speed"]);
        if (meta.find("mu") != meta.end()) mu = static_cast<float>(meta["mu"]);
        if (meta.find("lambda") != meta.end())  lambda = static_cast<float>(meta["lambda"]);
        if (meta.find("n") == meta.end()) n = std::uniform_int_distribution<unsigned>(100, 500)(gen);
        
        if (meta.find("repeat") != meta.end()) repeat = static_cast<unsigned>(meta["repeat"]);
        else repeat = 1 ;

        if (meta.find("demo") != meta.end())  demo = true;
    }
protected:
    using Pos = std::pair<problem::obj_t, problem::obj_t>;
    using Dat = problem::obj_t;
    using instance = std::pair<std::vector<Pos>, std::vector<Dat>>;

public:


    std::pair<std::shared_ptr<problem>, std::pair<solution, problem::obj_t> > construct_single( ESstats&stat )  {
        // std::cout << "es!\n";
        unsigned t = T - 1;

       

        std::string p_name = property("problem");
        std::uniform_int_distribution<int> dis_mu(0, mu-1) ;

        std::vector<instance> population ;
        for ( unsigned i = 0 ; i < mu ; i++ ) population.push_back(initialze()) ;

        auto best_ins = population[0] ;
        auto init_p = ProblemFactory::produce(p_name, generate(best_ins), k);
        solution best_GAs_sol = solv->solve(*init_p);
        solution best_solv4_sol = mmccpSolver.solve(*init_p) ;
        problem::obj_t best_GAs_obj   = init_p->objective(best_GAs_sol) ; 
        problem::obj_t best_solv4_obj = init_p->objective(best_solv4_sol) ; 

    
        // record stats
        bool record_graph = false, record_solution = false ;
        if (meta.find("save_graph") != meta.end()) record_graph = true;
        if (meta.find("save_solution") != meta.end()) record_solution = true;
        std::ofstream fout;
        unsigned block = T;
        std::chrono::steady_clock::time_point begin;
        begin = std::chrono::steady_clock::now();

        if ( record_graph ) {
            block = 100;
            if (meta.find("block") != meta.end()) block = static_cast<unsigned>(meta["block"]); 
        }

        if (record_solution) {
            string src = "data/min-max/es/" ;
            if (meta.find("file_loc") != meta.end() ) src = string(meta["file_loc"]) ;
            string fname = meta["save_solution"], sig_str = std::to_string(sigma) ;
            fname = src+ "solution_"+"step="+sig_str.substr(0, sig_str.find(".")+3)+"_"+fname ;
            fout.open(fname);
            fout << std::fixed;
            begin = std::chrono::steady_clock::now();
            block = 100;
            if (meta.find("block") != meta.end()) block = static_cast<unsigned>(meta["block"]); 
        }



        auto update_best = [&](unsigned t, unsigned & Gs, bool sorted = false ) {

            bool success = false ;
            for ( unsigned i = 0 ; i < population.size() ; i++ ) {
                auto p = ProblemFactory::produce(p_name, generate(population[i]), k);
                auto GAs_sol = solv->solve(*p) ;
                auto solv4_sol = mmccpSolver.solve(*p) ;

                auto GAs_obj   = p->objective(GAs_sol) ;
                auto solv4_obj = p->objective(solv4_sol) ;
                if ( solv4_obj/GAs_obj > best_solv4_obj/best_GAs_obj ) {
                    best_GAs_obj = GAs_obj, best_solv4_obj = solv4_obj, best_ins = population[i], best_GAs_sol = GAs_sol, best_solv4_sol = solv4_sol ;
                    success = true ;
                }

                if ( sorted ) break ;
            }

            if ( t != T && success ) Gs++ ;

            if (t % G == 0) {
                if (5 * Gs > G) sigma_pos /= a, sigma_dat /= a;
                else if (5 * Gs < G) sigma_pos *= a, sigma_dat *= a;
                Gs = 0;
            }


            if ( t % block == 0 ) {
                auto end = std::chrono::steady_clock::now();

                if ( record_graph ) {
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() ;
                    stat.infos.push_back( ESdata(T-t, ms, best_solv4_obj/best_GAs_obj, best_solv4_obj, best_GAs_obj ) ) ;
    
                }


                if ( record_solution ) {
                    fout << "Generation : T=" << T-t << ' ';
                    fout << "t=" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() <<  '\n';
                    fout << "best_ratio=" << best_solv4_obj/best_GAs_obj << ' ';
                    fout << "best_solv4_obj=" << best_solv4_obj << ' ';
                    fout << "best_GAs=" << best_GAs_obj << '\n';

                    fout << "best_instance= " << '\n';
                    fout << *ProblemFactory::produce(p_name, generate(best_ins), k) ;
                    fout << "best_GAs_solution " << "\n" ;
                    fout << best_GAs_sol ;
                    fout << "best_solv4_solution " << "\n" ;
                    fout << best_solv4_sol << "\n\n" ;
                }
            }


        };


        // record initial condition 
        
        unsigned Gs = 0;
       
        update_best(T, Gs) ;

        while (t--) {
            std::vector<instance> offsprings ;
            
            auto child = population[0] ;
            if ( mu > 1 ) child = crossover( population[dis_mu(gen)], population[dis_mu(gen)] ) ;
    

            for ( unsigned i = 0 ; i < lambda ; i++ ) {
                offsprings.push_back( evolve(child) ) ;
            }

            population.insert(population.end(), offsprings.begin(), offsprings.end()) ;

            std::vector<std::pair<problem::obj_t, unsigned>> order(mu+lambda);
            for ( unsigned i = 0 ; i < order.size() ; i++ ) {
                auto p = ProblemFactory::produce(p_name, generate(population[i]), k);
                order[i] = {-p->objective(mmccpSolver.solve(*p))/p->objective(solv->solve(*p)),i} ;
            }

            std::sort(order.begin(), order.end() ) ;

            std::vector<instance> fittest_population(mu) ;
            for ( unsigned i = 0 ; i < mu ; i++ ) fittest_population[i] = population[order[i].second] ;
            population = fittest_population ;

            update_best(t, Gs, true) ;
            
        }

        if (record_solution) fout.close();

        
        return {ProblemFactory::produce(property("problem"), generate(best_ins), k), {best_GAs_sol, best_solv4_obj/best_GAs_obj} };
    }




    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        std::vector<ESstats> stats ;
        std::pair<std::shared_ptr<problem>, solution> best_res ;
        problem::obj_t best_obj = 0.f ;
        for ( unsigned i = 0 ; i < repeat ; i++ ) {
            ESstats stat ;
            auto result = construct_single(stat) ;
            if ( result.second.second > best_obj ) 
                best_res = {result.first, result.second.first } ;
            stats.push_back(stat) ;
        }

        // write file 


        if (meta.find("save_graph") != meta.end()) {
            std::ofstream fout;
            string src = "data/min-max/es/" ;
            if (meta.find("file_loc") != meta.end() ) src = string(meta["file_loc"]) ;
            string fname = meta["save_graph"], sig_str = std::to_string(sigma) ;
            fname = src+"step="+sig_str.substr(0, sig_str.find(".")+3)+"_"+fname ;
            fout.open(fname);
            fout << std::fixed ;

            for ( unsigned i = 0 ; i < stats[0].infos.size() ; i++ ){
                unsigned cycle = stats[0].infos[i].T ;
                float avg_ms = 0.0, avg_best_r = 0.0, avg_best_solv4 = 0.0, avg_best_GAs = 0.0 ;
                for ( unsigned j = 0 ; j < stats.size() ; j++ ) {
                    avg_ms += stats[j].infos[i].t ;
                    avg_best_r += stats[j].infos[i].best_ratio ;
                    avg_best_solv4 += stats[j].infos[i].best_solv4 ;
                    avg_best_GAs += stats[j].infos[i].best_GAs ;
                }

                fout << "T=" << cycle << ' ';
                fout << "avg t=" << avg_ms/repeat << ' ';
                fout << "avg best_ratio=" << avg_best_r/repeat << ' ';
                fout << "avg best_solv4_obj=" << avg_best_solv4/repeat << ' ';
                fout << "avg best_GAs=" << avg_best_GAs/repeat << '\n';
            }
           
            fout.close() ;
        }


        return best_res ;
    }

protected:

    instance crossover( const instance &p1, const instance &p2 ) { // two parent for all var 
        auto [pos1, dat1] = p1; 
        auto [pos2, dat2] = p2; 
        for (auto& [x1, y1] : pos1) for ( auto&[x2, y2] :pos2 ) {
            x1 = (x1+x2)/2 ;
            y1 = (y1+y2)/2 ;
        }
        for (auto& v1 : dat1) for ( auto& v2 : dat2 ) v1 = (v1+v2)/2 ;
        return {pos1, dat1};
    }

    virtual instance evolve(const instance& org) {
        std::normal_distribution<float> dis_pos(0, sigma_pos), dis_dat(0, sigma_dat);
        auto [pos, dat] = org;
        for (auto& [x, y] : pos) {
            x += dis_pos(gen), y += dis_pos(gen);
            x = std::min(std::max(x, pos_lb), pos_ub);
            y = std::min(std::max(y, pos_lb), pos_ub);
        }
        for (auto& v : dat) v = std::min(std::max(v + dis_dat(gen), data_lb), data_ub);
        return {pos, dat};
    }   

    instance initialze() {
        std::vector<Pos> pos;
        std::vector<Dat> data;
        std::uniform_real_distribution<problem::obj_t> dis_pos(pos_lb, pos_ub), dis_data(data_lb, data_ub);
        for (unsigned i = 0; i < n; ++i) {
            pos.push_back({dis_pos(gen), dis_pos(gen)});
            data.push_back(dis_data(gen));
        }
        return {pos, data};
    }

    problem::graph_t generate(const instance& ins) const {
        const auto& [pos, dat] = ins;
        problem::graph_t g(n);
        for (unsigned i = 0; i < n; ++i) {
            for (unsigned j = 0; j < n; ++j) {
                problem::obj_t dx = pos[i].first - pos[j].first;
                problem::obj_t dy = pos[i].second - pos[j].second;
                g(i, j) = (dat[i] + dat[j]) / trans_rate / 2.f + std::sqrt(dx * dx + dy * dy) / fly_speed;
            }
        }
        return g;
    }


protected:
    float sigma_pos, sigma_dat, sigma, a;
    unsigned G;
    unsigned mu, lambda ;
    float pos_lb = 0.f, pos_ub = 5000.f;
    float data_lb = 5.f, data_ub = 10.f;

    float trans_rate = 1.f;
    float fly_speed = 10.f;

    MMCCPSolver mmccpSolver ;
    bool demo = false;
};

class InstanceMinDeploy : public DummyConstructor {
public:
    InstanceMinDeploy(const std::string& args = ""): DummyConstructor("name=min-deploy " + args) {
        T = 1;
    }

protected:
    virtual problem::graph_t generate(unsigned n) override {
        if (meta.find("n") == meta.end()) n = std::uniform_int_distribution<unsigned>(100, 500)(gen);
        std::vector<std::pair<problem::obj_t, problem::obj_t>> pos;
        std::vector<problem::obj_t> data;
        problem::obj_t trans_rate = 1.f;
        problem::obj_t fly_speed = 10.f;

        std::uniform_real_distribution<problem::obj_t> dis_pos(0, 5000), dis_data(5, 10);
        for (unsigned i = 0; i < n; ++i) {
            pos.push_back({dis_pos(gen), dis_pos(gen)});
            data.push_back(dis_data(gen));
        }

        problem::graph_t g(n);
        for (unsigned i = 0; i < n; ++i) {
            for (unsigned j = 0; j < n; ++j) {
                problem::obj_t dx = pos[i].first - pos[j].first;
                problem::obj_t dy = pos[i].second - pos[j].second;
                g(i, j) = (data[i] + data[j]) / trans_rate / 2.f + std::sqrt(dx * dx + dy * dy) / fly_speed;
            }
        }
        return g;
    }
};

