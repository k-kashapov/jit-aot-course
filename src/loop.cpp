#include <loop.h>

namespace IR {

void do_collect(BB* bb, bbSet& visited, bbSet& gray, std::set<edge> &backedges) {
    gray.insert(bb);
    visited.insert(bb);

    // std::cout << "visiting " << bb->getName() << '\n';

    const auto [l, r] = bb->getSuccessors();
    if (l) {
        // std::cout << "\t" << bb->getName() << ": left child " << l->getName();
        if (gray.contains(l)) {
            // std::cout << " already gray, adding backedge\n";
            backedges.insert(edge(bb, l));
        }

        if (!visited.contains(l)) {
            // std::cout << l->getName() << " has not been visited\n";
            do_collect(l, visited, gray, backedges);
        }
    }

    if (r) {
        // std::cout << "\t" << bb->getName() << ": right child " << r->getName();
        if (gray.contains(r)) {
            // std::cout << " already gray, adding backedge\n";
            backedges.insert(edge(bb, r));
        }

        if (!visited.contains(r)) {
            // std::cout << r->getName() << " has not been visited\n";
            do_collect(r, visited, gray, backedges);
        }
    }

    // std::cout << "leaving " << bb->getName() << "\n";
    gray.erase(bb);
}

std::set<edge> collect_backedges(BB* start) {
    bbSet gray;
    bbSet visited;
    std::set<edge> backedges;
    do_collect(start, visited, gray, backedges);

    return backedges;
}

Loops collect_loops(dominatorMap dmap, std::set<edge> backedges) {
    Loops loops;

    for (auto &b : backedges) {
        BB* header = b.second;
        BB* latch = b.first;

        if (dmap[latch].contains(header)) {
            auto headerIter = loops.find(header);
            if (headerIter != loops.end()) {
                headerIter->second.insert(latch);
                break;
            } else {
                loops[b.second] = {b.first};
            }
        }
    }

    return loops;
}

};
