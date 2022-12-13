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
    EvolutionStrategy(const std::string& args = ""): DummyConstructor("name=es " + args) {}

public:
    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        // std::cout << "es!\n";
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
};

class InstanceMinDeploy : public DummyConstructor {
public:
    InstanceMinDeploy(const std::string& args = ""): DummyConstructor("name=min-deploy " + args) {
        T = 1;
    }

protected:
    virtual problem::graph_t generate(unsigned n) override {
        n = std::uniform_int_distribution<unsigned>(100, 500)(gen);
        std::vector<std::pair<problem::obj_t, problem::obj_t>> pos;
        std::vector<problem::obj_t> data;
        problem::obj_t trans_rate = 1.f;
        problem::obj_t fly_speed = 10.f;

        std::uniform_real_distribution<problem::obj_t> dis_pos(0, 50000), dis_data(5, 10);
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