#ifndef IR_DOMINATION_H
#define IR_DOMINATION_H

#include <unordered_map>
#include <map>
#include <ir.h>
#include <utils.h>

namespace IR {

using dominatorMap = std::unordered_map<BB *, bbSet>;

dominatorMap find_dominators(BB *start, std::set<BB*> allNodes);

std::map<BB*, BB*>
find_immediate_dominators(BB* start, dominatorMap dmap);

};
#endif // IR_DOMINATION_H
