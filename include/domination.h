#include "ir.h"
#include <set>
#include <unordered_map>

std::unordered_map<IR::BasicBlock *, std::set<IR::BasicBlock *>>
find_dominators(IR::BasicBlock *start, std::set<IR::BasicBlock*> allNodes);
