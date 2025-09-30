#include "ir.h"
#include "operations.h"
#include "types.h"

int main() {
    auto n = IR::Op::create<IR::ParamOp>("n", IR::Type::EType::UI32);

    auto start = IR::BasicBlock::create("start", {n});

    auto one = IR::Op::create<IR::ConstOp>("one", IR::Type::EType::UI32, 1);
    start->addOp(one);
    auto ifeq1 = IR::Op::create<IR::EqOp>("nEq1", IR::Type::EType::BOOL, n, one);
    start->addOp(ifeq1);

    auto zero = IR::Op::create<IR::ConstOp>("zero", IR::Type::EType::UI32, 0);
    start->addOp(zero);
    auto ifeq0 = IR::Op::create<IR::EqOp>("nEq0", IR::Type::EType::BOOL, n, zero);
    start->addOp(ifeq0);

    auto zeroOrOne = IR::Op::create<IR::OrOp>("0or1", IR::Type::EType::BOOL, ifeq0, ifeq1);
    start->addOp(zeroOrOne);

    one = IR::Op::create<IR::ConstOp>("one", IR::Type::EType::UI32, 1);
    auto sub1 = IR::Op::create<IR::SubOp>("n-1", IR::Type::EType::UI32, n, one);

    auto call = IR::Op::create<IR::CallOp>("call", IR::Type::EType::UI32, start, std::initializer_list{static_cast<IR::Op*>(sub1)});
    auto loopBB = IR::BasicBlock::create("loopBB", {sub1, call});

    loopBB->addOp(one);
    start->linkSucc(loopBB);
    
    auto phiN = IR::Op::create<IR::PhiNode>("phiN", IR::Type::EType::UI32, std::initializer_list<IR::Op*>{n, sub1});
    start->insertOp(one, phiN);

    auto ret = IR::Op::create<IR::RetOp>("ret", IR::Type::EType::None, one);
    auto exitBB = IR::BasicBlock::create("exitBB", {ret});

    auto condBr = IR::Op::create<IR::CondBrOp>("cjmp", IR::Type::EType::None, zeroOrOne, exitBB);
    start->addOp(condBr);

    std::cout << *start << "\n";
    std::cout << *loopBB << "\n";
    std::cout << *exitBB << "\n";

    delete start;
    delete loopBB;
    delete exitBB;
}
