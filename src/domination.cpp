#include <algorithm>
#include <domination.h>

using BB = IR::BasicBlock;
using bbSet = std::set<BB *>;
using dominatorMap = std::unordered_map<BB *, bbSet>;

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
