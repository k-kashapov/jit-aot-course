#include <cassert>
#include <loop.h>

#define MAKE_BB(NAME) auto NAME = IR::Rewriter(#NAME, {}); func.addBB(NAME.bb())

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
//         // std::cout << "loop: header = " << l.first->getName() << "\n\tlatches: "; 
//         for (auto latch : l.second.innerBBs) {
//             // std::cout << latch->getName() << " ";
//         }
//         // std::cout << '\n';
//     }

//     std::vector<IR::BB*> postorder;
//     auto savePO = [&postorder](IR::BB* bb){ postorder.push_back(bb); };
//     IR::postorder(start, savePO);

//     std::map<IR::BB*, std::vector<IR::BB*>> latch_to_loop;

//     // std::cout << "postorder: "; 
//     for (auto node : postorder) {
//         for (auto backedge : backs) {
//             if (backedge.second == node) {
//                 // This is a latch
//                 // std::cout << "This is a latch: " << backedge.first->getName() << " -> " << node->getName() << "\n";

//                 std::vector<IR::BB*> dfsOrder;
//                 auto saveDFS = [&dfsOrder](IR::BB* bb){ dfsOrder.push_back(bb); };
//                 IR::reverse_dfs(backedge.first, saveDFS, node);

//                 std::cout << " dfs found: ";
//                 for (auto n : dfsOrder) {
//                     std::cout << n->getName() << "->";
//                 }

//                 latch_to_loop[node] = dfsOrder;

//                 std::cout << node->getName() << "\n";
//                 break;
//             }
//         }   
//     }
//     // std::cout << '\n';
//     return latch_to_loop;
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

    auto latch_to_loop = IR::FindAllLoops(func, a.bb());
    assert(latch_to_loop.empty());
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

    auto latch_to_loop = IR::FindAllLoops(func, a.bb());

    assert(latch_to_loop[e.bb()] == (std::vector<IR::BB*>{f.bb()}));
    assert(latch_to_loop[c.bb()] == (std::vector<IR::BB*>{d.bb()}));
    assert(latch_to_loop[b.bb()] == (std::vector<IR::BB*>{h.bb(), g.bb(), f.bb(), e.bb(), d.bb(), c.bb(), j.bb()}));
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

    auto latch_to_loop = IR::FindAllLoops(func, a.bb());

    assert(latch_to_loop[c.bb()] == (std::vector<IR::BB*>{g.bb(), d.bb(), e.bb(), b.bb(), a.bb(), f.bb(), h.bb()}));
    assert(latch_to_loop[b.bb()] == (std::vector<IR::BB*>{f.bb(), e.bb()}));
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

    auto latch_to_loop = IR::FindAllLoops(func, a.bb());
    assert(latch_to_loop[b.bb()] == (std::vector<IR::BB*>{e.bb(), d.bb()}));
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

    auto latch_to_loop = IR::FindAllLoops(func, a.bb());
    assert(latch_to_loop[b.bb()] == (std::vector<IR::BB*>{e.bb(), d.bb(), c.bb()}));
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

    auto latch_to_loop = IR::FindAllLoops(func, a.bb());
    assert(latch_to_loop[b.bb()] == (std::vector<IR::BB*>{g.bb(), f.bb(), c.bb(), d.bb()}));
    assert(latch_to_loop[a.bb()] == (std::vector<IR::BB*>{h.bb(), g.bb(), f.bb(), c.bb(), b.bb(), d.bb()}));
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
