#ifndef IR_UTILS_H
#define IR_UTILS_H

#include <functional>
#include <set>
#include <ir.h>

namespace IR {

using BB = IR::BasicBlock;
using bbSet = std::set<BB *>;
using edge = std::pair<BB*, BB*>;
using bbFunc = std::function<void (BB*)>;

std::vector<BB*> bfs(BB* bb, bbFunc fn);
void dfs(BB* start, bbFunc fn);
void reverse_dfs(BB* start, bbFunc fn, BB* end);
void postorder(BB* bb, bbFunc fn);

}; // namespace IR

#endif // IR_UTILS_H
