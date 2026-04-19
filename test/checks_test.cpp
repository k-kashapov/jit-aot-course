#include <cassert>
#include <iostream>
#include <ir.h>
#include <operations.h>
#include <checks.h>

using namespace IR;

void test_same_block() {
    Function f("test");
    auto *entry = BasicBlock::create(0, "entry");
    f.addBB(entry);
    f.setEntry(entry);

    auto *v = Op::create<ConstOp>(EType::SI32, 42);
    entry->addOp(v);
    auto *c1 = Op::create<NullCheck>(EType::None, v);
    auto *c2 = Op::create<NullCheck>(EType::None, v);
    entry->addOp(c1);
    entry->addOp(c2);
    entry->addOp(Op::create<RetOp>(EType::None, v));
    f.assignGlobalIds();

    std::cerr << "\nBEFORE OPT:\n" << f << "\n";

    optimizeChecks(&f);
    int cnt = 0;
    for (auto &op : entry->getOps())
        if (op->is<NullCheck>()) cnt++;
    assert(cnt == 1);

    std::cerr << "AFTER OPT:\n" << f << "\n";
}

void test_dominator() {
    Function f("test");
    auto *entry = BasicBlock::create(0, "entry");
    auto *then = BasicBlock::create(1, "then");
    f.addBB(entry);
    f.addBB(then);
    f.setEntry(entry);

    auto *v = Op::create<ConstOp>(EType::SI32, 100);
    entry->addOp(v);
    auto *c1 = Op::create<NullCheck>(EType::None, v);
    entry->addOp(c1);
    entry->addOp(Op::create<JumpOp>(EType::None, then));
    entry->linkTrue(then);

    auto *c2 = Op::create<NullCheck>(EType::None, v);
    then->addOp(c2);
    then->addOp(Op::create<RetOp>(EType::None, v));
    f.assignGlobalIds();

    std::cerr << "\nBEFORE OPT:\n" << f << "\n";

    optimizeChecks(&f);
    assert(c1->getBB() == entry);
    assert(then->getOps().size() == 1);

    std::cerr << "AFTER OPT:\n" << f << "\n";
}

void test_bound_merge() {
    Function f("test");
    auto *entry = BasicBlock::create(0, "entry");
    f.addBB(entry);
    f.setEntry(entry);

    auto *v = Op::create<ConstOp>(EType::SI32, 5);
    entry->addOp(v);
    auto *b1 = Op::create<BoundCheck>(EType::None, v, 3, 10);
    auto *b2 = Op::create<BoundCheck>(EType::None, v, 0, 8);
    entry->addOp(b1);
    entry->addOp(b2);
    entry->addOp(Op::create<RetOp>(EType::None, v));
    f.assignGlobalIds();

    std::cerr << "\nBEFORE OPT:\n" << f << "\n";

    optimizeChecks(&f);
    BoundCheck *survivor = nullptr;
    for (auto &op : entry->getOps())
        if ((survivor = dynamic_cast<BoundCheck*>(op.get()))) break;
    assert(survivor);
    assert(survivor->getLower() == 3);
    assert(survivor->getUpper() == 8);

    std::cerr << "AFTER OPT:\n" << f << "\n";
}

void test_diamond_no_opt() {
    Function f("diamond");
    auto *entry = BasicBlock::create(0, "entry");
    auto *left = BasicBlock::create(1, "left");
    auto *right = BasicBlock::create(2, "right");
    auto *merge = BasicBlock::create(3, "merge");
    f.addBB(entry);
    f.addBB(left);
    f.addBB(right);
    f.addBB(merge);
    f.setEntry(entry);

    auto *v = Op::create<ConstOp>(EType::SI32, 42);
    entry->addOp(v);
    auto *cond = Op::create<ConstOp>(EType::BOOL, 1);
    entry->addOp(cond);
    auto *br = Op::create<CondBrOp>(EType::None, cond, left);
    entry->addOp(br);
    entry->linkFalse(right);

    auto *c1 = Op::create<NullCheck>(EType::None, v);
    left->addOp(c1);
    left->addOp(Op::create<JumpOp>(EType::None, merge));
    left->linkTrue(merge);

    auto *c2 = Op::create<NullCheck>(EType::None, v);
    right->addOp(c2);
    right->addOp(Op::create<JumpOp>(EType::None, merge));
    right->linkTrue(merge);

    merge->addOp(Op::create<RetOp>(EType::None, v));
    f.assignGlobalIds();

    std::cerr << "\nBEFORE OPT:\n" << f << "\n";
    optimizeChecks(&f);
    std::cerr << "AFTER OPT:\n" << f << "\n";

    // Neither block dominates the other, so both checks survive
    bool hasLeft = false, hasRight = false;
    for (auto &op : left->getOps())
        if (op->is<NullCheck>()) hasLeft = true;
    for (auto &op : right->getOps())
        if (op->is<NullCheck>()) hasRight = true;
    assert(hasLeft && hasRight);
}

int main() {
    test_same_block();
    test_dominator();
    test_bound_merge();
    test_diamond_no_opt();
    std::cout << "All tests passed.\n";
    return 0;
}
