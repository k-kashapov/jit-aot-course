#include <loop.h>

namespace IR {

void do_collect(BB* bb, bbSet& visited, bbSet& gray, std::map<BB*, BB*> &backedges) {
    gray.insert(bb);
    visited.insert(bb);

    const auto [l, r] = bb->getSuccessors();
    if (l) {
        if (gray.contains(l)) {
            backedges.insert({l, bb});
        }

        if (!visited.contains(l)) {
            do_collect(l, visited, gray, backedges);
        }
    }

    if (r) {
        if (gray.contains(r)) {
            backedges.insert({r, bb});
        }

        if (!visited.contains(r)) {
            do_collect(r, visited, gray, backedges);
        }
    }

    gray.erase(bb);
}

std::map<BB*, BB*> collect_backedges(BB* start) {
    bbSet gray;
    bbSet visited;
    std::map<BB*, BB*> backedges;
    do_collect(start, visited, gray, backedges);

    return backedges;
}

LoopMap collect_loops(dominatorMap dmap, std::map<BB*, BB*> backedges) {
    LoopMap loops;

    for (auto &b : backedges) {
        BB* header = b.first;
        BB* latch = b.second;

        // std::cout << "backedge = " << latch->getName() << "->" << header->getName() << "\n";

        if (!dmap[latch].contains(header)) {
            continue;
        }

        // std::cout << "header = " << header << "\n";

        auto [headerIter, inserted] = loops.insert(std::pair{header, Loop(header, {}, {latch})});
        if (!inserted) {
            headerIter->second.innerBBs.insert(latch);
        }
    }

    return loops;
}

std::map<BB*, Loop> FindAllLoops(IR::Function& func, IR::BB* start) {
    auto allNodes = func.getBBs();
    auto dominators = find_dominators(start, allNodes);

    auto backs = IR::collect_backedges(start);
    auto loops = IR::collect_loops(dominators, backs);

    std::vector<IR::BB*> allNodesPostorder;
    auto savePO = [&allNodesPostorder](IR::BB* bb){ allNodesPostorder.push_back(bb); };
    IR::postorder(start, savePO);

    std::map<IR::BB*, IR::Loop*> bbToLoopMapping;

    // std::cout << "postorder:\n"; 
    for (auto node : allNodesPostorder) {
        if (loops.find(node) != loops.end()) {
            auto backedgeIter = backs.find(node);
            auto header = backedgeIter->first;
            auto latch = backedgeIter->second;
            // std::cout << "\theader = " << header->getName() << "\n";
            // std::cout << "\tlatch = " << latch->getName() << "\n";

            std::vector<IR::BB*> dfsOrder;
            auto saveDFS = [&dfsOrder](IR::BB* bb){ dfsOrder.push_back(bb); };
            IR::reverse_dfs(latch, saveDFS, header);

            // std::cout << " dfs found: ";
            for (auto bbToAdd : dfsOrder) {
                // std::cout << bbToAdd->getName() << "<-";

                auto visitedIter = bbToLoopMapping.find(bbToAdd);
                if (visitedIter != bbToLoopMapping.end()) {
                    loops[header].innerLoops.insert(visitedIter->second);
                } else {
                    loops[header].innerBBs.insert(bbToAdd);
                    bbToLoopMapping[bbToAdd] = &loops[header];
                }
            }

            // std::cout << header->getName() << "\n";
        } else {
            // std::cout << "\tnot a latch: " << node->getName() << "\n";
        }
    }

    // std::cout << '\n';
    return loops;
}


}; // namespace IR
