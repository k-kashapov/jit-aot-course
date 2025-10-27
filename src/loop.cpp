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

};
