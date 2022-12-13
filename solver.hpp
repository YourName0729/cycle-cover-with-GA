#pragma once

#include <random>
#include <algorithm>
#include <array>

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

class GeneticAlgorithm : public solver {
public:
    GeneticAlgorithm(const std::string& args = "") : solver(args + "name=ga"), m(6), T(500) {
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());
        if (meta.find("m") != meta.end()) m = static_cast<unsigned>(meta["m"]);
        if (meta.find("T") != meta.end()) T = static_cast<unsigned>(meta["T"]);
    }

protected:
    using gene = std::vector<unsigned>;

protected:
    gene random_gene() {
        std::uniform_int_distribution<> dis(0, n - 1);
        gene re;
        for (unsigned i = 0; i < n; ++i) {
            re.push_back(i);
            unsigned idx = std::uniform_int_distribution<unsigned>(0, i)(gen);
            std::swap(re[i], re[idx]);
        }
        // for (unsigned i = 0; i < n; ++i) re.push_back(dis(gen));
        for (unsigned i = n; i < len; ++i) {
            re.push_back(i - n);
        }
        return re;
    }

    virtual std::vector<gene> initialize_pool() {
        std::vector<gene> pool(m);
        for (unsigned i = 0; i < m; ++i) {
            pool[i] = random_gene();
        }
        return pool;
    }

    solution decode(const gene& gn) const {
        auto g = gn;
        std::sort(g.begin() + n, g.end());
        solution sol(len - n + 1);
        unsigned l = 0;
        for (unsigned i = n; i < len; ++i) {
            for (unsigned j = l; j < g[i]; ++j) {
                sol[i - n].push_back(g[j]);
            }
            l = g[i];
        }
        for (unsigned j = l; j < n; ++j) {
            sol.back().push_back(g[j]);
        }
        return sol;
    }

    virtual std::vector<gene> selection(const std::vector<gene>& org, const problem& ins) {
        std::vector<problem::obj_t> score;
        problem::obj_t sum = 0;
        for (auto& g : org) {
            // score.push_back(std::exp(-ins.objective(decode(g))));
            // std::cout << ins.objective(decode(g)) << '\n';
            score.push_back(1 / ins.objective(decode(g)));
            sum += score.back();
            // std::cout << score.back() << ' ';
        }
        // std::cout << " sum " << sum << '\n';

        std::uniform_real_distribution<problem::obj_t> dis(0, sum);
        std::vector<problem::obj_t> trials;
        for (unsigned i = 0; i < org.size(); ++i) {
            trials.push_back(dis(gen));
        }
        std::sort(trials.begin(), trials.end());

        std::vector<gene> re;
        problem::obj_t acc = 0;
        unsigned l = 0;
        for (unsigned i = 0; i < trials.size(); ++i) {
            while (acc + score[l] < trials[i]) acc += score[l++];
            re.push_back(org[i]);
        }

        std::shuffle(re.begin(), re.end(), gen);
        return re;
    }

    virtual std::vector<gene> crossover(const std::vector<gene>& pool) {
        std::vector<gene> re;
        for (unsigned i = 0; i < pool.size(); i += 2) {
            // std::cout << "-> " << i << ' ' << pool.size() << '\n';
            // for (auto g : pool) {
            //     for (auto v : g) {
            //         std::cout << v << ' ';
            //     }
            //     std::cout << '\n';
            // }
            // cycle crossover
            gene ch[2] = {gene(n), gene(n)};
            std::vector<unsigned> vst(len, 0);

            std::vector<unsigned> inv(n);
            for (unsigned j = 0; j < n; ++j) {
                inv[pool[i][j]] = j;
            }

            unsigned alt = 1;
            for (unsigned j = 0; j < n; ++j) {
                if (!vst[j]) alt ^= 1u;
                while (!vst[j]) {
                    ch[alt][j] = pool[i][j], ch[alt ^ 1u][j] = pool[i + 1][j];
                    vst[j] = 1;
                    j = inv[pool[i + 1][j]];
                }
            }

            std::vector<unsigned> cuts;
            for (unsigned j = n; j < len; ++j) {
                cuts.push_back(pool[i][j]);
                cuts.push_back(pool[i + 1][j]);
            }
            std::shuffle(cuts.begin(), cuts.end(), gen);


            std::fill(vst.begin(), vst.end(), false);
            for (unsigned j = 0; j < cuts.size() && ch[0].size() < len; ++j) {
                if (vst[cuts[j]]) continue;
                vst[cuts[j]] = true;
                ch[0].push_back(cuts[j]);
            }

            std::fill(vst.begin(), vst.end(), 0);
            std::reverse(cuts.begin(), cuts.end());
            for (unsigned j = 0; j < cuts.size() && ch[1].size() < len; ++j) {
                if (vst[cuts[j]]) continue;
                vst[cuts[j]] = true;
                ch[1].push_back(cuts[j]);
            }

            re.push_back(ch[0]);
            re.push_back(ch[1]);
        }

        std::shuffle(re.begin(), re.end(), gen);
        return re;
    }

    virtual std::vector<gene> mutation(const std::vector<gene>& pool) {
        
        std::vector<gene> re;
        std::uniform_int_distribution<unsigned> dis2n(0, n - 1);
        std::uniform_int_distribution<unsigned> dis2nm1(0, n - 2);

        std::uniform_int_distribution<unsigned> dis2k(0, len - n - 1);
        std::uniform_int_distribution<unsigned> dis2nmk(0, n - (len - n) - 1);
        for (auto& ge : pool) {
            re.push_back(ge);
            auto& g = re.back();

            // swap mutaion
            auto x = dis2n(gen), y = (x + dis2nm1(gen)) % n;
            std::swap(g[x], g[y]);

            // choose one cut and change it
            unsigned idx = dis2k(gen); // index to change
            std::swap(g[n], g[n + idx]);
            std::sort(g.begin() + n + 1, g.end());
            unsigned cut_idx = dis2nmk(gen);
            for (unsigned j = n + 1; j < len && g[j] <= cut_idx; ++j, ++cut_idx);
            g[n] = cut_idx;
        }

        return re;        
    }

public:
    virtual solution solve(const problem& ins) override {
        std::cout << "solve\n";
        n = ins().size();
        len = n + ins.get_k() - 1;

        auto pool = initialize_pool();
        std::cout << "---\n";
        auto best_gene = pool[0];
        problem::obj_t best_score = ins.objective(decode(best_gene));

        unsigned t = T;
        while (t--) {
            // std::cout << "t = " << t << '\n';
            // std::cout << "selection\n";
            pool = selection(pool, ins);
            // std::cout << "mutation\n";
            pool = mutation(pool);
            // std::cout << "crossover\n";
            pool = crossover(pool);

        
            // for (auto g : pool) {
            //     for (auto v : g) {
            //         std::cout << v << ' ';
            //     }
            //     std::cout << " = " << ins.objective(decode(g)) << '\n';
            // }

            for (auto& g : pool) {
                problem::obj_t nobj = ins.objective(decode(g));
                if (nobj < best_score) best_gene = g, best_score = nobj;
            }
        }

        return decode(best_gene);
    }

private:
    unsigned m; // size of gene pool
    unsigned T; // # of iterations


    unsigned n; // # of nodes
    unsigned len; // length of a gene <permutation of # of nodes, k-1 cut on # of nodes>

    std::default_random_engine gen;
};