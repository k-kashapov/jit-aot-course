#include "ir.h"
#include "operations.h"
#include "types.h"

int main() {
    auto in1 = IR::Op::create<IR::ParamOp>("in1", IR::Type::EType::F16);
    auto in2 = IR::Op::create<IR::ParamOp>("in2", IR::Type::EType::F16);
    auto add = IR::Op::create<IR::AddOp>("add", IR::Type::EType::F16, in1, in2);

    auto bb = IR::BasicBlock::create("testBB", {in1, in2, add});

    in1 = IR::Op::create<IR::ParamOp>("in1", IR::Type::EType::F16);
    in2 = IR::Op::create<IR::ParamOp>("in2", IR::Type::EType::F16);
    auto add2 = IR::Op::create<IR::AddOp>("add", IR::Type::EType::F16, in1, in2);

    auto bb2 = IR::BasicBlock::create("testBB2", {in1, in2, add2});

    auto phi = IR::Op::create<IR::PhiNode>("Phi", IR::Type::EType::F16, std::initializer_list<IR::Op*>{add, add2});

    auto bb3 = IR::BasicBlock::create("testBB3", {phi});

    auto jmp = IR::Op::create<IR::JumpOp>("jmp", IR::Type::EType::None, bb3);
    bb->addOp(jmp);

    jmp = IR::Op::create<IR::JumpOp>("jmp", IR::Type::EType::None, bb3);
    bb2->addOp(jmp);

    std::cout << *bb << "\n";
    std::cout << *bb2 << "\n";
    std::cout << *bb3 << "\n";

    delete bb;
    delete bb2;
    delete bb3;

    return 0;
}
