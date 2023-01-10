#pragma once

#include <memory>
#include <functional>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "solver.hpp"

class GeneticAlgorithm : public solver {
public:
    GeneticAlgorithm(const std::string& args = "") : solver("name=ga" + args) {
        if (meta.find("seed") != meta.end()) gen.seed(static_cast<unsigned int>(meta["seed"]));
        else gen.seed(std::random_device()());
        if (meta.find("demo") != meta.end()) demo = true;
    
        if (meta.find("selection") != meta.end()) {
            std::string name = meta["selection"];
            if      (name == "dummy")          selection = std::bind(&GeneticAlgorithm::selection_dummy         , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "random")         selection = std::bind(&GeneticAlgorithm::selection_random        , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "elitism")        selection = std::bind(&GeneticAlgorithm::selection_elitism       , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "roulette_wheel") selection = std::bind(&GeneticAlgorithm::selection_roulette_wheel, this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "tournament")     selection = std::bind(&GeneticAlgorithm::selection_tournament    , this, std::placeholders::_1, std::placeholders::_2);
            else                               selection = std::bind(&GeneticAlgorithm::selection_dummy         , this, std::placeholders::_1, std::placeholders::_2);
        }

        if (meta.find("replacement") != meta.end()) {
            std::string name = meta["replacement"];
            if      (name == "dummy")          replacement = std::bind(&GeneticAlgorithm::selection_dummy         , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "random")         replacement = std::bind(&GeneticAlgorithm::selection_random        , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "elitism")        replacement = std::bind(&GeneticAlgorithm::selection_elitism       , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "roulette_wheel") replacement = std::bind(&GeneticAlgorithm::selection_roulette_wheel, this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "tournament")     replacement = std::bind(&GeneticAlgorithm::selection_tournament    , this, std::placeholders::_1, std::placeholders::_2);
            else                               replacement = std::bind(&GeneticAlgorithm::selection_dummy         , this, std::placeholders::_1, std::placeholders::_2);
        }

        if (meta.find("crossover") != meta.end()) {
            std::string name = meta["crossover"];
            if      (name == "dummy")       crossover = std::bind(&GeneticAlgorithm::crossover_dummy      , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "pmx")         crossover = std::bind(&GeneticAlgorithm::crossover_pmx        , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "ox")          crossover = std::bind(&GeneticAlgorithm::crossover_ox         , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "cycle")       crossover = std::bind(&GeneticAlgorithm::crossover_cycle      , this, std::placeholders::_1, std::placeholders::_2);
            else if (name == "edge_recomb") crossover = std::bind(&GeneticAlgorithm::crossover_edge_recomb, this, std::placeholders::_1, std::placeholders::_2);
            else                            crossover = std::bind(&GeneticAlgorithm::crossover_dummy      , this, std::placeholders::_1, std::placeholders::_2);
        }

        if (meta.find("mutation") != meta.end()) {
            std::string name = meta["mutation"];
            if      (name == "dummy")    mutation = std::bind(&GeneticAlgorithm::mutation_dummy   , this, std::placeholders::_1);
            else if (name == "insert")   mutation = std::bind(&GeneticAlgorithm::mutation_insert  , this, std::placeholders::_1);
            else if (name == "swap")     mutation = std::bind(&GeneticAlgorithm::mutation_swap    , this, std::placeholders::_1);
            else if (name == "inverse")  mutation = std::bind(&GeneticAlgorithm::mutation_inverse , this, std::placeholders::_1);
            else if (name == "scramble") mutation = std::bind(&GeneticAlgorithm::mutation_scramble, this, std::placeholders::_1);
            else                         mutation = std::bind(&GeneticAlgorithm::mutation_dummy   , this, std::placeholders::_1);
        }

        assign("m", m);
        assign("T", T);
        assign("repeat", repeat);
        assign("parent_ratio", parent_ratio);
        assign("mutation_rate", mutation_rate);
        assign("tournament_k", tournament_k);
        assign("tournament_p", tournament_p);
        assign("debug", debug);
    }

protected:
    class chromosome : public std::vector<unsigned> {
    public:
        using std::vector<unsigned>::vector;

