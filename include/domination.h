#include <functional>
#include <set>
#include <unordered_map>
#include <map>
#include <ir.h>

namespace IR {

using BB = IR::BasicBlock;
using bbSet = std::set<BB *>;
using dominatorMap = std::unordered_map<BB *, bbSet>;

dominatorMap find_dominators(BB *start, std::set<BB*> allNodes);

std::map<BB*, BB*>
find_immediate_dominators(BB* start, dominatorMap dmap);

std::vector<BB*> bfs(BB* bb, std::function<void (BB*)> fn);
std::vector<BB*> dfs(BB* start, std::function<void (BB*)> fn);

};
