#pragma once

#include <thread>
#include <deque>

#include "constructor.hpp"

class GridSearch : public InstanceMinDeploy {
    /*
        solvers going to search

        name: standard-ga
        selection: random elitism roulette_wheel tournament
        replacement: elitism roulette_wheel tournament

        name: ss-ga
        selection: random elitism roulette_wheel tournament
        no replacement

        crossover: pmx ox cycle edge_recomb
        mutation: insert swap invert scramble
    */
public:
    GridSearch(const std::string& args = ""): InstanceMinDeploy("name=grid-search " + args) {
        if (meta.find("ins_size") != meta.end()) ins_size = static_cast<unsigned>(meta["ins_size"]);
        if (meta.find("thread_size") != meta.end()) thread_size = static_cast<unsigned>(meta["thread_size"]);
        if (meta.find("demo") != meta.end()) demo = true;
        if (meta.find("problem") != meta.end()) pname = meta["problem"].value;
        if (meta.find("dir") != meta.end()) dir = meta["dir"].value;
    }

protected:
    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        std::vector<std::shared_ptr<problem>> inss(ins_size);
        for (unsigned i = 0; i < ins_size; ++i) {
            unsigned n = (ins_size < 2)? 100 : 100 + i * (500 - 100) / (ins_size - 1);
            inss[i] = ProblemFactory::produce(pname, generate(n), k);
        }
        std::deque<std::thread> thrs;
        auto args = solver_args();
        for (unsigned i = 0; i < args.size(); ++i) {
            for (unsigned j = 0; j < ins_size; ++j) {
                auto ac = args[i];
                ac()["save"].value += "_" + std::to_string(j) + ".txt";
                std::string arg = static_cast<std::string>(ac);
                auto solv = SolverFactory::produce(arg);
                while (thrs.size() >= thread_size) {
                    thrs.front().join();
                    thrs.pop_front();
                }
                if (demo) std::cout << arg << '\n';
                thrs.push_back(std::thread(&solver::solve, solv, std::cref(*inss[j])));
            }
        }

        while (thrs.size()) {
            thrs.front().join();
            thrs.pop_front();
        }

        return {nullptr, solution()};
    }

    std::vector<ArgContainer> solver_args() const {
        ArgContainer ac;
        std::vector<ArgContainer> re;
        std::vector<std::string> selections = {"elitism", "tournament", "random", "roulette_wheel"};
        std::vector<std::string> replacements = {"elitism"};
        std::vector<std::string> crossovers = {"cycle", "edge_recomb", "pmx", "ox"};
        std::vector<std::string> mutations = {"insert", "inverse", "scramble", "swap"};
        
        ac.insert("m", "100");
        ac.insert("T", "50000");
        ac.insert("block", "500");
        ac.insert("parent_ratio", "0.8");
        ac.insert("mutation_rate", "0.2");

        // ac.insert("name", "ss-ga");
        // std::string s = "tournament", c = "edge_recomb", m = "scramble";
        // ac.insert("selection", s);
        // ac.insert("crossover", c);
        // ac.insert("mutation", m);
        // ac.insert("save", "data/ss/" + s + "_" + c + "_" + m);
        // re.push_back(ac);
        // return re;

        // for (auto& s : selections) for (auto& c : crossovers) for (auto& m : mutations) {
        //     ac.insert("selection", s);
        //     ac.insert("crossover", c);
        //     ac.insert("mutation", m);
        //     ac.insert("save", dir + "/ss/" + s + "_" + c + "_" + m);
        //     re.push_back(ac);
        // }

        ac.insert("name", "tabu-ga");
        for (auto& s : selections) for (auto& r : replacements) for (auto& c : crossovers) for (auto& m : mutations) {
            ac.insert("selection", s);
            ac.insert("replacement", r);
            ac.insert("crossover", c);
            ac.insert("mutation", m);
            ac.insert("save", dir + "/tabu2/" + s + "_" + r + "_" + c + "_" + m);
            re.push_back(ac);
        }
        return re;
    }

protected:
    unsigned ins_size = 10;
    unsigned thread_size = 10;
    bool demo = false;

    std::string pname = "min-max";
    std::string dir = "data/min-max";
};