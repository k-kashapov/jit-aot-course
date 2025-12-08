#include <cassert>
#include <loop.h>

#define MAKE_BB(NAME) auto NAME = IR::Rewriter(#NAME, {}); func.addBB(NAME.bb())

void printMap(IR::LoopMap loops) {
    for (auto iter : loops) {
        auto header = iter.first;
        auto loop = iter.second;
        std::cout << "header: " << header->getName() << "\n";
        std::cout << "\tinner bbs: \n";
        for (auto* innerBB : loop.innerBBs) {
            std::cout << "\t\t" << innerBB->getName() << "\n";
        }

        std::cout << "\tinner loops: \n";
        for (auto* inLoop : loop.innerLoops) {
            if (!inLoop->header) {
                std::cout << "\t\tnullheader\n";
                continue;
            }

            std::cout << "\t\t" << inLoop->header->getName() << "\n";
        }
    }
}

// auto testLoops(std::set<IR::BB*> allNodes, IR::BB* start) {
//     for (auto *node : allNodes) {
//         // std::cout << *node << "\n";
//     }

//     auto dominators = find_dominators(start, allNodes);

//     auto backs = IR::collect_backedges(start);
//     for (auto p : backs) {
//         // std::cout << "backedge: " << p.first->getName() << " -> " << p.second->getName() << "\n";
//     }

//     auto loops = IR::collect_loops(dominators, backs);
//     for (auto l : loops) {
//         std::cout << "loop: header = " << l.first->getName() << "\n\tlatches: "; 
//         for (auto latch : l.second.innerBBs) {
//             std::cout << latch->getName() << " ";
//         }
//         std::cout << '\n';
//     }

//     std::vector<IR::BB*> allNodesPostorder;
//     auto savePO = [&allNodesPostorder](IR::BB* bb){ allNodesPostorder.push_back(bb); };
//     IR::postorder(start, savePO);

//     std::map<IR::BB*, IR::Loop*> bbToLoopMapping;

//     // std::cout << "postorder: "; 
//     for (auto node : allNodesPostorder) {
//         for (auto backedge : backs) {
//             if (backedge.second == node) {
//                 // This is a latch
//                 // std::cout << "This is a latch: " << backedge.first->getName() << " -> " << node->getName() << "\n";

//                 std::vector<IR::BB*> dfsOrder;
//                 auto saveDFS = [&dfsOrder](IR::BB* bb){ dfsOrder.push_back(bb); };
//                 IR::reverse_dfs(backedge.first, saveDFS, node);

//                 std::cout << " dfs found: ";
//                 for (auto bbToAdd : dfsOrder) {
//                     std::cout << bbToAdd->getName() << "<-";

//                     auto visitedIter = bbToLoopMapping.find(bbToAdd);
//                     if (visitedIter != bbToLoopMapping.end()) {
//                         loops[node].innerLoops.insert(visitedIter->second);
//                     } else {
//                         loops[node].innerBBs.insert(bbToAdd);
//                         bbToLoopMapping[bbToAdd] = &loops[node];
//                     }
//                 }

//                 std::cout << node->getName() << "\n";
//                 break;
//             }
//         }   
//     }
//     // std::cout << '\n';
//     return loops;
// }

void test1() {
    IR::Function func("test1");

    MAKE_BB(a);
    MAKE_BB(b);
    MAKE_BB(c);
    MAKE_BB(d);
    MAKE_BB(e);
    MAKE_BB(f);
    MAKE_BB(g);

    a->linkTrue(b.bb());
    b->linkTrue(c.bb());
    b->linkFalse(f.bb());
    c->linkTrue(d.bb());
    f->linkTrue(e.bb());
    f->linkFalse(g.bb());
    e->linkTrue(d.bb());
    g->linkTrue(d.bb());

    auto loop_map = IR::FindAllLoops(func, a.bb());
    assert(loop_map.empty());
}

void test2() {
    IR::Function func("test2");

    MAKE_BB(a);
    MAKE_BB(b);
    MAKE_BB(c);
    MAKE_BB(d);
    MAKE_BB(e);
    MAKE_BB(f);
    MAKE_BB(g);
    MAKE_BB(h);
    MAKE_BB(i);
    MAKE_BB(j);
    MAKE_BB(k);

    a->linkTrue(b.bb());
    b->linkTrue(c.bb());
    c->linkTrue(d.bb());
    d->linkTrue(e.bb());
    e->linkTrue(f.bb());
    f->linkTrue(g.bb());
    g->linkTrue(i.bb());
    h->linkTrue(b.bb());
    i->linkTrue(k.bb());

    g->linkFalse(h.bb());
    f->linkFalse(e.bb());
    d->linkFalse(c.bb());
    b->linkFalse(j.bb());
    j->linkFalse(c.bb());

    auto loop_map = IR::FindAllLoops(func, a.bb());
    // printMap(loop_map);

    assert(loop_map[e.bb()].innerBBs == (std::set<IR::BB*>{f.bb()}));
    assert(loop_map[c.bb()].innerBBs == (std::set<IR::BB*>{d.bb()}));
    assert(loop_map[b.bb()].innerBBs == (std::set<IR::BB*>{c.bb(), e.bb(), g.bb(), h.bb(), j.bb()}));
    assert(loop_map[b.bb()].innerLoops.size() == 2);
}