        friend std::ostream& operator<<(std::ostream& out, const GeneticAlgorithm::chromosome& chr) {
            for (auto v : chr) out << v << ' ';
            return out;
        }
    };
    // using chromosome = std::vector<unsigned>;
    using population = std::vector<chromosome>;

    using func_selection = std::function<std::vector<unsigned>(population&, unsigned)>;
    using func_crossover = std::function<void(chromosome&, chromosome&)>;
    using func_mutation = std::function<void(chromosome&)>;

protected:
    // helper functions
    template<typename T>
    void assign(const std::string& name, T& s) {
        if (meta.find(name) != meta.end()) s = static_cast<T>(meta[name]);
    }

    unsigned get_n() const { return (*ins)().size(); }
    unsigned get_len() const { return get_n() + (*ins).get_k() - 1; }
    problem::obj_t fitness(const chromosome& chr) const { return -(*ins).objective(decode(chr)); }

    virtual population initialize_pool() {
        auto random_chromosome = [&]() {
            chromosome chr;
            unsigned len = get_len();
            for (unsigned i = 0; i < len; ++i) {
                chr.push_back(i);
            }
            std::shuffle(chr.begin(), chr.end(), gen);
            return chr;
        };
        
        population ppl;
        for (unsigned i = 0; i < m; ++i) {
            ppl.emplace_back(random_chromosome());
        }
        return ppl;
    }

    solution decode(const chromosome& chr) const {
        solution sol(1);
        unsigned n = get_n();
        for (auto v : chr) {
            if (v >= n) sol.push_back(tour());
            else        sol.back().push_back(v);
        }
        return sol;
    }

protected:
    virtual void generation(population& pool, chromosome& best_chr, problem::obj_t& best_fit, unsigned parent_size) {}

public:
    virtual solution solve(const problem& ins) {
        this->ins = &ins;

        solution best = solve_single(ins);
        unsigned r = repeat - 1;
        while (r--) {
            auto sol = solve_single(ins);
            if (ins.objective(sol) < ins.objective(best)) best = sol;
        }
        return best;
    }

protected:
    virtual solution solve_single(const problem& ins) {
        auto pool = initialize_pool();
        unsigned parent_size = m * parent_ratio;
        pool.reserve(m + parent_size);

        auto best_chr = pool[0];
        problem::obj_t best_fit = fitness(best_chr);

        // record stats
        bool record = false;
        if (meta.find("save") != meta.end()) record = true;
        std::ofstream fout;
        unsigned block = T;
        std::chrono::steady_clock::time_point begin;
        if (record) {
            fout.open(meta["save"]);
            fout << std::fixed;
            begin = std::chrono::steady_clock::now();
            block = 1000;
            assign("block", block);
        }

        auto update_best = [&](unsigned t) {
            if (t % block == 0) {
                problem::obj_t mx = -1e9, sm = 0, mn = 1e9;
                for (auto& chr : pool) {
                    problem::obj_t nfit = fitness(chr);
                    if (record) mx = std::max(mx, nfit), sm += nfit, mn = std::min(mn, nfit);
                    if (best_fit < nfit) best_chr = chr, best_fit = nfit;
                }

                auto end = std::chrono::steady_clock::now();

                fout << "T=" << T - t << ' ';
                fout << "t=" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << ' ';
                fout << "best=" << best_fit << ' ';
                fout << "mn=" << mn << ' ';
                fout << "avg=" << sm / m << ' ';
                fout << "mx=" << mx << '\n';
            }
            else {
                for (auto& chr : pool) {
                    problem::obj_t nfit = fitness(chr);
                    if (best_fit < nfit) best_chr = chr, best_fit = nfit;
                }
            }
            if (debug) {
                std::cout << T - t << '\n';
                for (auto& chr : pool) {
                    std::cout << chr << ": " << fitness(chr) << '\n';
                }
            }
        };

        update_best(T);
        unsigned t = T - 1;
        while (t--) {
            generation(pool, best_chr, best_fit, parent_size);

            update_best(t);
        }

        if (record) fout.close();

        return decode(best_chr);
    }

protected:
    // selections
    // return the index of selected chromosomes, of size k

