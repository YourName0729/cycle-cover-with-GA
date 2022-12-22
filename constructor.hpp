#pragma once

#include "solver.hpp"
#include "factory.hpp"


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
    EvolutionStrategy(const std::string& args = ""): DummyConstructor("name=es " + args), sigma_pos(50), sigma_dat(0.05), a(0.85), G(10), mu(10), lambda(70) {
        if (meta.find("sigma_pos") != meta.end()) sigma_pos = static_cast<float>(meta["sigma_pos"]);
        if (meta.find("sigma_dat") != meta.end()) sigma_dat = static_cast<float>(meta["sigma_dat"]);
        if (meta.find("sigma") != meta.end()) {
            float d = static_cast<float>(meta["sigma"]);
            sigma_pos *= d, sigma_dat *= d;
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


    std::pair<std::shared_ptr<problem>, std::pair<solution, problem::obj_t> > run()  {
        // std::cout << "es!\n";
        unsigned t = T - 1;

        

        std::string p_name = property("problem");
        std::uniform_int_distribution<int> dis_mu(0, mu-1) ;

        std::vector<instance> population ;
        for ( unsigned i = 0 ; i < mu ; i++ ) population.push_back(initialze()) ;

        auto best_ins = population[0] ;
        auto init_p = ProblemFactory::produce(p_name, generate(best_ins), k);
        solution best_s = solv->solve(*init_p);
        problem::obj_t target = init_p->objective(mmccpSolver.solve(*init_p))/init_p->objective(best_s) ;
    
        if (demo) std::cout << "initial target = " << target << '\n';

        unsigned Gs = 0;
       
        while (t--) {
            std::vector<instance> child ;
            for ( unsigned i = 0 ; i < lambda ; i++ ) {
                child.push_back( evolve(population[dis_mu(gen)]) ) ;
            }

            population.insert(population.end(), child.begin(), child.end()) ;

            std::vector<std::pair<problem::obj_t, unsigned>> order(mu+lambda);
            for ( unsigned i = 0 ; i < order.size() ; i++ ) {
                auto p = ProblemFactory::produce(p_name, generate(population[i]), k);
                order[i] = {-p->objective(mmccpSolver.solve(*p))/p->objective(solv->solve(*p)),i} ;
            }

            std::sort(order.begin(), order.end() ) ;

            std::vector<instance> fittest_population(mu) ;
            for ( unsigned i = 0 ; i < mu ; i++ ) fittest_population[i] = population[order[i].second] ;

            auto p = ProblemFactory::produce(p_name, generate(population[order[0].second]), k);
            problem::obj_t nmx = -order[0].first ;
     
            if (demo) {
                std::cout << "current best is : " << nmx << '\n' ;
                if (nmx > target) std::cout << "better! " << target << '\n';
                else std::cout << "worse... " << target << '\n';
            }
            if (nmx > target) best_ins = population[order[0].second], target = nmx, best_s = solv->solve(*p), ++Gs;
            
            if (t % G == 0) {
                if (5 * Gs > G) sigma_pos /= a, sigma_dat /= a;
                else if (5 * Gs < G) sigma_pos *= a, sigma_dat *= a;
                Gs = 0;
            }

            
            population = fittest_population ;

            out.close() ;
        }

        
        return {ProblemFactory::produce(property("problem"), generate(best_ins), k), {best_s, target} };
    }


    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        std::pair<std::shared_ptr<problem>, solution> best_res ;
        problem::obj_t best_tar = 1.f ;
        for ( unsigned i = 0 ; i < repeat ; i++ ) {
            auto result = run() ;
            std::cout << "best ratio = " << result.second.second << "\n" ;
            if ( result.second.second < best_tar ) 
                best_res = {result.first, result.second.first } ;
        }
        return best_res ;
    }

protected:
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
    float sigma_pos, sigma_dat, a;
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

