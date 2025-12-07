#include <utils.h>

namespace IR {

std::vector<BB*>& _bfs(BB* bb, std::vector<BB*>& order, bbFunc fn) {
    const auto [l, r] = bb->getSuccessors();
    bool visitL = false;
    bool visitR = false;

    if (l && std::find(order.begin(), order.end(), l) == order.end()) {
        visitL = true;
        order.push_back(l);
        if (fn) { fn(l); }
    }
    if (r && std::find(order.begin(), order.end(), r) == order.end()) {
        visitR = true;
        order.push_back(r);
        if (fn) { fn(r); }
    }

    if (visitL) {
        _bfs(l, order, fn);
    }
    if (visitR) {
        _bfs(r, order, fn);
    }

    return order;
}

std::vector<BB*> bfs(BB* start, bbFunc fn) {
    std::vector<BB*> order{start};
    if (fn) { fn(start); }
    return _bfs(start, order, fn);
}

void _dfs(BB* bb, bbSet& visited, bbFunc fn) {
    if (fn) { fn(bb); }
    visited.insert(bb);

    const auto [l, r] = bb->getSuccessors();
    if (l && !visited.contains(l)) {
        _dfs(l, visited, fn);
    }
    if (r && !visited.contains(r)) {
        _dfs(r, visited, fn);
    }
}

void dfs(BB* start, bbFunc fn) {
    std::set<BB*> visited;
    _dfs(start, visited, fn);
}

void _reverse_dfs(BB* bb, bbSet& visited, bbFunc fn) {
    // std::cout << "\tvisiting " << bb->getName() << '\n';
    if (fn) { fn(bb); }
    visited.insert(bb);

    for (auto p : bb->getPreds()) {
        // std::cout << "\t\tpred " << p->getName() << ' ';
        if (p && !visited.contains(p)) {
            // std::cout << "has not been visited\n";
            _reverse_dfs(p, visited, fn);
            // std::cout << "\t\treturning to " << bb->getName() << "\n";
        }
    }
}

void reverse_dfs(BB* start, bbFunc fn, BB* end) {
    std::set<BB*> visited = {end};
    _reverse_dfs(start, visited, fn);
}


void _postorder(BB* bb, bbSet &visited, bbFunc fn) {
    visited.insert(bb);

    const auto [l, r] = bb->getSuccessors();
    if (l && !visited.contains(l)) {
        _postorder(l, visited, fn);
    }
    if (r && !visited.contains(r)) {
        _postorder(r, visited, fn);
    }

    fn(bb);
}

void postorder(BB* bb, bbFunc fn) {
    bbSet visited;
    _postorder(bb, visited, fn);
}

}; // namespace IR