    std::vector<unsigned> selection_dummy(population& ppl, unsigned k) {
        std::vector<unsigned> re(k);
        for (unsigned i = 0; i < k; ++i) re[i] = i % ppl.size();
        return re;
    }

    std::vector<unsigned> selection_random(population& ppl, unsigned k) {
        std::vector<unsigned> re(k);
        std::uniform_int_distribution<unsigned> dis(0, ppl.size() - 1);
        for (unsigned i = 0; i < k; ++i) {
            re[i] = dis(gen);
        }
        return re;
    }

    std::vector<unsigned> selection_elitism(population& ppl, unsigned k) {
        std::vector<std::pair<problem::obj_t, unsigned>> order(ppl.size());
        for (unsigned i = 0; i < ppl.size(); ++i) order[i] = {-fitness(ppl[i]), i};
        std::partial_sort(order.begin(), order.begin() + k, order.end());

        std::vector<unsigned> re(k);
        for (unsigned i = 0; i < k; ++i) re[i] = order[i].second;
        return re;
    }

    std::vector<unsigned> selection_roulette_wheel(population& ppl, unsigned k) {
        std::vector<std::pair<problem::obj_t, unsigned>> order(ppl.size());
        for (unsigned i = 0; i < ppl.size(); ++i) order[i] = {fitness(ppl[i]), i};
        problem::obj_t mn = 1e9, sm = 0;
        for (auto& [fit, idx] : order) mn = std::min(mn, fit);
        for (auto& [fit, idx] : order) fit -= mn, sm += fit;

        std::uniform_real_distribution<problem::obj_t> dis(0, sm);
        std::vector<problem::obj_t> samp(k);
        for (auto& v : samp) v = dis(gen);
        std::sort(samp.begin(), samp.end());

        std::vector<unsigned> re;
        problem::obj_t pfx = 0;
        unsigned idx = 0;
        for (unsigned i = 0; i < samp.size(); ++i) {
            while (pfx + order[idx].first < samp[i]) pfx += order[idx++].first;
            re.push_back(order[idx].second);
        }

        return re;
    }

    std::vector<unsigned> selection_tournament(population& ppl, unsigned k) {
        std::vector<problem::obj_t> fit(ppl.size());
        for (unsigned i = 0; i < ppl.size(); ++i) fit[i] = fitness(ppl[i]);

        std::uniform_int_distribution<unsigned> dis(0, ppl.size() - 1);
        std::uniform_real_distribution<float> dis01(0, 1);
        auto choose1 = [&]() {
            std::vector<unsigned> picks(tournament_k);
            for (unsigned i = 0; i < tournament_k; ++i) picks[i] = dis(gen);
            std::sort(picks.begin(), picks.end(), [&](unsigned a, unsigned b) {
                return fit[a] > fit[b];
            });

            for (unsigned i = 0; i < picks.size() - 1; ++i) {
                if (dis01(gen) < tournament_p) return picks[i];
            }
            return picks.back();
        };

        std::vector<unsigned> re(k);
        for (unsigned i = 0; i < k; ++i) re[i] = choose1();
        return re;
    }

protected:
    // crossovers

    void crossover_dummy(chromosome& a, chromosome& b) {}

