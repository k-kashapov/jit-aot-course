#include <algorithm>
#include <domination.h>

namespace IR {

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
    std::map<BB*, int64_t> enumeration;
    int64_t idx = 0;
    auto collect_idx = [&enumeration, &idx](BB* bb){ enumeration.insert(std::pair{bb, idx++}); };
    auto order = bfs(start, collect_idx);

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

};
