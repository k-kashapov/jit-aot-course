#include <cassert>
#include <iostream>
#include <set>
#include <vector>

#include "ir.h"
#include "operations.h"
#include "loop.h"
#include "liveInterval.h"

#define MAKE_BB(NAME)                                      \
    auto NAME = IR::Rewriter(id++, #NAME, {});             \
    func.addBB(NAME.bb())

void test1() {
    int64_t id = 0;
    IR::Function func("test1");

    MAKE_BB(entry);
    MAKE_BB(block);
    MAKE_BB(exit);

    entry.bb()->linkTrue(block.bb());
    block.bb()->linkTrue(exit.bb());

    auto* a   = entry.createOp<IR::ConstOp>(IR::EType::SI32, 42);
    auto* b   = entry.createOp<IR::ConstOp>(IR::EType::SI32, 100);
    auto* use = block.createOp<IR::AddOp>(IR::EType::SI32, a, a);
    auto* ret = exit.createOp<IR::RetOp>(IR::EType::None, use);

    // func.assignGlobalIds(entry.bb());

    IR::LiveIntervalAnalyzer analyzer;
    analyzer.analyze(&func, entry.bb());

    auto& intervals = analyzer.intervals;

    auto it = intervals.find(a);
    assert(it != intervals.end());
    const auto& ivals_a = it->second;
    bool found0_1 = false, found2_2 = false;
    for (auto& iv : ivals_a) {
        if (iv.first == 0 && iv.second == 1) found0_1 = true;
        if (iv.first == 2 && iv.second == 2) found2_2 = true;
    }
    assert(found0_1 && found2_2);

    it = intervals.find(b);
    assert(it != intervals.end());
    assert(it->second.size() == 1);
    assert(it->second[0].first == 1 && it->second[0].second == 1);

    it = intervals.find(use);
    assert(it != intervals.end());
    bool found2 = false, found3 = false;
    for (auto& iv : it->second) {
        if (iv.first == 2 && iv.second == 2) found2 = true;
        if (iv.first == 3 && iv.second == 3) found3 = true;
    }
    assert(found2 && found3);

    it = intervals.find(ret);
    assert(it != intervals.end());
    assert(it->second.size() == 1);
    assert(it->second[0].first == 3 && it->second[0].second == 3);
}

void test2() {
    int64_t id = 0;
    IR::Function func("test2");

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

    auto* cond = entry.createOp<IR::ConstOp>(IR::EType::BOOL, 1);
    auto* br   = entry.createOp<IR::CondBrOp>(IR::EType::None, cond, ifT.bb());
    entry.bb()->linkFalse(ifF.bb());

    auto* xT   = ifT.createOp<IR::ConstOp>(IR::EType::SI32, 10);
    auto* jmpT = ifT.createOp<IR::JumpOp>(IR::EType::None, merge.bb());

    auto* xF   = ifF.createOp<IR::ConstOp>(IR::EType::SI32, 20);
    auto* jmpF = ifF.createOp<IR::JumpOp>(IR::EType::None, merge.bb());

    auto* phi = merge.createOp<IR::PhiNode>(IR::EType::SI32);
    phi->addSource(ifT.bb(), xT);
    phi->addSource(ifF.bb(), xF);

    auto* ret = exit.createOp<IR::RetOp>(IR::EType::None, phi);

    func.assignGlobalIds(entry.bb());

    IR::LiveIntervalAnalyzer analyzer;
    analyzer.analyze(&func, entry.bb());

    int64_t phi_id = phi->getGlobalId();
    bool xT_ok = false, xF_ok = false;
    for (auto& iv : analyzer.intervals[xT])
        if (iv.second == phi_id) xT_ok = true;
    for (auto& iv : analyzer.intervals[xF])
        if (iv.second == phi_id) xF_ok = true;
    assert(xT_ok && xF_ok);

    int64_t ret_id = ret->getGlobalId();
    bool phi_def = false, phi_use = false;
    for (auto& iv : analyzer.intervals[phi]) {
        if (iv.first == phi_id && iv.second == phi_id) phi_def = true;
        if (iv.first == ret_id && iv.second == ret_id) phi_use = true;
    }
    assert(phi_def && phi_use);
}

void test3() {
    int64_t id = 0;
    IR::Function func("test3");

    MAKE_BB(entry);
    MAKE_BB(header);
    MAKE_BB(body);
    MAKE_BB(exit);

    entry.bb()->linkTrue(header.bb());
    header.bb()->linkTrue(body.bb());
    header.bb()->linkFalse(exit.bb());
    body.bb()->linkTrue(header.bb());

    auto* a    = entry.createOp<IR::ConstOp>(IR::EType::SI32, 0);
    auto* ten  = entry.createOp<IR::ConstOp>(IR::EType::SI32, 10);
    auto* jmpE = entry.createOp<IR::JumpOp>(IR::EType::None, header.bb());

    auto* i_phi = header.createOp<IR::PhiNode>(IR::EType::SI32);
    i_phi->addSource(entry.bb(), a);
    auto* cmp = header.createOp<IR::GreaterOp>(IR::EType::BOOL, i_phi, ten);
    auto* br  = header.createOp<IR::CondBrOp>(IR::EType::None, cmp, exit.bb());
    header.bb()->linkFalse(body.bb());

    auto* i_next = body.createOp<IR::AddOp>(IR::EType::SI32, i_phi, a);
    auto* jmpB   = body.createOp<IR::JumpOp>(IR::EType::None, header.bb());

    i_phi->addSource(body.bb(), i_next);

    auto* ret = exit.createOp<IR::RetOp>(IR::EType::None, i_phi);

    func.assignGlobalIds(entry.bb());

    IR::LiveIntervalAnalyzer analyzer;
    analyzer.analyze(&func, entry.bb());

    auto loopMap = IR::FindAllLoops(func, entry.bb());
    assert(loopMap.size() == 1);
    assert(loopMap.count(header.bb()));

    const auto& loop = loopMap[header.bb()];
    int64_t loopEndId = header.bb()->getOps().back()->getGlobalId();
    for (auto* bb : loop.innerBBs) {
        if (!bb->getOps().empty()) {
            int64_t last = bb->getOps().back()->getGlobalId();
            if (last > loopEndId) loopEndId = last;
        }
    }

    int64_t headerStart = header.bb()->getOps().front()->getGlobalId();
    bool found = false;
    for (auto& iv : analyzer.intervals[a]) {
        if (iv.first <= headerStart && iv.second >= loopEndId)
            found = true;
    }
    assert(found);
}

int main() {
    std::cout << "Test1 "; test1(); std::cout << "clear\n";
    std::cout << "Test2 "; test2(); std::cout << "clear\n";
    std::cout << "Test3 "; test3(); std::cout << "clear\n";
    std::cout << "All tests passed.\n";
    return 0;
}