    void crossover_pmx(chromosome& a, chromosome& b) {
        std::uniform_int_distribution<unsigned> dis(0, a.size() - 1);
        unsigned l = dis(gen), r = dis(gen);
        if (l > r) std::swap(l, r);

        auto fill_rest = [&](chromosome& chr, const chromosome& oth, const chromosome& invoth) {
            std::vector<bool> vst(chr.size(), false);
            for (unsigned i = l; i <= r; ++i) vst[chr[i]] = true;
            for (unsigned i = l; i <= r; ++i) {
                if (vst[oth[i]]) continue;
                unsigned cur = i;
                while (l <= cur && cur <= r) cur = invoth[chr[cur]];
                chr[cur] = oth[i];
                vst[oth[i]] = true;
            }
            for (unsigned i = 0; i < chr.size(); ++i) if (!vst[oth[i]]) chr[i] = oth[i];
        };

        chromosome acopy = a;
        chromosome inva(a.size()), invb(b.size());
        for (unsigned i = 0; i < a.size(); ++i) inva[a[i]] = invb[b[i]] = i;
        fill_rest(a, b, invb);
        fill_rest(b, acopy, inva);
    }

    void crossover_ox(chromosome& a, chromosome& b) {
        std::uniform_int_distribution<unsigned> dis(0, a.size() - 1);
        unsigned l = dis(gen), r = dis(gen);
        if (l > r) std::swap(l, r);
        
        auto fill_rest = [&](chromosome& chr, const chromosome& proto) {
            std::vector<bool> vst(chr.size(), false);
            for (unsigned i = l; i <= r; ++i) vst[chr[i]] = true;

            unsigned ip = 0;
            for (unsigned i = 0; i < l; ++i) {
                while (vst[proto[ip]]) ++ip;
                chr[i] = proto[ip++];
            }
            for (unsigned i = r + 1; i < chr.size(); ++i) {
                while (vst[proto[ip]]) ++ip;
                chr[i] = proto[ip++];
            }
        };

        chromosome acopy = a;
        fill_rest(a, b);
        fill_rest(b, acopy);
    }

    void crossover_cycle(chromosome& a, chromosome& b) {
        chromosome inva(a.size());
        for (unsigned i = 0; i < a.size(); ++i) inva[a[i]] = i;

        bool alt = false;
        std::vector<bool> vst(a.size(), false);
        for (unsigned cur = 0; cur < a.size(); ++cur) {
            if (alt) {
                while (!vst[cur]) {
                    std::swap(a[cur], b[cur]);
                    vst[cur] = true;
                    cur = inva[a[cur]];
                }
            }
            else while (!vst[cur]) vst[cur] = true, cur = inva[b[cur]];
            alt = !alt;
        }
    }

    void crossover_edge_recomb(chromosome& a, chromosome& b) {
        auto recomb = [this](unsigned cur, const std::vector<std::vector<unsigned>>& adj) {
            unsigned len = adj.size();
            std::vector<bool> vst(len, false);
            std::vector<unsigned> cnt(len, 0);
            for (unsigned i = 0; i < len; ++i) cnt[i] = adj[i].size();

            chromosome re(len);
            for (unsigned i = 0; i < len; ++i) {
                re[i] = cur;
                vst[cur] = true;
                std::vector<unsigned> cand;
                for (auto nxt : adj[cur]) if (!vst[nxt]) --cnt[nxt], cand.push_back(nxt);
                
                if (cand.empty()) {
                    std::vector<unsigned> un;
                    for (unsigned j = 0; j < len; ++j) if (!vst[i]) un.push_back(i);
                    cur = std::uniform_int_distribution<unsigned>(0, un.size() - 1)(gen);
                }
                else {
                    std::sort(cand.begin(), cand.end(), [&](unsigned a, unsigned b) {
                        return cnt[a] < cnt[b];
                    });
                    while (cnt[cand.front()] != cnt[cand.back()]) cand.pop_back();
                    cur = cand[std::uniform_int_distribution<unsigned>(0, cand.size() - 1)(gen)];
                }
            }
            return re;
        };

        unsigned len = a.size();
        std::vector<std::vector<unsigned>> adj(len);
        for (unsigned i = 0; i < len; ++i) {
            unsigned n1 = a[(i + 1) % len], n2 = a[(i + len - 1) % len];
            adj[a[i]].push_back(n1);
            adj[a[i]].push_back(n2);
        }
        for (unsigned i = 0; i < len; ++i) {
            unsigned n1 = a[(i + 1) % len], n2 = a[(i + len - 1) % len];
            auto& vec = adj[a[i]];
            if (n1 != vec.front() && n1 != vec[1]) vec.push_back(n1);
            if (n2 != vec.front() && n2 != vec[1]) vec.push_back(n2);
        }

        unsigned x = std::uniform_int_distribution<unsigned>(0, len - 1)(gen);
        unsigned y = (x + std::uniform_int_distribution<unsigned>(0, len - 2)(gen)) % len;
        a = recomb(x, adj), b = recomb(y, adj);
    }

protected:
    // mutations
    // void mutation(population& ppl)
    //      mutate ppl directly

