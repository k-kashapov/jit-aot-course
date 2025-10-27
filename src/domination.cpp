#include <algorithm>
#include <domination.h>

using BB = IR::BasicBlock;
using bbSet = std::set<BB *>;
using dominatorMap = std::unordered_map<BB *, bbSet>;

std::map<BB*, int64_t>& _bfs(BB* bb, std::map<BB*, int64_t>& res, int64_t &idx) {
    const auto [l, r] = bb->getSuccessors();
    bool visitL = false;
    bool visitR = false;

    if (l && !res.contains(l)) {
        visitL = true;
        res.emplace(std::pair{l, idx++});
    }
    if (r && !res.contains(r)) {
        visitR = true;
        res.emplace(std::pair{r, idx++});
    }

    if (visitL) {
        _bfs(l, res, idx);
    }
    if (visitR) {
        _bfs(r, res, idx);
    }

    return res;
}

std::map<BB*, int64_t> bfs(BB* bb) {
    std::map<BB*, int64_t> res{std::pair(bb, 0)};
    int64_t idx = 1;

    return _bfs(bb, res, idx);
}

dominatorMap find_dominators(BB *start, bbSet allNodes) {
    dominatorMap dominators = {};
    for (auto *bb : allNodes) {
        dominators[bb] = allNodes;
    }

    dominators[start] = {start};

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto *bb : allNodes) {
            // std::cout << bb->getName() << ":\n";
            const auto &preds = bb->getPreds();
            if (preds.size() == 0) {
                // std::cout << "\tNo preds\n";
                continue;
            }

            bbSet intersection;
            if (preds.size() == 1) {
                // std::cout << "\tOne pred\n";
                intersection = dominators[preds[0]];
            } else {
                // std::cout << "\tPreds num: " << preds.size() << "\n";
                bbSet firstDoms = dominators[preds[0]];
                for (size_t idx = 1; idx < preds.size(); idx++) {
                    const auto &predDoms = dominators[preds[idx]];
                    std::set_intersection(firstDoms.begin(), firstDoms.end(), predDoms.begin(),
                                          predDoms.end(),
                                          std::inserter(intersection, intersection.begin()));
                }
            }

            intersection.insert(bb);
            if (dominators[bb] != intersection) {
                changed = true;
            }

            dominators[bb] = intersection;
        }
    }

    for (auto *bb : allNodes) {
        dominators[bb].erase(bb);
    }

    return dominators;
}

std::map<BB*, BB*> find_immediate_dominators(BB* start, dominatorMap dmap) {
    auto enumeration = bfs(start);
    for (auto p : enumeration) {
        std::cout << "bb " << p.first->getName() << " : " << p.second << "\n";
    }

    std::map<BB*, BB*> res = {};
    for (const auto &[bb, doms] : dmap) {
        int64_t max_dom_idx = -1;
        BB* immdom = nullptr;
        for (const auto dom : doms) {
            int64_t enum_idx = enumeration[dom];
            if (enum_idx > max_dom_idx) {
                max_dom_idx = enum_idx;
                immdom = dom;
            }
        }

        res[bb] = immdom;
    }

    return res;
}
