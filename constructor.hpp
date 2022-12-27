#pragma once

#include "solver.hpp"
#include "factory.hpp"
#include "statistics.hpp"
#include <fstream>
#include <sstream>


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


    std::pair<std::shared_ptr<problem>, std::pair<solution, problem::obj_t> > construct_single( ESstats&stat, unsigned run )  {
        // std::cout << "es!\n";
        unsigned t = T - 1;

       

        std::string p_name = property("problem");
       


        // record initial condition 
        
        std::uniform_int_distribution<int> dis_mu(0, mu-1) ;

        std::vector<instance> population(mu) ;
    
        instance best_ins ;
        solution best_GAs_sol, best_solv4_sol ;
        problem::obj_t best_GAs_obj = 1 , best_solv4_obj = 0 ;
        std::vector<std::shared_ptr<problem>> problem_set(mu+lambda) ;
        std::vector<solution> GA_solution_set(mu+lambda) ;
        std::vector<solution> solv4_solution_set(mu+lambda) ;
        std::vector<problem::obj_t> GA_obj_set(mu+lambda) ;
        std::vector<problem::obj_t> solv4_obj_set(mu+lambda) ;


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
            std::stringstream ss ;
            ss << std::fixed << std::setprecision(3) << sigma ;
            string fname = meta["save_solution"] ;
            fname = src+ "run="+ std::to_string(run) + "_solution_step="+ss.str()+"_"+fname ;
            fout.open(fname);
            fout << std::fixed;
            begin = std::chrono::steady_clock::now();
            block = 100;
            if (meta.find("block") != meta.end()) block = static_cast<unsigned>(meta["block"]); 
        }



        auto update_best = [&](unsigned t, unsigned & Gs, unsigned index ) {
  
            if ( solv4_obj_set[index]/GA_obj_set[index] > best_solv4_obj/best_GAs_obj ) {
                best_solv4_obj = solv4_obj_set[index], best_GAs_obj = GA_obj_set[index], best_ins = population[index] ;
                if ( t != T ) Gs++ ;
            }

            if (t % G == 0) {
                if (5 * Gs > G) sigma_pos /= a, sigma_dat /= a;
                else if (5 * Gs < G) sigma_pos *= a, sigma_dat *= a;
                Gs = 0;
            }


            if ( t % block == 0 ) {
                auto end = std::chrono::steady_clock::now();

                if ( record_graph ) {
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() ;
                    stat.infos.push_back( ESdata(T-t, ms, solv4_obj_set[index]/GA_obj_set[index] , solv4_obj_set[index],  GA_obj_set[index] ) ) ;
    
                }


                if ( record_solution ) {
                    fout << "T=" << T-t << ' ';
                    fout << "t=" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() <<  ' ';
                    fout << "best_ratio=" << solv4_obj_set[index]/GA_obj_set[index] << ' ';
                    fout << "best_solv4_obj=" <<  solv4_obj_set[index] << ' ';
                    fout << "best_GAs=" << GA_obj_set[index] << ' ';

                    auto [pos, dat] = population[index] ;
                    fout << "best_instance=" << pos.size() << ' ';

                    fout << "X=" ;
                    for ( unsigned i = 0 ; i < pos.size() ; i++ )  fout << pos[i].first << ' ';
                    fout << "Y=" ;
                    for ( unsigned i = 0 ; i < pos.size() ; i++ )  fout << pos[i].second << ' ' ;
                    fout << "data=" ;
                    for ( unsigned i = 0 ; i < pos.size() ; i++ )  fout << dat[i] << ' ' ;
                    fout << "best_GAs_solution=" ;
                    fout << GA_solution_set[index] ;
                    fout << "best_solv4_solution=" ;
                    fout << solv4_solution_set[index] ;
                }
            }


        };


        std::vector<std::pair<problem::obj_t, unsigned>> order(mu);
        for ( unsigned i = 0 ; i < mu ; i++ ) {
            population[i] = initialze() ;
            auto p = ProblemFactory::produce(p_name, generate(population[i]), k);
            problem_set[i] = p, GA_solution_set[i] = solv->solve(*p), solv4_solution_set[i] = mmccpSolver.solve(*p) ;
            GA_obj_set[i] = p->objective(GA_solution_set[i]), solv4_obj_set[i] = p->objective(solv4_solution_set[i]) ;
            order[i] = {-solv4_obj_set[i]/GA_obj_set[i],i} ;
        }

        std::sort(order.begin(), order.end() ) ;    


        unsigned Gs = 0;
       
        update_best(T, Gs, order[0].second) ;

        order.resize(mu+lambda) ;
        while (t--) {
            std::vector<instance> offsprings ;
            
            for ( unsigned i = 0 ; i < lambda ; i++ ) {
                auto child = population[0] ;
                if ( mu > 1 ) child = crossover( population[dis_mu(gen)], population[dis_mu(gen)] ) ;

                offsprings.push_back( evolve(child) ) ;
            }

            population.insert(population.end(), offsprings.begin(), offsprings.end()) ;

            for ( unsigned i = 0 ; i < order.size() ; i++ ) {
                if ( i >= mu ) problem_set[i] = ProblemFactory::produce(p_name, generate(population[i]), k);

                GA_solution_set[i] = solv->solve(*problem_set[i]), solv4_solution_set[i] = mmccpSolver.solve(*problem_set[i]) ;
                GA_obj_set[i] = problem_set[i]->objective(GA_solution_set[i]), solv4_obj_set[i] = problem_set[i]->objective(solv4_solution_set[i]) ;
                order[i] = {-solv4_obj_set[i]/GA_obj_set[i],i} ;
            }

            std::sort(order.begin(), order.end() ) ;
            update_best(t, Gs, order[0].second) ;

            std::vector<instance> temp_pop(mu) ;
            auto temp_problem = problem_set ;
            auto temp_GA = GA_solution_set ; 
            auto temp_sovl4 = solv4_solution_set ;
            auto temp_GAobj = GA_obj_set ;
            auto temp_solv4_obj = solv4_obj_set ;
            for ( unsigned i = 0 ; i < mu ; i++ ) {
                temp_pop[i] = population[order[i].second] ;
                temp_problem[i] = problem_set[order[i].second] ;
                temp_GA[i] = GA_solution_set[order[i].second] ; 
                temp_sovl4[i] = solv4_solution_set[order[i].second] ;
                temp_GAobj[i] = GA_obj_set[order[i].second] ;
                temp_solv4_obj[i] = solv4_obj_set[order[i].second] ;
            }

            problem_set = temp_problem ;
            GA_solution_set = temp_GA ;
            solv4_solution_set = temp_sovl4 ;
            GA_obj_set = temp_GAobj ;
            solv4_obj_set = temp_solv4_obj ;
            population = temp_pop ;
            
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
            auto result = construct_single(stat, i) ;
            if ( result.second.second > best_obj ) 
                best_res = {result.first, result.second.first } ;
            stats.push_back(stat) ;
        }

        // write file 


        if (meta.find("save_graph") != meta.end()) {
            std::ofstream fout;
            string src = "data/min-max/es/" ;
            if (meta.find("file_loc") != meta.end() ) src = string(meta["file_loc"]) ;
            std::stringstream ss ;
            ss << std::fixed << std::setprecision(3) << sigma ;
            string fname = meta["save_graph"] ;
            fname = src+"step="+ss.str()+"_"+fname ;
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
                fout << "avg_t=" << avg_ms/repeat << ' ';
                fout << "avg_best_ratio=" << avg_best_r/repeat << ' ';
                fout << "avg_best_solv4_obj=" << avg_best_solv4/repeat << ' ';
                fout << "avg_best_GAs=" << avg_best_GAs/repeat << '\n';
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

