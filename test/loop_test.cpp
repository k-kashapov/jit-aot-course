#include <cassert>
#include <loop.h>

#define MAKE_BB(NAME) auto NAME = IR::Rewriter(#NAME, {}); allNodes.insert(NAME.bb())

void test1() {
    std::set<IR::BasicBlock*> allNodes;

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

    for (auto *node : allNodes) {
        std::cout << *node << "\n";
    }

    auto dominators = find_dominators(a.bb(), allNodes);
    auto immdoms = find_immediate_dominators(a.bb(), dominators);

    assert(immdoms[a.bb()] == nullptr);
    assert(immdoms[b.bb()] == a.bb());
    assert(immdoms[c.bb()] == b.bb());
    assert(immdoms[d.bb()] == b.bb());
    assert(immdoms[e.bb()] == f.bb());
    assert(immdoms[f.bb()] == b.bb());
    assert(immdoms[g.bb()] == f.bb());

    auto backs = IR::collect_backedges(a.bb());
    for (auto p : backs) {
        std::cout << "backedge: " << p.first->getName() << " -> " << p.second->getName() << "\n";
    }

    auto loops = IR::collect_loops(dominators, backs);
    for (auto l : loops) {
        std::cout << "loop: header = " << l.first->getName() << ";\n\tlatches: "; 
        for (auto latch : l.second) {
            std::cout << latch->getName() << " ";
        }
        std::cout << '\n';
    }
}

void test2() {
    std::set<IR::BasicBlock*> allNodes;
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

    for (auto *node : allNodes) {
        std::cout << *node << "\n";
    }

    auto dominators = find_dominators(a.bb(), allNodes);
    auto immdoms = find_immediate_dominators(a.bb(), dominators);

    assert(immdoms[a.bb()] == nullptr);
    assert(immdoms[b.bb()] == a.bb());
    assert(immdoms[c.bb()] == b.bb());
    assert(immdoms[d.bb()] == c.bb());
    assert(immdoms[e.bb()] == d.bb());
    assert(immdoms[f.bb()] == e.bb());
    assert(immdoms[g.bb()] == f.bb());
    assert(immdoms[h.bb()] == g.bb());
    assert(immdoms[i.bb()] == g.bb());
    assert(immdoms[j.bb()] == b.bb());
    assert(immdoms[k.bb()] == i.bb());

    auto backs = IR::collect_backedges(a.bb());
    for (auto p : backs) {
        std::cout << "backedge: " << p.first->getName() << " -> " << p.second->getName() << "\n";
    }

    auto loops = IR::collect_loops(dominators, backs);
    for (auto l : loops) {
        std::cout << "loop: header = " << l.first->getName() << ";\n\tlatches: "; 
        for (auto latch : l.second) {
            std::cout << latch->getName() << " ";
        }
        std::cout << '\n';
    }
}

void test3() {
    std::set<IR::BasicBlock*> allNodes;
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

    for (auto *node : allNodes) {
        std::cout << *node << "\n";
    }

    auto dominators = find_dominators(a.bb(), allNodes);
    auto immdoms = find_immediate_dominators(a.bb(), dominators);

    assert(immdoms[a.bb()] == nullptr);
    assert(immdoms[b.bb()] == a.bb());
    assert(immdoms[c.bb()] == b.bb());
    assert(immdoms[d.bb()] == b.bb());
    assert(immdoms[e.bb()] == b.bb());
    assert(immdoms[f.bb()] == e.bb());
    assert(immdoms[g.bb()] == b.bb());
    assert(immdoms[h.bb()] == f.bb());
    assert(immdoms[i.bb()] == b.bb());

    auto backs = IR::collect_backedges(a.bb());
    for (auto p : backs) {
        std::cout << "backedge: " << p.first->getName() << " -> " << p.second->getName() << "\n";
    }

    auto loops = IR::collect_loops(dominators, backs);
    for (auto l : loops) {
        std::cout << "loop: header = " << l.first->getName() << ";\n\tlatches: "; 
        for (auto latch : l.second) {
            std::cout << latch->getName() << " ";
        }
        std::cout << '\n';
    }
}

void test4() {
    std::set<IR::BasicBlock*> allNodes;
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

    for (auto *node : allNodes) {
        std::cout << *node << "\n";
    }

    auto dominators = find_dominators(a.bb(), allNodes);
    auto immdoms = find_immediate_dominators(a.bb(), dominators);

    auto backs = IR::collect_backedges(a.bb());
    for (auto p : backs) {
        std::cout << "backedge: " << p.first->getName() << " -> " << p.second->getName() << "\n";
    }

    auto loops = IR::collect_loops(dominators, backs);
    for (auto l : loops) {
        std::cout << "loop: header = " << l.first->getName() << ";\n\tlatches: "; 
        for (auto latch : l.second) {
            std::cout << latch->getName() << " ";
        }
        std::cout << '\n';
    }
}

void test5() {
    std::set<IR::BasicBlock*> allNodes;
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

    for (auto *node : allNodes) {
        std::cout << *node << "\n";
    }

    auto dominators = find_dominators(a.bb(), allNodes);
    auto immdoms = find_immediate_dominators(a.bb(), dominators);

    auto backs = IR::collect_backedges(a.bb());
    for (auto p : backs) {
        std::cout << "backedge: " << p.first->getName() << " -> " << p.second->getName() << "\n";
    }

    auto loops = IR::collect_loops(dominators, backs);
    for (auto l : loops) {
        std::cout << "loop: header = " << l.first->getName() << ";\n\tlatches: "; 
        for (auto latch : l.second) {
            std::cout << latch->getName() << " ";
        }
        std::cout << '\n';
    }
}

void test6() {
    std::set<IR::BasicBlock*> allNodes;
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

    for (auto *node : allNodes) {
        std::cout << *node << "\n";
    }

    auto dominators = find_dominators(a.bb(), allNodes);
    auto immdoms = find_immediate_dominators(a.bb(), dominators);

    auto backs = IR::collect_backedges(a.bb());
    for (auto p : backs) {
        std::cout << "backedge: " << p.first->getName() << " -> " << p.second->getName() << "\n";
    }

    auto loops = IR::collect_loops(dominators, backs);
    for (auto l : loops) {
        std::cout << "loop: header = " << l.first->getName() << ";\n\tlatches: "; 
        for (auto latch : l.second) {
            std::cout << latch->getName() << " ";
        }
        std::cout << '\n';
    }
}

int main() {
    std::cout << "Test1\n";
    test1();
    std::cout << "Test2\n";
    test2();
    std::cout << "Test3\n";
    test3();
    std::cout << "Test4\n";
    test4();
    std::cout << "Test5\n";
    test5();
    std::cout << "Test6\n";
    test6();

    return 0;
}
