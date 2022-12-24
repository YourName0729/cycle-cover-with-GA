#pragma once

#include <unordered_set>
#include <algorithm>
#include <functional>
#include <iterator>

#include "ga.hpp"

class TabuGA : public GeneticAlgorithm {
public:
    TabuGA(const std::string& args = "") : GeneticAlgorithm("name=tabu-ga " + args) {}

protected:
    class TabuSet {
    public:
        TabuSet() {}

    public:
        bool contains(const chromosome& chr) const { return us.find(hashing(chr)) != us.end(); }
        bool insert(const chromosome& chr) { return us.insert(hashing(chr)).second; }
        bool erase(const chromosome& chr) { return us.erase(hashing(chr)) == 1u;}

    public:
        void setHashTable(unsigned len, std::default_random_engine& gen) {
            // len: len of chromosome
            hvt.resize(len);
            std::uniform_int_distribution<std::size_t> dis(0);
            for (auto& vec : hvt) {
                vec.resize(len);
                for (auto& v : vec) v = dis(gen);
            }
            // for (auto& vec : hvt) {
            //     std::copy(vec.begin(), vec.end(), std::ostream_iterator<std::size_t>(std::cout, "\t"));
            //     std::cout << "\n";
            // }
        }
    protected:
        std::size_t hashing(const chromosome& chr) const {
            std::size_t v = 0;
            for (unsigned i = 0; i < chr.size(); ++i) v ^= hvt[i][chr[i]];
            return v;
        }
    protected:
        std::vector<std::vector<std::size_t>> hvt; // hash value table
        std::unordered_set<std::size_t> us;
    };

protected:
    chromosome random_chromosome() {
        chromosome chr(get_len());
        do {
            std::iota(chr.begin(), chr.end(), 0);
            std::shuffle(chr.begin(), chr.end(), gen);
            ++tabu_hit;
        } while (tbs.contains(chr));
        --tabu_hit, ++tabu_miss;
        tbs.insert(chr);
        return chr;
    }

    virtual population initialize_pool() override {
        population ppl(m);
        std::generate(ppl.begin(), ppl.end(), std::bind(&TabuGA::random_chromosome, this));
        return ppl;
    }

public:
    virtual solution solve(const problem& ins) override {
        tbs.setHashTable(ins().size() + ins.get_k() - 1, gen);
        return GeneticAlgorithm::solve(ins);
    }

     virtual solution solve_single(const problem& ins) override {
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
            fout.open(meta["save"], std::fstream::app);
            fout << std::fixed;
            begin = std::chrono::steady_clock::now();
            block = 1000;
            assign("block", block);
        }

        auto update_best = [&](unsigned t) {
            problem::obj_t mx = -1e9, mn = 1e9;
            for (auto& chr : pool) {
                problem::obj_t nfit = fitness(chr);
                mx = std::max(mx, nfit), chr_avg += nfit, mn = std::min(mn, nfit);
                if (best_fit < nfit) best_chr = chr, best_fit = nfit;
            }
            chr_max += mx, chr_min += mn;
            if (t % block == 0) {
                // problem::obj_t mx = -1e9, sm = 0, mn = 1e9;
                // for (auto& chr : pool) {
                //     problem::obj_t nfit = fitness(chr);
                //     if (record) mx = std::max(mx, nfit), sm += nfit, mn = std::min(mn, nfit);
                //     if (best_fit < nfit) best_chr = chr, best_fit = nfit;
                // }

                // unsigned dsm = 0;
                // auto dista = [](const chromosome& a, const chromosome& b) {
                //     unsigned re = 0;
                //     for (unsigned i = 0; i < a.size(); ++i) re += std::min(a[i] - b[i], b[i] - a[i]);
                //     return re;
                // };
                // std::uniform_int_distribution<unsigned> dis(0, pool.size() - 1);
                // for (unsigned i = 0; i < 20; ++i) {
                //     dsm += dista(pool[dis(gen)], pool[dis(gen)]);
                // }

                auto end = std::chrono::steady_clock::now();

                fout << "T=" << T - t << ' ';
                fout << "t=" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << ' ';
                fout << "best=" << best_fit << ' ';
                fout << "mn=" << chr_min / block << ' ';
                fout << "avg=" << chr_avg / block / m << ' ';
                fout << "mx=" << chr_max / block << ' ';
                fout << "miss=" << tabu_miss << ' ';
                fout << "hit=" << tabu_hit << ' ';
                fout << "new=" << tabu_new << ' ';
                fout << "wams_miss=" << wams_miss << ' ';
                fout << "diversity=" << diversity << '\n';

                tabu_hit = 0, tabu_miss = 0, tabu_new = 0;
                wams_miss = 0;
                diversity = 0;
                chr_max = 0, chr_min = 0, chr_avg = 0;
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
            // generation
            // parent selection
            // std::cout << "generation " << T - t << '\n';
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
                std::uniform_real_distribution<float> dis(0, 1);
                problem::obj_t chr_fit = fitness(chr);
                auto accept_rate = [&]() {
                    return (std::pow(wams_rate, worst_fit / chr_fit) - 1) / (wams_rate - 1);
                };
                if (worst_fit < chr_fit || dis(gen) < accept_rate()) {
                    pool[worst] = chr;
                    tbs.insert(chr);
                }
                else ++wams_miss;
            };

            // crossover
            std::uniform_real_distribution<float> dis01(0, 1);
            for (unsigned i = 0; i + 1 < parent_size; i += 2) {
                chromosome c1, c2;
                bool v1, v2;
                unsigned cnt = 0;
                do {
                    c1 = pool[parent[i]], c2 = pool[parent[i + 1]];
                    crossover(c1, c2);

                    // mutation
                    if (dis01(gen) < mutation_rate) mutation(c1);
                    if (dis01(gen) < mutation_rate) mutation(c2);
                    
                    v1 = tbs.contains(c1), v2 = tbs.contains(c2);
                    if (!v1) wams(c1);
                    if (!v2) wams(c2);

                    tabu_hit += v1 + v2;
                    tabu_miss += !v1 + !v2;
                } while (v1 && v2 && ++cnt < tabu_tol);
                
                if (cnt > tabu_tol) pool[parent[i]] = random_chromosome(), pool[parent[i + 1]] = random_chromosome(), tabu_new += 2;
            }

            // mutation all
            for (auto& chr : pool) {
                unsigned cnt = 0;
                chromosome cpy = chr;
                while (!tbs.contains(cpy) && ++cnt < tabu_tol);
                tabu_hit += cnt;
                if (cnt > tabu_tol) {
                    // almost all neighbor of chr is visited, replace chr with a random chromosome directly
                    chr = random_chromosome();
                    ++tabu_new;
                }
                else {
                    ++tabu_miss;
                    chr = std::move(cpy);
                    tbs.insert(chr);
                }
            }

            update_best(t);
            std::uniform_int_distribution<unsigned> dis_ppl(0, pool.size() - 1);
            diversity += dis(pool[dis_ppl(gen)], pool[dis_ppl(gen)]);
        }

        if (record) fout.close();

        return decode(best_chr);
    }

protected:
    // statistic
    unsigned tabu_hit = 0, tabu_miss = 0, tabu_new = 0;
    unsigned wams_miss = 0;
    unsigned diversity = 0;
    problem::obj_t chr_max = 0, chr_min = 0, chr_avg = 0;

    float wams_rate = 1000;
    unsigned group_size = 10;
    unsigned tabu_tol = 10;
    TabuSet tbs;
};