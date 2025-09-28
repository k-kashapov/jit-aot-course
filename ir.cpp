#include "ir.h"
#include "operations.h"
#include "types.h"

int main() {
    auto in1 = IR::Op::create<IR::ParamOp>("in1", IR::Type::EType::F16);
    auto in2 = IR::Op::create<IR::ParamOp>("in2", IR::Type::EType::F16);
    auto add = IR::Op::create<IR::AddOp>("add", IR::Type::EType::F16, in1, in2);

    std::cout << *in1 << "\n";
    std::cout << *in2 << "\n";
    std::cout << *add << "\n";

    auto bb = IR::BasicBlock::create("testBB", {in1, in2, add});

    std::cout << *bb << "\n";

    delete bb;

    return 0;
}
