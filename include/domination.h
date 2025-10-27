#include <functional>
#include <set>
#include <unordered_map>
#include <map>
#include <ir.h>

std::unordered_map<IR::BasicBlock *, std::set<IR::BasicBlock *>>
find_dominators(IR::BasicBlock *start, std::set<IR::BasicBlock*> allNodes);

std::map<IR::BasicBlock*, IR::BasicBlock*>
find_immediate_dominators(IR::BasicBlock* start, std::unordered_map<IR::BasicBlock*, std::set<IR::BasicBlock *>> dmap);

std::vector<IR::BasicBlock*> bfs(IR::BasicBlock* bb, std::function<void (IR::BasicBlock*)> fn);
std::vector<IR::BasicBlock*> dfs(IR::BasicBlock* start, std::function<void (IR::BasicBlock*)> fn);
