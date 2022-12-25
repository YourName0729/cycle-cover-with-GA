#pragma once

#include "constructor.hpp"

class Comparison : public constructor {
public:
    Comparison(const std::string& args = ""): constructor( args+ " name=comparison") {
        if (meta.find("T") != meta.end()) T = static_cast<unsigned>(meta["T"]);
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());
        if (meta.find("demo") != meta.end()) demo = true;
        assign("n_lb", n_lb);
        assign("n_ub", n_ub);
    }

public:
    using Pos = std::pair<problem::obj_t, problem::obj_t>;
    using Dat = problem::obj_t;
    struct instance {
        std::vector<Pos> pos;
        std::vector<Dat> dat;
    };

protected:
    template<typename T>
    void assign(const std::string& name, T& s) {
        if (meta.find(name) != meta.end()) s = static_cast<T>(meta[name]);
    }

    instance initialze(unsigned n) {
        std::vector<Pos> pos;
        std::vector<Dat> data;
        std::uniform_real_distribution<problem::obj_t> dis_pos(pos_lb, pos_ub), dis_data(data_lb, data_ub);
        for (unsigned i = 0; i < n; ++i) {
            pos.push_back({dis_pos(gen), dis_pos(gen)});
            data.push_back(dis_data(gen));
        }
        return instance{pos, data};
    }

    problem::graph_t generate(const instance& ins) const {
        const auto& [pos, dat] = ins;
        unsigned n = dat.size();
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

public:
    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        std::string arg1 = "name=tabu-ga selection=elitism crossover=edge_recomb mutation=insert m=100 T=30000";
        std::string arg2 = "name=min-max demo=0";
        assign("solver1", arg1);
        assign("solver2", arg2);
        std::vector<std::shared_ptr<solver>> solv = {SolverFactory::produce(arg1), SolverFactory::produce(arg2)};

        std::string fname = "data/comp/tabu-ga min-max.txt";
        assign("save", fname);
        std::ofstream fout;
        if (fname.size()) fout.open(fname);

        std::shared_ptr<problem> ins;
        solution sol1, sol2;

        for (unsigned i = 0; i < linspace; ++i) {
            for (unsigned j = 0; j < T; ++j) {
                unsigned n = (n_ub - n_lb) * i / (linspace - 1) + n_lb;
                if (demo) std::cout << i << ' ' << j << '\n';
                ins = ProblemFactory::produce("min-max", generate(initialze(n)), k);
                if (demo) std::cout << "solve 1\n";
                sol1 = solv[0]->solve(*ins);
                if (demo) std::cout << "solve 2\n";
                sol2 = solv[1]->solve(*ins);
                
                fout << "T=" << j << ' ';
                fout << "n=" << n << ' ';
                fout << "obj1=" << ins->objective(sol1) << ' ';
                fout << "obj2=" << ins->objective(sol2) << ' ';
                fout << std::endl;
            }
        }

        if (fname.size()) fout.close();
        return {ins, sol1};
    }

protected:
    unsigned T = 10;
    unsigned linspace = 10;
    std::default_random_engine gen;
    bool demo = false;

    unsigned k = 30;

    unsigned n_lb = 100, n_ub = 500;

    float pos_lb = 0.f, pos_ub = 5000.f;
    float data_lb = 5.f, data_ub = 10.f;

    float trans_rate = 1.f;
    float fly_speed = 10.f;
};