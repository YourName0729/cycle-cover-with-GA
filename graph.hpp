#pragma once

#include <vector>
#include <iostream>

template<typename T>
class graph {
public:
    using adj_mat_t = std::vector<std::vector<T>>;

public:
    graph(): n(0) {}
    graph(unsigned n): n(n) { 
        adj.resize(n);
        for (auto& vec : adj) vec.resize(n);
    }
    graph(unsigned n, const adj_mat_t& adj): n(n), adj(adj) {}

public:
    adj_mat_t& operator()() { return adj; }
    const adj_mat_t& operator()() const { return adj; }
    T& operator()(unsigned i, unsigned j) { return adj[i][j]; }
    const T& operator()(unsigned i, unsigned j) const { return adj[i][j]; }

    unsigned size() const { return n; }

public:
    friend std::ostream& operator<<(std::ostream& out, const graph<T>& g) {
        out << g.n << '\n';
        for (unsigned i = 0; i < g.n; ++i) {
            for (unsigned j = 0; j < g.n; ++j) {
                out << g.adj[i][j] << '\t';
            }
            out << '\n';
        }
        return out;
    }

private:
    unsigned n;
    adj_mat_t adj; // weight of each node i and j
};