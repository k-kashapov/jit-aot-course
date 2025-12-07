#ifndef IR_LOOP_H
#define IR_LOOP_H

#include <domination.h>
#include <utils.h>

namespace IR {

struct Loop {
    BB* header;
    std::set<Loop*> innerLoops;
    bbSet innerBBs;
};

using LoopMap = std::map<BB*, Loop>;

std::map<BB*, BB*> collect_backedges(BB* start);
LoopMap collect_loops(dominatorMap dmap, std::map<BB*, BB*> backedges);
std::map<BB*, Loop> FindAllLoops(IR::Function& func, BB* start);

};

#endif // IR_LOOP_H
