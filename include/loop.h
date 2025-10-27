#ifndef IR_LOOP_H
#define IR_LOOP_H

#include <domination.h>
#include <utils.h>

namespace IR {

struct Loop {
    std::set<Loop*> innerLoops;
    bbSet innerBBs;
};

using LoopMap = std::map<BB*, Loop>;

std::set<edge> collect_backedges(IR::BB* start);
LoopMap collect_loops(dominatorMap dmap, std::set<edge> backedges);

};

#endif // IR_LOOP_H
