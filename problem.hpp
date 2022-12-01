#pragma once

#include "graph.hpp"

using tour = std::vector<unsigned>;
using solution = std::vector<tour>;

std::ostream& operator<<(std::ostream& out, const solution& sol) {
    out << sol.size() << '\n';
    for (auto& cyc : sol) {
        for (auto v : cyc) {
            out << v << ' ';
        }
        out << '\n';
    }
    return out;
}

class problem {
public:
    using obj_t = float;
    using graph_t = graph<obj_t>;

public:
    problem(const graph_t& gr, unsigned k): g(gr), k(k) {}

public:
    virtual obj_t objective(const solution&) const { return 0; }
    virtual bool feasible(const solution&) const { return false; }

public:
    const graph_t& operator()() const { return g; }

    unsigned get_k() const { return k; }

public:
    friend std::ostream& operator<<(std::ostream& out, const problem& pro) {
        out << pro.k << ' ' << pro.g;
        return out;
    }

protected:
    graph_t g;
    unsigned k;
};

class MinSumProblem : public problem {
public:
    MinSumProblem(const graph<obj_t>& gr, unsigned k): problem(gr, k) {}

public:
    virtual obj_t objective(const solution& sol) const override {
        obj_t re = 0;
        for (auto& cyc : sol) {
            if (cyc.empty()) continue;
            for (unsigned i = 0; i < cyc.size() - 1; ++i) {
                re += g(cyc[i], cyc[i + 1]);
            }
            re += g(cyc.front(), cyc.back());
        }
        return re;
    }

    virtual bool feasible(const solution& sol) const override {
        if (sol.size() > k) return false;
        for (auto& cyc : sol) {
            if (cyc.empty()) continue;
            for (auto v : cyc) if (v >= g.size()) return false;
        }
        return true;
    }
};