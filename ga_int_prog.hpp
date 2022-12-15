// genetic algorithm based on integer program
#pragma once

#include <unordered_set>
#include <unordered_map>
#include <optional>

#include "solver.hpp"
#include "dsu.hpp"

class GeneAlgoIntProg : public GeneticAlgorithm {
public:
    class chromosome {
    public:
        using frag = unsigned long long;
    public:
        chromosome(unsigned n) : n(n), val(n >> SFT + (n & ((1 << SFT) - 1) != 0)) {}

        std::vector<frag> operator()() { return val; }
        const std::vector<frag> operator()() const { return val; }

        void resize(unsigned n) {
            this->n = n;
            val.resize(n >> SFT + (n & ((1 << SFT) - 1) != 0));
        } 
        std::size_t size() const { return n; }

    public:
        void random(std::default_random_engine& gen) {
            for (auto& v : val) v = std::uniform_int_distribution<frag>(0)(gen);
            if (n % BITS == 0) return;
            frag excess_mask = (1 << (n % BITS)) - 1;
            val.back() &= excess_mask;
        }

        void swap_slice(chromosome& oth, unsigned l, unsigned r) {
            unsigned idxl = l / BITS, idxr = r / BITS;
            unsigned reml = l % BITS, remr = r % BITS;
            if (idxl == idxr) {
                frag mask = ((remr == BITS - 1)? -1 : ((1 << (remr + 1)) - 1)) & ~((1 << reml) - 1);
                frag slicea = val[idxl] & mask, sliceb = oth.val[idxl] & mask;
                val[idxl] ^= slicea ^ sliceb;
                oth.val[idxl] ^= slicea ^ sliceb;
            }
            else {
                frag maskl = ~((1 << reml) - 1), maskr = (remr == BITS - 1)? -1 : ((1 << (remr + 1)) - 1);
                frag sliceal = val[idxl] & maskl, slicebl = oth.val[idxl] & maskl;
                frag slicear = val[idxr] & maskr, slicebr = oth.val[idxr] & maskr;
                val[idxl]     ^= sliceal ^ slicebl;
                oth.val[idxl] ^= sliceal ^ slicebl;
                val[idxr]     ^= slicear ^ slicebr;
                oth.val[idxr] ^= slicear ^ slicebr;
                for (unsigned i = idxl + 1; i < idxr; ++i) std::swap(val[i], oth.val[i]);
            }
        }

        void flip(unsigned i) {
            val[i / BITS] ^= (1 << (i % BITS));
        }

    protected:
        std::size_t n;
        std::vector<frag> val;
    public:
        static const unsigned BITS = 64, SFT = 6;
    };

protected:
    struct variable {
        // edge {i, j}, k-th cycle
        // idx = k * (n * (n + 1)) / 2 + n + (n - 1) + ... + (n - i + 1) + (j - i)
        unsigned i, j, k;
    };

public:
    GeneAlgoIntProg(const std::string& args = "") : GeneticAlgorithm("name=gaip " + args) {
        if (meta.find("elite_rate") != meta.end()) elite_rate = static_cast<float>(meta["elite_rate"]);
        if (meta.find("mutation_rate") != meta.end()) mutation_rate = static_cast<float>(meta["mutation_rate"]);

        if (meta.find("penalty_cover") != meta.end()) penalty_cover = static_cast<float>(meta["penalty_cover"]);
        if (meta.find("penalty_cycle") != meta.end()) penalty_cycle = static_cast<float>(meta["penalty_cycle"]);
        if (meta.find("penalty_connect") != meta.end()) penalty_connect = static_cast<float>(meta["penalty_connect"]);
    }

protected:
    std::vector<chromosome> initialize(const problem& ins) {
        std::vector<chromosome> re(m);
        for (auto& v : re) {
            // indexed by {i, j} * k
            v.resize(n * (n + 1) * ins.get_k() / 2);
            v.random(gen);
        }
        return re;
    }

    std::vector<variable> decode(const chromosome& chr) const {
        auto bit_scan = [](chromosome::frag u) {
            unsigned re = 0, v = 32;
            while (u != 1) {
                if (u >> v) re += v;
                v >>= 2;
            }
            return re;
        };
        auto lsb = [](chromosome::frag u) { return u & -u; };
        auto reset = [](chromosome::frag u) { return u & (u - 1); };

        std::vector<variable> re;

        unsigned acc = 0;
        unsigned tri = 0, tri_base = n, tri_all = n * (n + 1) / 2;
        for (auto v : chr()) {
            for (auto u = v; u; u = reset(u)) {
                unsigned idx = acc + bit_scan(lsb(u));
                unsigned k = idx / tri_all;
                if (idx < tri) tri = 0, tri_base = n;
                while (tri + tri_base < idx) tri += tri_base--;
                // this bit 1 indecates edge {i, j}
                unsigned i = n - tri_base, j = i + idx - tri;

                re.push_back({i, j, k});
            }
            acc += chromosome::BITS;
        }
        return re;
    }

