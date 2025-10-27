#include <domination.h>

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

    std::cout << a << '\n';
    std::cout << b << '\n';
    std::cout << c << '\n';
    std::cout << d << '\n';
    std::cout << e << '\n';
    std::cout << f << '\n';
    std::cout << g << '\n';
    auto dominators = find_dominators(a.bb(), allNodes);

    std::cout << "Found dominators:\n";
    for (const auto &[key, value] : dominators) {
        std::cout << key->getName() << " dominators: ";
        for (const auto *v : value) {
            std::cout << v->getName() << ' ';
        }
        std::cout << '\n';
    }

    auto immdoms = find_immediate_dominators(a.bb(), dominators);
    for (const auto &[key, value] : immdoms) {
        // std::cout << key << ", " << value << "\n";
        std::cout << key->getName() << " immediate dominator: ";
        std::cout << (value ? value->getName() : "none") << "\n";
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

    std::cout << a << '\n';
    std::cout << b << '\n';
    std::cout << c << '\n';
    std::cout << d << '\n';
    std::cout << e << '\n';
    std::cout << f << '\n';
    std::cout << g << '\n';
    std::cout << h << '\n';
    std::cout << i << '\n';
    std::cout << j << '\n';
    std::cout << k << '\n';
    auto dominators = find_dominators(a.bb(), allNodes);

    std::cout << "Found dominators:\n";
    for (const auto &[key, value] : dominators) {
        std::cout << key->getName() << " dominators: ";
        for (const auto *v : value) {
            std::cout << v->getName() << ' ';
        }
        std::cout << '\n';
    }

    auto immdoms = find_immediate_dominators(a.bb(), dominators);
    for (const auto &[key, value] : immdoms) {
        // std::cout << key << ", " << value << "\n";
        std::cout << key->getName() << " immediate dominator: ";
        std::cout << (value ? value->getName() : "none") << "\n";
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

    std::cout << a << '\n';
    std::cout << b << '\n';
    std::cout << c << '\n';
    std::cout << d << '\n';
    std::cout << e << '\n';
    std::cout << f << '\n';
    std::cout << g << '\n';
    std::cout << h << '\n';
    std::cout << i << '\n';
    auto dominators = find_dominators(a.bb(), allNodes);

    std::cout << "Found dominators:\n";
    for (const auto &[key, value] : dominators) {
        std::cout << key->getName() << " dominators: ";
        for (const auto *v : value) {
            std::cout << v->getName() << ' ';
        }
        std::cout << '\n';
    }

    auto immdoms = find_immediate_dominators(a.bb(), dominators);
    for (const auto &[key, value] : immdoms) {
        // std::cout << key << ", " << value << "\n";
        std::cout << key->getName() << " immediate dominator: ";
        std::cout << (value ? value->getName() : "none") << "\n";
    }
}

int main() {
    std::cout << "Test1\n";
    test1();
    std::cout << "Test2\n";
    test2();
    std::cout << "Test3\n";
    test3();

    return 0;
}
