#include <set>
#include <unordered_map>
#include <map>
#include <ir.h>

std::unordered_map<IR::BasicBlock *, std::set<IR::BasicBlock *>>
find_dominators(IR::BasicBlock *start, std::set<IR::BasicBlock*> allNodes);

std::map<IR::BasicBlock*, IR::BasicBlock*>
find_immediate_dominators(IR::BasicBlock* start, std::unordered_map<IR::BasicBlock*, std::set<IR::BasicBlock *>> dmap);