    problem::obj_t fitness(const chromosome& chr, unsigned t, const problem& ins) const {
        problem::obj_t fitn = 0;

        std::vector<std::vector<unsigned>> covered(n, std::vector<unsigned>(ins.get_k()));
        std::vector<dsu> ds(ins.get_k(), dsu(n));
        std::vector<std::unordered_set<unsigned>> us(ins.get_k());

        auto vars = decode(chr);

        for (auto& [i, j, k] : vars) {
            fitn -= ins()(i, j);
            ++covered[i][k], ++covered[j][k];
            ds[k].union_byrank(i, j);
        }

        for (auto& [i, j, k] : vars) {
            us[k].insert(ds[k].find(i));
            us[k].insert(ds[k].find(j));
        }

        for (auto& s : us) {
            // if s.size() == 0, no any edge, fine
            // if s.size() == 1, exactly one connected component, good
            // else bad
            if (s.size() >= 2)
                fitn -= (s.size() - 1) * penalty_connect * t;
        }

        for (auto& nd_cover : covered) {
            unsigned cnt = 0;
            for (auto v : nd_cover) {
                // penalty on cycle k accually not forming a cycle    
                fitn -= std::abs(static_cast<int>(v) - 2) * penalty_cycle * t;
                cnt += v;
            }
            
            // penalty on not covering current node
            if (cnt == 0) fitn -= penalty_cover * t;
        }

        return fitn;
    }

    std::vector<chromosome> selection(const std::vector<chromosome>& pool, unsigned t, const problem& ins) {
        auto copy = pool;
        unsigned elite_size = copy.size() * elite_rate;
        std::partial_sort(copy.begin(), copy.begin() + elite_size, copy.end(), [&](const chromosome& a, const chromosome& b) {
            return fitness(a, t, ins) > fitness(b, t, ins);
        });
        copy.erase(copy.begin() + elite_size, copy.end());
        std::shuffle(copy.begin(), copy.end(), gen);
        return copy;
    }

    std::vector<chromosome> crossover(const std::vector<chromosome>& pool, unsigned num) {
        // generate num children
        std::vector<chromosome> re;
        std::uniform_int_distribution<unsigned> dis(0, len - 1);
        while (re.size() < num) {
            for (unsigned i = 0; i + 1 < pool.size() && re.size() < num; i += 2) {
                // 2 point crossover
                unsigned l = dis(gen), r = dis(gen);
                if (r < l) std::swap(l, r);
                re.push_back(pool[i]);
                re.push_back(pool[i + 1]);
                re[re.size() - 2].swap_slice(re.back(), l, r);
            }
            std::shuffle(pool.begin(), pool.end(), gen);
        }
            
        while (re.size() > num) re.pop_back();
        return re;
    }

    void mutation(std::vector<chromosome>& pool) {
        auto choose = [&](unsigned n, unsigned k) {
            // choose k position in [0, n-1]
            std::unordered_set<unsigned> us;
            for (unsigned i = n - k; i < n; ++i) {
                unsigned t = std::uniform_int_distribution<unsigned>(0, i)(gen);
                if (us.find(t) == us.end()) us.insert(t);
                else us.insert(i);
            }

            std::vector<unsigned> re;
            for (auto v : us) re.push_back(v);
            return re;
        };

        unsigned cnt = len * mutation_rate;
        for (auto& chr : pool) {
            // flip cnt bits randomly
            for (auto v : choose(len, cnt)) {
                chr.flip(v);
            }
        }
    }

    std::optional<solution> phenotype(const std::vector<variable>& vars, const problem& ins) const {
        std::vector<std::vector<std::pair<unsigned, unsigned>>> cycs(ins.get_k());
        for (auto& [i, j, k] : vars) cycs[k].push_back({i, j});

        solution sol(ins.get_k());

        for (unsigned i = 0; i < ins.get_k(); ++i) {
            auto& cyc_sol = sol[i];
            if (cyc_sol.empty()) continue;

            std::vector<std::vector<unsigned>> adj(n);
            auto& cyc = cycs[i];
            for (auto& [u, v] : cyc) {
                adj[u].push_back(v);
                adj[v].push_back(u);
            }
            for (auto& vec : adj) {
                // not forming a cycle
                if (vec.size() != 0 && vec.size() != 2) return std::nullopt;
            }

            // collect the cycle containing cyc.back().first
            // while there may be other cycles under index k, here just ignore them
            unsigned cnt = 0, cur = cyc.back().first;
            std::vector<bool> vst(n);
            while (!vst[cur]) {
                cyc_sol.push_back(cur);
                vst[cur] = true;
                ++cnt;
                for (auto nxt : adj[cur]) {
                    if (!vst[nxt]) {
                        cur = nxt;
                        break;
                    }
                }
            }
        }

        std::vector<bool> vst(n);
        for (auto& cyc : sol) {
            for (auto v : cyc) {
                vst[v] = true;
            }
        }
        for (auto b : vst) {
            // not covering all nodes
            if (!b) return std::nullopt;
        }
        return sol;
    }

public:
    virtual solution solve(const problem& ins) override {
        n = ins().size();
        len = n * (n + 1) * ins.get_k() / 2;
        if (meta.find("mutation_rate") == meta.end()) mutation_rate = 1.0 / n;

        unsigned t = T;
        unsigned elite_size = m * elite_rate;

        auto pool = initialize(ins);
        auto best_c = pool[0];
        auto best_fitness = fitness(best_c, T - t, ins);

        while (t--) {
            auto elite = selection(pool, T - t, ins);
            auto child = crossover(elite, m - elite_size);
            mutation(child);
            pool = elite;
            pool.insert(pool.end(), child.begin(), child.end());

            for (auto& chr : elite) {
                problem::obj_t nfit = fitness(chr, T - t, ins);
                if (best_fitness < nfit) best_c = chr, best_fitness = nfit;
            }
        }

        // decode to phenotype aka solution
        auto sol = phenotype(decode(best_c), ins);
        return (sol)? *sol : solution();
    }

protected:
    float penalty_cover = 10;
    float penalty_cycle = 10;
    float penalty_connect = 10;
    float elite_rate = 0.1;
    float mutation_rate = 0;
};