void test3() {
    IR::Function func("test3");

    MAKE_BB(a);
    MAKE_BB(b);
    MAKE_BB(c);
    MAKE_BB(d);
    MAKE_BB(e);
    MAKE_BB(f);
    MAKE_BB(g);
    MAKE_BB(h);
    MAKE_BB(i);

    a->linkTrue(b.bb());
    b->linkTrue(c.bb());
    c->linkTrue(d.bb());
    d->linkTrue(g.bb());
    g->linkTrue(i.bb());
    
    g->linkFalse(c.bb());

    b->linkFalse(e.bb());
    e->linkTrue(f.bb());
    f->linkTrue(h.bb());
    h->linkTrue(i.bb());
    
    e->linkFalse(d.bb());

    h->linkFalse(g.bb());

    f->linkFalse(b.bb());

    auto loop_map = IR::FindAllLoops(func, a.bb());
    // printMap(loop_map);

    assert(loop_map[b.bb()].innerBBs == (std::set<IR::BB*>{f.bb(), e.bb()}));
    assert(!loop_map.contains(c.bb()));
}

void test4() {
    IR::Function func("test4");

    MAKE_BB(a);
    MAKE_BB(b);
    MAKE_BB(c);
    MAKE_BB(d);
    MAKE_BB(e);

    a->linkTrue(b.bb());
    b->linkTrue(c.bb());
    b->linkFalse(d.bb());
    d->linkTrue(e.bb());
    e->linkFalse(b.bb());

    auto loop_map = IR::FindAllLoops(func, a.bb());
    // printMap(loop_map);
    assert(loop_map[b.bb()].innerBBs == (std::set<IR::BB*>{e.bb(), d.bb()}));
    assert(loop_map[b.bb()].innerLoops.size() == 0);
}

void test5() {
    IR::Function func("test5");

    MAKE_BB(a);
    MAKE_BB(b);
    MAKE_BB(c);
    MAKE_BB(d);
    MAKE_BB(e);
    MAKE_BB(f);

    a->linkTrue(b.bb());
    b->linkTrue(c.bb());
    c->linkTrue(d.bb());
    c->linkFalse(f.bb());
    d->linkFalse(f.bb());
    d->linkTrue(e.bb());
    e->linkTrue(b.bb());

    auto loop_map = IR::FindAllLoops(func, a.bb());
    // printMap(loop_map);
    assert(loop_map[b.bb()].innerBBs == (std::set<IR::BB*>{e.bb(), d.bb(), c.bb()}));
    assert(loop_map[b.bb()].innerLoops.size() == 0);
}

void test6() {
    IR::Function func("test6");

    MAKE_BB(a);
    MAKE_BB(b);
    MAKE_BB(c);
    MAKE_BB(d);
    MAKE_BB(e);
    MAKE_BB(f);
    MAKE_BB(g);
    MAKE_BB(h);

    a->linkTrue(b.bb());
    b->linkTrue(c.bb());
    b->linkFalse(d.bb());

    c->linkTrue(e.bb());
    c->linkFalse(f.bb());

    d->linkTrue(f.bb());

    f->linkTrue(g.bb());
    g->linkTrue(h.bb());

    g->linkFalse(b.bb());
    h->linkFalse(a.bb());

    auto loop_map = IR::FindAllLoops(func, a.bb());

    assert(loop_map[b.bb()].innerBBs == (std::set<IR::BB*>{g.bb(), f.bb(), c.bb(), d.bb()}));
    assert(loop_map[b.bb()].innerLoops.empty());

    assert(loop_map[a.bb()].innerBBs == (std::set<IR::BB*>{b.bb(), h.bb()}));
    assert(loop_map[a.bb()].innerLoops.size() == 1);
}

int main() {
    std::cout << "Test1 ";
    test1();
    std::cout << "clear\n";
    std::cout << "Test2 ";
    test2();
    std::cout << "clear\n";
    std::cout << "Test3 ";
    test3();
    std::cout << "clear\n";
    std::cout << "Test4 ";
    test4();
    std::cout << "clear\n";
    std::cout << "Test5 ";
    test5();
    std::cout << "clear\n";
    std::cout << "Test6 ";
    test6();
    std::cout << "clear\n";

    return 0;
}
