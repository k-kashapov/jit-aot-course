#include <loop.h>

namespace IR {

void do_collect(BB* bb, bbSet& visited, bbSet& gray, std::set<edge> &backedges) {
    gray.insert(bb);
    visited.insert(bb);

    const auto [l, r] = bb->getSuccessors();
    if (l) {
        if (gray.contains(l)) {
            backedges.insert(edge(bb, l));
        }

        if (!visited.contains(l)) {
            do_collect(l, visited, gray, backedges);
        }
    }

    if (r) {
        if (gray.contains(r)) {
            backedges.insert(edge(bb, r));
        }

        if (!visited.contains(r)) {
            do_collect(r, visited, gray, backedges);
        }
    }

    gray.erase(bb);
}

std::set<edge> collect_backedges(BB* start) {
    bbSet gray;
    bbSet visited;
    std::set<edge> backedges;
    do_collect(start, visited, gray, backedges);

    return backedges;
}

LoopMap collect_loops(dominatorMap dmap, std::set<edge> backedges) {
    LoopMap loops;

    for (auto &b : backedges) {
        BB* header = b.second;
        BB* latch = b.first;

        if (!dmap[latch].contains(header)) {
            continue;
        }

        auto [headerIter, inserted] = loops.insert(std::pair{header, Loop({}, {latch})});
        if (!inserted) {
            headerIter->second.innerBBs.insert(latch);
        }
    }

    return loops;
}

std::map<IR::BB*, std::vector<IR::BB*>> FindAllLoops(IR::Function& func, IR::BB* start) {
    auto allNodes = func.getBBs();
    auto dominators = find_dominators(start, allNodes);

    auto backs = IR::collect_backedges(start);
    auto loops = IR::collect_loops(dominators, backs);

    std::vector<IR::BB*> postorder;
    auto savePO = [&postorder](IR::BB* bb){ postorder.push_back(bb); };
    IR::postorder(start, savePO);

    std::map<IR::BB*, std::vector<IR::BB*>> latch_to_loop;

    for (auto node : postorder) {
        for (auto backedge : backs) {
            if (backedge.second == node) {
                // This is a latch

                std::vector<IR::BB*> dfsOrder;
                auto saveDFS = [&dfsOrder](IR::BB* bb){ dfsOrder.push_back(bb); };
                IR::reverse_dfs(backedge.first, saveDFS, node);

                latch_to_loop[node] = dfsOrder;
                break;
            }
        }   
    }

    return latch_to_loop;
}


}; // namespace IR
