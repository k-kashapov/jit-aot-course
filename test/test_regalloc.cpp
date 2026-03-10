#include <cassert>
#include <iostream>
#include <set>

#include "ir.h"
#include "live_interval.h"
#include "operations.h"
#include "regalloc.h"

#define MAKE_BB(NAME)                                                                              \
    auto NAME = IR::Rewriter(id++, #NAME, {});                                                     \
    func.addBB(NAME.bb());                                                                         \
    allNodes.insert(NAME.bb())

void test1() {
    int64_t id = 0;
    IR::Function func("test1");
    std::set<IR::BasicBlock *> allNodes;

    MAKE_BB(entry);
    MAKE_BB(block);
    MAKE_BB(exit);

    entry.bb()->linkTrue(block.bb());
    block.bb()->linkTrue(exit.bb());

    auto *a = entry.createOp<IR::ConstOp>(IR::EType::SI32, 1);
    auto *b = entry.createOp<IR::ConstOp>(IR::EType::SI32, 2);
    auto *c = entry.createOp<IR::ConstOp>(IR::EType::SI32, 3);
    auto *add1 = block.createOp<IR::AddOp>(IR::EType::SI32, a, b);
    auto *add2 = block.createOp<IR::AddOp>(IR::EType::SI32, add1, c);
    exit.createOp<IR::RetOp>(IR::EType::None, add2);

    func.assignGlobalIds(entry.bb());

    IR::LiveIntervalAnalyzer analyzer;
    analyzer.analyze(&func, entry.bb());

    IR::LinearScan regAlloc(2); // two physical registers
    regAlloc.allocate(analyzer.intervals);

    const auto &intervals = regAlloc.getIntervals();
    for (const auto &iv : intervals) {
        // Every interval must have either a register or a spill slot
        assert(iv.reg != -1 || iv.spillSlot != -1);
    }

    // With two registers and no high pressure, no spills should occur
    bool hasSpill = false;
    for (const auto &iv : intervals) {
        if (iv.spillSlot != -1)
            hasSpill = true;
    }
    assert(!hasSpill);
}

void test2() {
    int64_t id = 0;
    IR::Function func("test2");
    std::set<IR::BasicBlock *> allNodes;

    MAKE_BB(entry);
    MAKE_BB(ifT);
    MAKE_BB(ifF);
    MAKE_BB(merge);
    MAKE_BB(exit);

    entry.bb()->linkTrue(ifT.bb());
    entry.bb()->linkFalse(ifF.bb());
    ifT.bb()->linkTrue(merge.bb());
    ifF.bb()->linkTrue(merge.bb());
    merge.bb()->linkTrue(exit.bb());

    auto *cond = entry.createOp<IR::ConstOp>(IR::EType::BOOL, 1);
    entry.createOp<IR::CondBrOp>(IR::EType::None, cond, ifT.bb());
    entry.bb()->linkFalse(ifF.bb());

    auto *xT = ifT.createOp<IR::ConstOp>(IR::EType::SI32, 10);
    ifT.createOp<IR::JumpOp>(IR::EType::None, merge.bb());

    auto *xF = ifF.createOp<IR::ConstOp>(IR::EType::SI32, 20);
    ifF.createOp<IR::JumpOp>(IR::EType::None, merge.bb());

    auto *phi = merge.createOp<IR::PhiNode>(IR::EType::SI32);
    phi->addSource(ifT.bb(), xT);
    phi->addSource(ifF.bb(), xF);

    exit.createOp<IR::RetOp>(IR::EType::None, phi);

    func.assignGlobalIds(entry.bb());

    IR::LiveIntervalAnalyzer analyzer;
    analyzer.analyze(&func, entry.bb());

    IR::LinearScan regAlloc(2);
    regAlloc.allocate(analyzer.intervals);

    const auto &intervals = regAlloc.getIntervals();
    for (const auto &iv : intervals) {
        assert(iv.reg != -1 || iv.spillSlot != -1);
    }

    // xT and xF should not be simultaneously live, so they can share a register
    // We can check that they have the same register if they both got registers
    auto findReg = [&](IR::Op *op) -> int {
        for (const auto &iv : intervals)
            if (iv.vreg == op)
                return iv.reg;
        return -2;
    };
    int regT = findReg(xT);
    int regF = findReg(xF);
    if (regT != -1 && regF != -1) {
        assert(regT == regF);
    }
}

void test3() {
    int64_t id = 0;
    IR::Function func("test3");
    std::set<IR::BasicBlock *> allNodes;

    MAKE_BB(entry);
    MAKE_BB(header);
    MAKE_BB(body);
    MAKE_BB(exit);

    entry.bb()->linkTrue(header.bb());
    header.bb()->linkTrue(body.bb());
    header.bb()->linkFalse(exit.bb());
    body.bb()->linkTrue(header.bb());

    auto *a = entry.createOp<IR::ConstOp>(IR::EType::SI32, 0);
    auto *n = entry.createOp<IR::ConstOp>(IR::EType::SI32, 10);
    auto *one = entry.createOp<IR::ConstOp>(IR::EType::SI32, 1);
    entry.createOp<IR::JumpOp>(IR::EType::None, header.bb());

    auto *i_phi = header.createOp<IR::PhiNode>(IR::EType::SI32);
    i_phi->addSource(entry.bb(), a);
    auto *acc_phi = header.createOp<IR::PhiNode>(IR::EType::SI32);
    acc_phi->addSource(entry.bb(), a);
    auto *cmp = header.createOp<IR::GreaterOp>(IR::EType::BOOL, i_phi, n);
    header.createOp<IR::CondBrOp>(IR::EType::None, cmp, exit.bb());
    header.bb()->linkFalse(body.bb());

    auto *i_next = body.createOp<IR::AddOp>(IR::EType::SI32, i_phi, one);
    auto *acc_next = body.createOp<IR::AddOp>(IR::EType::SI32, acc_phi, i_phi);
    body.createOp<IR::JumpOp>(IR::EType::None, header.bb());

    i_phi->addSource(body.bb(), i_next);
    acc_phi->addSource(body.bb(), acc_next);

    exit.createOp<IR::RetOp>(IR::EType::None, acc_phi);

    func.assignGlobalIds(entry.bb());

    IR::LiveIntervalAnalyzer analyzer;
    analyzer.analyze(&func, entry.bb());

    IR::LinearScan regAlloc(2);
    regAlloc.allocate(analyzer.intervals);

    const auto &intervals = regAlloc.getIntervals();
    for (const auto &iv : intervals) {
        assert(iv.reg != -1 || iv.spillSlot != -1);
    }

    // With only two registers, some intervals may be spilled; just ensure allocation completed.
    // Optionally, count registers used vs spilled.
    int regCount = 0, spillCount = 0;
    for (const auto &iv : intervals) {
        if (iv.reg != -1)
            regCount++;
        if (iv.spillSlot != -1)
            spillCount++;
    }
    // At least some registers should be used (otherwise everything spilled)
    assert(regCount > 0);
    assert(spillCount == 0);
}

int main() {
    std::cout << "Test1\n";
    test1();
    std::cout << "Test2\n";
    test2();
    std::cout << "Test3\n";
    test3();
    std::cout << "All tests passed.\n";
    return 0;
}
