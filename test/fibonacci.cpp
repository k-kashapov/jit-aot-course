#include "ir.h"
#include "operations.h"
#include "types.h"

int main() {
    auto n = IR::Op::create<IR::ParamOp>(IR::EType::UI32);
    auto start = IR::Rewriter("start", {n});

    auto one = start.createOp<IR::ConstOp>(IR::EType::UI32, 1);
    auto ifeq1 = start.createOp<IR::EqOp>(IR::EType::BOOL, n, one);

    auto zero = start.createOp<IR::ConstOp>(IR::EType::UI32, 0);
    auto ifeq0 = start.createOp<IR::EqOp>(IR::EType::BOOL, n, zero);

    auto zeroOrOne = start.createOp<IR::OrOp>(IR::EType::BOOL, ifeq0, ifeq1);

    one = IR::Op::create<IR::ConstOp>(IR::EType::UI32, 1);
    auto sub1 = IR::Op::create<IR::SubOp>(IR::EType::UI32, n, one);

    auto call = IR::Op::create<IR::CallOp>(IR::EType::UI32, start.bb(),
                                           IR::Op::Range{sub1});
    auto loopBB = IR::Rewriter("loopBB", {sub1, call});

    loopBB->addOp(one);
    start->linkFalse(loopBB.bb());

    auto phiN =
        IR::Op::create<IR::PhiNode>(IR::EType::UI32, IR::Op::Range{n, sub1});
    start->insertOp(one, phiN);

    auto ret = IR::Op::create<IR::RetOp>(IR::EType::None, one);
    auto exitBB = IR::Rewriter("exitBB", {ret});

    start->linkTrue(exitBB.bb());

    auto condBr = IR::Op::create<IR::CondBrOp>(IR::EType::None, zeroOrOne, exitBB.bb());
    start->addOp(condBr);

    start.bb()->sort();
    loopBB.bb()->sort();
    exitBB.bb()->sort();

    std::cout << *start << "\n";
    std::cout << *loopBB << "\n";
    std::cerr << *exitBB << "\n";
}
