#pragma once

#include <random>
#include <algorithm>

#include "agent.hpp"
#include "problem.hpp"

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