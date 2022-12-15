#pragma once

#include <memory>
#include <string>

#include "arg_container.hpp"
#include "problem.hpp"
#include "solver.hpp"
#include "ga_int_prog.hpp"

class ProblemFactory {
public:
    static std::shared_ptr<problem> produce(std::string name, const problem::graph_t& gr = problem::graph_t(), unsigned k = 0, float B = 0 ) {
        if (name == "min-sum")      return std::make_shared<MinSumProblem>(gr, k);
        else if (name == "min-max") return std::make_shared<MinMaxProblem>(gr, k);
        else if ( name == "mccp" )  return std::make_shared<MinCycleProblem>(gr, B) ;
        else                        return std::make_shared<MinSumProblem>(gr, k);
    }
};

class SolverFactory {
public:
    static std::shared_ptr<solver> produce(std::string arg = "") {
        ArgContainer ac(arg);
        std::string name = "dummy";
        if (ac().find("name") != ac().end()) {
            name = static_cast<std::string>(ac()["name"]);
            // ac().erase("name");
        }

        if (name == "dummy")           return std::make_shared<DummySolver>(static_cast<std::string>(ac));
        else if (name == "ga")         return std::make_shared<GeneticAlgorithm>(static_cast<std::string>(ac));
        else if (name == "elitism-ga") return std::make_shared<ElitismGA>(static_cast<std::string>(ac));
        else if (name == "mccp" )      return std::make_shared<MCCPSolver>(static_cast<std::string>(ac));
        else if (name == "min-max")    return std::make_shared<MMCCPSolver>(static_cast<std::string>(ac));
        else if (name == "gaip")       return std::make_shared<GeneAlgoIntProg>(static_cast<std::string>(ac));
        else                           return std::make_shared<DummySolver>(static_cast<std::string>(ac));
    }

};