    void mutation_dummy(chromosome& chr) {}

    void mutation_insert(chromosome& chr) {
        unsigned x = std::uniform_int_distribution<unsigned>(0, chr.size() - 1)(gen);
        unsigned y = (x + std::uniform_int_distribution<unsigned>(0, chr.size() - 2)(gen)) % chr.size();
        if (x > y) std::swap(x, y);
        unsigned p = chr[y];
        for (unsigned i = y; i > x; --i) chr[i] = chr[i - 1];
        chr[x] = p;
    }

    void mutation_swap(chromosome& chr) {
        unsigned x = std::uniform_int_distribution<unsigned>(0, chr.size() - 1)(gen);
        unsigned y = (x + std::uniform_int_distribution<unsigned>(0, chr.size() - 2)(gen)) % chr.size();
        std::swap(chr[x], chr[y]);
    }

    void mutation_inverse(chromosome& chr) {
        std::uniform_int_distribution<unsigned> dis(0, chr.size() - 1);
        unsigned x = dis(gen), y = dis(gen);
        if (x > y) std::swap(x, y);
        std::reverse(chr.begin() + x, chr.begin() + y + 1);
    }

    void mutation_scramble(chromosome& chr) {
        unsigned x = std::uniform_int_distribution<unsigned>(0, chr.size() - 1)(gen);
        unsigned y = (x + std::uniform_int_distribution<unsigned>(0, chr.size() - 2)(gen)) % chr.size();
        if (x > y) std::swap(x, y);
        std::shuffle(chr.begin() + x, chr.begin() + y + 1, gen);
    }

protected:
    const problem* ins; // problem instance

    std::default_random_engine gen;
    func_selection selection = std::bind(&GeneticAlgorithm::selection_dummy, this, std::placeholders::_1, std::placeholders::_2);
    func_selection replacement = std::bind(&GeneticAlgorithm::selection_dummy, this, std::placeholders::_1, std::placeholders::_2);
    func_crossover crossover = std::bind(&GeneticAlgorithm::crossover_dummy, this, std::placeholders::_1, std::placeholders::_2);
    func_mutation mutation = std::bind(&GeneticAlgorithm::mutation_dummy, this, std::placeholders::_1);

    unsigned m = 100; // population
    unsigned T = 500; // # of iterations
    unsigned repeat = 1; // repeat multiple times and choose the best

    float parent_ratio = 0.6; // ratio of population being parents
    float mutation_rate = 0.75;

    unsigned tournament_k = 2;
    unsigned tournament_p = 0.7;

    bool demo = false;
    bool debug = false;
};

class ElitismGA : public GeneticAlgorithm {
    /*
        parents are the same as elites
        thus parent selection is the same as survivor selection
    */ 
public:
    ElitismGA(const std::string& args = "") : GeneticAlgorithm("name=elitism-ga " + args) {
        selection = std::bind(&ElitismGA::selection_elitism , this, std::placeholders::_1, std::placeholders::_2);
    }

protected:
    virtual void generation(population& pool, chromosome& best_chr, problem::obj_t& best_fit, unsigned parent_size) override {
        auto elite = selection(pool, parent_size);
        std::sort(elite.begin(), elite.end());
        for (unsigned i = 0; i < parent_size; ++i) std::swap(pool[i], pool[elite[i]]);
        for (unsigned i = parent_size; i < m; ++i) pool[i] = pool[i - parent_size];
        
        std::shuffle(pool.begin() + parent_size, pool.end(), gen);
        for (unsigned i = parent_size; i + 1 < m; i += 2) {
            crossover(pool[i], pool[i + 1]);
        }

        std::uniform_real_distribution<float> dis01(0, 1);
        for (unsigned i = parent_size; i < m; ++i) {
            if (dis01(gen) < mutation_rate)
                mutation(pool[i]);
        }
    }
};

