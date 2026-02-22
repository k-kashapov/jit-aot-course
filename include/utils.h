#ifndef IR_UTILS_H
#define IR_UTILS_H

#include <functional>
#include <ir.h>
#include <set>

namespace IR {

using BB = IR::BasicBlock;
using bbSet = std::set<BB *>;
using edge = std::pair<BB *, BB *>;
using bbFunc = std::function<void(BB *)>;
using OpRange = std::initializer_list<Op *>;

std::vector<BB *> bfs(BB *bb, bbFunc fn);
void dfs(BB *start, bbFunc fn);
void reverse_dfs(BB *start, bbFunc fn, BB *end);
void postorder(BB *bb, bbFunc fn);

}; // namespace IR

#endif // IR_UTILS_H
