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
    DummyConstructor(const std::string& args = ""): constructor("name=dummy " + args), solv(SolverFactory::produce(property("solver"))), T(100), n(10), k(3) {
        if (meta.find("T") != meta.end()) T = static_cast<unsigned>(meta["T"]);
        if (meta.find("n") != meta.end()) n = static_cast<unsigned>(meta["n"]);
        if (meta.find("k") != meta.end()) k = static_cast<unsigned>(meta["k"]);
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());
    }

public:
    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        unsigned t = T - 1;

        // MinSumProblem best_p(generate(n), k);
        auto best_p = ProblemFactory::produce(property("problem"), generate(n), k);
        solution best_s = solv->solve(*best_p);
        problem::obj_t mx = best_p->objective(best_s);
        while (t--) {
            auto pro = ProblemFactory::produce(property("problem"), generate(n), k);
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

protected:
    // DummySolver solver;
    std::shared_ptr<solver> solv;
    std::default_random_engine gen;

    unsigned T;
    unsigned n, k; // # of nodes and # of cycles

};

class EvolutionStrategy : public DummyConstructor {
public:
    EvolutionStrategy(const std::string& args = ""): DummyConstructor("name=es " + args), sigma_pos(50), sigma_dat(0.05), a(0.85), G(10) {
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
        if (meta.find("n") == meta.end()) n = std::uniform_int_distribution<unsigned>(100, 500)(gen);

        if (meta.find("demo") != meta.end())  demo = true;
    }
protected:
    using Pos = std::pair<problem::obj_t, problem::obj_t>;
    using Dat = problem::obj_t;
    using instance = std::pair<std::vector<Pos>, std::vector<Dat>>;

public:
    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        // std::cout << "es!\n";
        unsigned t = T - 1;
        std::string p_name = property("problem");

        // MinSumProblem best_p(generate(n), k);
        auto best_ins = initialze();
        auto init_p = ProblemFactory::produce(p_name, generate(best_ins), k);
        solution best_s = solv->solve(*init_p);
        problem::obj_t mx = init_p->objective(best_s);
        if (demo) std::cout << "initial mx = " << mx << '\n';

        unsigned Gs = 0;
        while (t--) {
            auto ch = evolve(best_ins);
            auto p = ProblemFactory::produce(p_name, generate(ch), k);
            solution s = solv->solve(*p);
            // std::cout << "sol: \n";
            // for (auto& cyc : s) {
            //     for (auto v : cyc) 
            //         std::cout << v << ' ';
            //     std::cout << '\n';
            // }
            problem::obj_t nmx = p->objective(s);
            if (demo) {
                if (nmx > mx) std::cout << "better! " << nmx << '\n';
                else std::cout << "worse... " << nmx << '\n';
            }
            if (nmx > mx) best_ins = ch, best_s = s, mx = nmx, ++Gs;
            
            if (t % G == 0) {
                if (5 * Gs > G) sigma_pos /= a, sigma_dat /= a;
                else if (5 * Gs < G) sigma_pos *= a, sigma_dat *= a;
                Gs = 0;
            }
        }
        return {ProblemFactory::produce(property("problem"), generate(best_ins), k), best_s};
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

    float pos_lb = 0.f, pos_ub = 5000.f;
    float data_lb = 5.f, data_ub = 10.f;
    float trans_rate = 1.f;
    float fly_speed = 10.f;

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


class MCCPConstructor : public DummyConstructor {
public:
    MCCPConstructor(const std::string& args = ""): DummyConstructor("name=mccp " + args) {
        T = 1;
    }

protected:
    virtual problem::graph_t generate(unsigned n) override {
    
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