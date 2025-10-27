#include <domination.h>

namespace IR {

using bbSet = std::set<BB*>;
using edge = std::pair<BB*, BB*>;
using Loops = std::map<BB*, bbSet>;

std::set<edge> collect_backedges(IR::BB* start);
Loops collect_loops(dominatorMap dmap, std::set<edge> backedges);

};
