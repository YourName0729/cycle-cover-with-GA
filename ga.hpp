#pragma once

#include <memory>
#include <functional>

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
        assign("parent_ratio", parent_ratio);
        assign("mutation_rate", mutation_rate);
    }

protected:
    using chromosome = std::vector<unsigned>;
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

    population initialize_pool() {
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

public:
    virtual solution solve(const problem& ins) {
        return solution();
    }

protected:
    // selections
    // return the index of selected chromosomes, of size k

    std::vector<unsigned> selection_dummy(population& ppl, unsigned k) {
        std::vector<unsigned> re(k);
        for (unsigned i = 0; i < k; ++i) re[i] = k % ppl.size();
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
        for (unsigned i = 0; i < k; ++i) order[i] = {fitness(ppl[i]), i};
        std::partial_sort(order.begin(), order.begin() + k, order.end());

        std::vector<unsigned> re(k);
        for (unsigned i = 0; i < k; ++i) re[i] = order[i].second;
        return re;
    }

    std::vector<unsigned> selection_roulette_wheel(population& ppl, unsigned k) {
        std::vector<std::pair<problem::obj_t, unsigned>> order(ppl.size());
        for (unsigned i = 0; i < k; ++i) order[i] = {fitness(ppl[i]), i};
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
        // TODO
        return selection_dummy(ppl, k);
    }

protected:
    // crossovers

    void crossover_dummy(chromosome& a, chromosome& b) {}

    void crossover_pmx(chromosome& a, chromosome& b) {
        // TODO
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
        // TODO
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

    float parent_ratio = 0.75; // ratio of population being parents
    float mutation_rate = 0.5;

    unsigned tournament_k = 3;
    unsigned tournament_p = 0.9;

    bool demo = false;
};

class ElitismGA : public GeneticAlgorithm {
    /*
        parents are the same as elites
        thus parent selection is the same as survivor selection
    */ 
public:
    ElitismGA(const std::string& args = "") : GeneticAlgorithm("name=elitism-ga " + args) {
        if (meta.find("selection") == meta.end()) selection = std::bind(&ElitismGA::selection_elitism , this, std::placeholders::_1, std::placeholders::_2);
    }

public:
    virtual solution solve(const problem& ins) override {
        this->ins = &ins;
        auto pool = initialize_pool();
        unsigned elite_size = m * elite_rate;
        auto best_chr = pool[0];
        problem::obj_t best_fit = fitness(best_chr);
        
        unsigned t = T - 1;
        while (t--) {
            auto elite = selection(pool, elite_size);
            std::sort(elite.begin(), elite.end());
            for (unsigned i = 0; i < elite_size; ++i) pool[i] = pool[elite[i]];
            for (unsigned i = elite_size; i < m; ++i) pool[i] = pool[i - elite_size];
            
            std::shuffle(pool.begin() + elite_size, pool.end(), gen);
            for (unsigned i = elite_size; i + 1 < m; i += 2) {
                crossover(pool[i], pool[i + 1]);
            }

            std::uniform_real_distribution<float> dis01(0, 1);
            for (unsigned i = elite_size; i < m; ++i) {
                if (dis01(gen) < mutation_rate)
                    mutation(pool[i]);
            }

            for (auto& chr : pool) {
                problem::obj_t nfit = fitness(chr);
                if (best_fit < nfit) best_chr = chr, best_fit = nfit;
            }
        }

        return decode(best_chr);
    }

protected:

    float elite_rate = 0.1;
};

class StandardGA : public GeneticAlgorithm {
    /*
        selection parent from population
        parent crossover and mutate to children
        new population is selected among population + children
    */
public:
    StandardGA(const std::string& args = "") : GeneticAlgorithm("name=standard-ga " + args) {}

public:
    virtual solution solve(const problem& ins) override {
        this->ins = &ins;
        auto pool = initialize_pool();
        unsigned parent_size = m * parent_ratio;
        pool.reserve(m + parent_size);

        auto best_chr = pool[0];
        problem::obj_t best_fit = fitness(best_chr);

        unsigned t = T - 1;
        while (t--) {
            // parent selection
            auto parent = selection(pool, parent_size);
            for (auto v : parent) pool.push_back(pool[v]);

            // crossover
            std::shuffle(pool.begin() + m, pool.end(), gen);
            for (unsigned i = m; i + 1 < m + parent_size; i += 2) crossover(pool[i], pool[i + 1]);

            // mutation
            std::uniform_real_distribution<float> dis01(0, 1);
            for (unsigned i = m; i < m + parent_size; ++i) {
                if (dis01(gen) < mutation_rate)
                    mutation(pool[i]);
            }

            // replacement (survivor selection)
            auto survivor_idx = replacement(pool, m);
            population survivor(m);
            for (unsigned i = 0; i < m; ++i) survivor[i] = pool[survivor_idx[i]];
            pool = std::move(survivor);

            for (auto& chr : pool) {
                problem::obj_t nfit = fitness(chr);
                if (best_fit < nfit) best_chr = chr, best_fit = nfit;
            }
        }

        return decode(best_chr);
    }
};

class FastGA : public GeneticAlgorithm {
    /*
        no complicated replacement 
        parent are directly replaced with children
        no copy of chromosome needed, so it's fast  ...?
    */
public:
    FastGA(const std::string& args = "") : GeneticAlgorithm("name=fast-ga " + args) {}

public:
    virtual solution solve(const problem& ins) override {
        this->ins = &ins;
        auto pool = initialize_pool();
        unsigned parent_size = m * parent_ratio;
        pool.reserve(m + parent_size);

        auto best_chr = pool[0];
        problem::obj_t best_fit = fitness(best_chr);

        unsigned t = T - 1;
        while (t--) {
            // parent selection
            auto parent = selection(pool, parent_size);

            // crossover
            for (unsigned i = 0; i + 1 < parent_size; i += 2) {
                crossover(pool[parent[i]], pool[parent[i + 1]]);
            }

            // mutation
            std::uniform_real_distribution<float> dis01(0, 1);
            for (auto v : parent) {
                if (dis01(gen) < mutation_rate)
                    mutation(pool[v]);
            }

            for (auto& chr : pool) {
                problem::obj_t nfit = fitness(chr);
                if (best_fit < nfit) best_chr = chr, best_fit = nfit;
            }
        }

        return decode(best_chr);
    }

};