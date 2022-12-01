#pragma once

#include "solver.hpp"

class constructor : public agent {
public:
    constructor(const std::string& args = ""): agent(args + " role=constructor") {}

public:
    virtual std::pair<problem, solution> construct() = 0;
};

class DummyConstructor : public constructor {
public:
    DummyConstructor(const std::string& args = ""): constructor(args + " name=dummy"), solver(property("solver")), T(100) {
        if (meta.find("T") != meta.end()) T = static_cast<unsigned>(meta["T"]);
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());
    }

public:
    virtual std::pair<problem, solution> construct() override {
        unsigned t = T;

        unsigned n = 10, k = 3;

        MinSumProblem best_p(generate(n), k);
        solution best_s = solver.solve(best_p);
        problem::obj_t mx = best_p.objective(best_s);
        while (t--) {
            MinSumProblem pro(generate(n), k);
            solution sol = solver.solve(pro);
            problem::obj_t nmx = pro.objective(sol);
            if (nmx > mx) best_p = pro, best_s = sol, mx = nmx;
        }
        return {best_p, best_s};
    }

private:
    problem::graph_t generate(unsigned n) {
        problem::graph_t g(n);
        std::uniform_real_distribution<problem::obj_t> dis(1, 10);
        for (unsigned i = 0; i < n; ++i) {
            for (unsigned j = 0; j < n; ++j) {
                g(i, j) = i == j? 0 : dis(gen);
            }
        }
        return g;
    }

private:
    DummySolver solver;
    std::default_random_engine gen;

    unsigned T;

};