class StandardGA : public GeneticAlgorithm {
    /*
        selection parent from population
        parent crossover and mutate to children
        new population is selected among population + children
    */
public:
    StandardGA(const std::string& args = "") : GeneticAlgorithm("name=standard-ga " + args) {}

protected:
    virtual void generation(population& pool, chromosome& best_chr, problem::obj_t& best_fit, unsigned parent_size) override {
        // parent selection
        if (debug) std::cout << "selection" << std::endl;
        auto parent = selection(pool, parent_size);
        std::shuffle(parent.begin(), parent.end(), gen);
        for (auto v : parent) pool.push_back(pool[v]);

        // crossover
        if (debug) std::cout << "crossover" << std::endl;
        for (unsigned i = m; i + 1 < m + parent_size; i += 2) crossover(pool[i], pool[i + 1]);

        // mutation
        if (debug) std::cout << "mutation" << std::endl;
        std::uniform_real_distribution<float> dis01(0, 1);
        for (unsigned i = m; i < m + parent_size; ++i) {
            if (dis01(gen) < mutation_rate)
                mutation(pool[i]);
        }

        // replacement (survivor selection)
        if (debug) std::cout << "replacement" << std::endl;
        auto survivor_idx = replacement(pool, m);
        // std::shuffle(survivor_idx.begin(), survivor_idx.end(), gen);
        if (debug) std::cout << "update" << std::endl;
        population survivor(m);
        for (unsigned i = 0; i < m; ++i) survivor[i] = pool[survivor_idx[i]];
        pool = std::move(survivor);
    }
};

class SteadyStateGA : public GeneticAlgorithm {
    /*
        no complicated replacement 
        parent are directly replaced with children
        no copy of chromosome needed, so it's fast  ...?
    */
public:
    SteadyStateGA(const std::string& args = "") : GeneticAlgorithm("name=ss-ga " + args) {
        assign("group_size", group_size);
    }

protected:
    virtual void generation(population& pool, chromosome& best_chr, problem::obj_t& best_fit, unsigned parent_size) override {
        // parent selection
        auto parent = selection(pool, parent_size);
        std::shuffle(parent.begin(), parent.end(), gen);

        // crossover
        auto dis = [](const chromosome& a, const chromosome& b) {
            unsigned re = 0;
            for (unsigned i = 0; i < a.size(); ++i) re += std::min(a[i] - b[i], b[i] - a[i]);
            return re;
        };
        auto wams = [&](const chromosome& chr) {
            auto picks = selection_random(pool, group_size);
            unsigned worst = pool.size();
            problem::obj_t worst_fit = 1e9;
            for (unsigned i = 0; i < m; i += group_size) {
                unsigned nearest = i, nearest_dis = 1e9;
                for (unsigned j = 0; j < group_size; ++j) {
                    unsigned ndis = dis(pool[i + j], chr);
                    if (ndis < nearest_dis) nearest = i + j, nearest_dis = ndis;
                }
                problem::obj_t nfit = fitness(pool[nearest]);
                if (nfit < worst_fit) worst = nearest, worst_fit = nfit;
            } 
            if (worst_fit < fitness(chr)) {
                pool[worst] = chr;
            }

        };
        std::uniform_real_distribution<float> dis01(0, 1);
        for (unsigned i = 0; i + 1 < parent_size; i += 2) {
            chromosome c1 = pool[parent[i]], c2 = pool[parent[i + 1]];
            crossover(c1, c2);

            // mutation
            if (dis01(gen) < mutation_rate)
                mutation(c1);
            if (dis01(gen) < mutation_rate)
                mutation(c2);
            wams(c1);
            wams(c2);
        }
    }

protected:
    unsigned group_size = 20;
};