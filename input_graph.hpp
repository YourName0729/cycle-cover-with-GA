#pragma once

#include <thread>

#include "constructor.hpp"

class InputGraph : public constructor {
    /*
        load: location of the graph file
        save: location of save solution
        solver: parameter for the solver to solve
    */
public:
    InputGraph(const std::string& args = "") : constructor(args + " name=input") {
        if (meta.find("load") != meta.end()) load = meta["load"].value;
        if (meta.find("thread_size") != meta.end()) thread_size = static_cast<unsigned>(meta["thread_size"]);
    }

    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        std::ifstream fin(load);
        instance ins;
        fin >> ins;
        auto pro = ProblemFactory::produce(pname, generate(ins), ins.k);

        std::vector<std::shared_ptr<solver>> solvs;
        for (unsigned i = 0; i < thread_size; ++i) solvs.push_back(SolverFactory::produce(meta["solver"].value));

        std::vector<solution> sols(thread_size);

        auto solve = [](std::shared_ptr<solver> solv, const problem& ins, solution& sol) {
            sol = solv->solve(ins);
        };

        std::vector<std::thread> vec;
        for (unsigned i = 0; i < thread_size; ++i) {
            vec.push_back(std::thread(solve, solvs[i], std::cref(*pro), std::ref(sols[i])));
        }
        for (unsigned i = 0; i < thread_size; ++i) vec[i].join();

        solution best = sols[0];
        for (unsigned i = 0; i < thread_size; ++i)
            if (pro->objective(sols[i]) < pro->objective(best)) best = sols[i];

        // auto sol = solv->solve(*pro);
        if (meta.find("save") != meta.end()) {
            std::ofstream fout(meta["save"].value);
            fout << best;
            fout.close();
        }
        return {pro, best};
    }

protected:
    using Pos = std::pair<problem::obj_t, problem::obj_t>;
    using Dat = problem::obj_t;
    struct instance {
        unsigned k;
        std::vector<Pos> pos;
        std::vector<Dat> dat;

        friend std::istream& operator>>(std::istream& in, instance& ins) {
            unsigned n;
            in >> n >> ins.k;
            for (unsigned i = 0; i < n; ++i) {
                float x, y;
                in >> x >> y;
                ins.pos.push_back({x, y});
            }
            for (unsigned i = 0; i < n; ++i) {
                float d;
                in >> d;
                ins.dat.push_back(d);
            }
            return in;
        }

        friend std::ostream& operator<<(std::ostream& out, const instance& ins) {
            for (auto& [x, y] : ins.pos) out << x << ' ' << y << '\n';
            for (auto v : ins.dat) out << v << ' ';
            return out;
        }
    };

protected:
    problem::graph_t generate(const instance& ins) const {
        const auto& [k, pos, dat] = ins;
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
protected:
    std::string load;
    std::string pname = "min-max";
    unsigned thread_size = 10;

    float pos_lb = 0.f, pos_ub = 5000.f;
    float data_lb = 5.f, data_ub = 10.f;
    float trans_rate = 1.f;
    float fly_speed = 10.f;
};