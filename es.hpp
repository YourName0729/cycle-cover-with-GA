#pragma once

#include "constructor.hpp"
#include "thread"

class ES : public constructor {
    /*
        Maximize solver 1 / solver 2
    */
public:
    ES(const std::string& args = "") : constructor(args + " name=ES") {
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
        // if (meta.find("mu") != meta.end()) mu = static_cast<float>(meta["mu"]);
        // if (meta.find("lambda") != meta.end())  lambda = static_cast<float>(meta["lambda"]);
        if (meta.find("n") != meta.end()) n = static_cast<unsigned>(meta["n"]);
        if (meta.find("k") != meta.end()) k = static_cast<unsigned>(meta["k"]);
        if (meta.find("demo") != meta.end())  demo = true;

        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());

        if (meta.find("save_graph") != meta.end()) save_graph = meta["save_graph"].value;
        if (meta.find("save") != meta.end()) save = meta["save"].value;
    }

protected:
    using Pos = std::pair<problem::obj_t, problem::obj_t>;
    using Dat = problem::obj_t;
    struct instance {
        std::vector<Pos> pos;
        std::vector<Dat> dat;

        friend std::ostream& operator<<(std::ostream& out, const instance& ins) {
            for (auto& [x, y] : ins.pos) out << x << ' ' << y << '\n';
            for (auto v : ins.dat) out << v << ' ';
            return out;
        }
    };

protected:
    virtual std::pair<std::shared_ptr<problem>, solution> construct() override {
        if (demo) std::cout << "ES\n";
        std::shared_ptr<solver> solv1p = SolverFactory::produce(solv1_arg);
        std::shared_ptr<solver> solv1c = SolverFactory::produce(solv1_arg);
        std::shared_ptr<solver> solv2p = SolverFactory::produce(solv2_arg);
        std::shared_ptr<solver> solv2c = SolverFactory::produce(solv2_arg);

        std::ofstream finfo;
        if (save.size()) finfo.open(save);
        if (demo) std::cout << "save " << save << ", " << save_graph << '\n';
        
        instance best = initialze(n);
        problem::obj_t obj2p_acc = 0;
        unsigned cnt2p_acc = 0;

        unsigned t = T - 1;
        unsigned Gs = 0;
        while (t--) {
            auto ins = ProblemFactory::produce(pname, generate(best), k);
            auto ch = evolve(best);
            auto insc = ProblemFactory::produce(pname, generate(ch), k);

            auto solve = [](std::shared_ptr<solver> solv, const problem& ins, solution& sol) {
                sol = solv->solve(ins);
            };

            std::cout << "solve\n";
            solution sol1p, sol2p, sol1c, sol2c;
            // std::vector<thread> thrs = {
            //     std::thread(solve, solv1p, std::cref(*ins), std::ref(sol1p)),
            //     std::thread(solve, solv2p, std::cref(*ins), std::ref(sol2p)),
            //     std::thread(solve, solv1c, std::cref(*insc), std::ref(sol1c)),
            //     std::thread(solve, solv2c, std::cref(*insc), std::ref(sol2c))
            // };
            std::vector<thread> thrs;
            thrs.push_back(std::thread(solve, solv1p, std::cref(*ins), std::ref(sol1p)));
            thrs.push_back(std::thread(solve, solv2p, std::cref(*ins), std::ref(sol2p)));
            thrs.push_back(std::thread(solve, solv1c, std::cref(*insc), std::ref(sol1c)));
            thrs.push_back(std::thread(solve, solv2c, std::cref(*insc), std::ref(sol2c)));

            for (auto& t : thrs) t.join();

            // if (demo) std::cout << "calc sol1p" << '\n';
            // auto sol1p = solv1->solve(*ins);
            // if (demo) std::cout << "calc sol2p" << '\n';
            // auto sol2p = solv2->solve(*ins);
            // if (demo) std::cout << "calc sol1c" << '\n';
            // auto sol1c = solv1->solve(*insc);
            // if (demo) std::cout << "calc sol2c" << '\n';
            // auto sol2c = solv2->solve(*insc);

            auto obj1p = ins->objective(sol1p);
            auto obj2p = ins->objective(sol2p);
            auto obj1c = insc->objective(sol1c);
            auto obj2c = insc->objective(sol2c);

            obj2p_acc += obj2p;
            ++cnt2p_acc;

            problem::obj_t fitp = obj1p / (obj2p_acc / cnt2p_acc);
            problem::obj_t fitc = obj1c / obj2c;

            if (demo) {
                std::cout << "Obj parent:\t" << obj1p << "\t/ " << (obj2p_acc / cnt2p_acc) << '\n';
                std::cout << "Obj child:\t" << obj1c << "\t/ " << obj2c << '\n';
            }

            if (fitc > fitp) best = ch, ++Gs, cnt2p_acc = 1, obj2p_acc = obj2c;
            if (t % G == 0) {
                // if (5 * Gs > 3 * G) sigma /= a, update_sigma();
                // else if (5 * Gs < 3 * G) sigma *= a, update_sigma();
                if (demo) std::cout << "Gs=" << Gs << ' ' << "Sigma=" << sigma << '\n';

                if (5 * Gs > G) sigma /= a, update_sigma();
                else if (5 * Gs < G) sigma *= a, update_sigma();
                Gs = 0;

                
            }

            if (t % block == 0) {
                if (save_graph.size()) {
                    std::ofstream fout(save_graph + std::to_string(T - t) + ".txt");
                    fout << n << ' ' << k << '\n';
                    fout << best << '\n';
                    fout.close();
                }
            }

            finfo << "T=" << T - t << ' ';
            finfo << "Gs=" << Gs << ' ';
            finfo << "sigma=" << sigma << ' ';
            if (fitc > fitp) finfo << "obj1=" << obj1c << ' ' << "obj2=" << obj2c << std::endl;
            else             finfo << "obj1=" << obj1p << ' ' << "obj2=" << (obj2p_acc / cnt2p_acc) << std::endl;

            if (demo) std::cout << t << ' ' << sigma << ' ' << Gs << ' ' << obj1p << ' ' << obj2p << '\n';
        }

        if (save.size()) finfo.close();

        return {nullptr, solution()};
    }

protected:
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

    void update_sigma() { sigma_pos = 5000 * sigma, sigma_dat = 5 * sigma; }

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

protected:
    std::string solv1_arg = "name=min-max demo=0";
    std::string solv2_arg = "name=tabu-ga selection=roulette_wheel replacement=elitism crossover=edge_recomb mutation=swap parent_ratio=0.8 mutation_rate=0.2 m=100 T=50000";
    std::string pname = "min-max";

    std::string save_graph = "./data/es2/graph/";
    std::string save = "./data/es2/es.txt";
    unsigned block = 10;

    float sigma_pos = 5000, sigma_dat = 5, sigma = 0.2, a = 0.85;
    unsigned G = 10;
    // unsigned mu = 1, lambda = 1;

    float pos_lb = 0.f, pos_ub = 5000.f;
    float data_lb = 5.f, data_ub = 10.f;
    float trans_rate = 1.f;
    float fly_speed = 10.f;

    bool demo = false;

    unsigned T = 202;
    unsigned n = 100, k = 30; // # of nodes and # of cycles

    std::default_random_engine gen;
};