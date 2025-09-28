#include "ir.h"

int main() {
    auto in1 = IR::Op::create<IR::ParamOp>("in1");
    auto in2 = IR::Op::create<IR::ParamOp>("in2");
    auto add = IR::Op::create<IR::AddOp>("add", in1, in2);

    std::cout << *in1 << "\n";
    std::cout << *in2 << "\n";
    std::cout << *add << "\n";

    delete in1;
    delete in2;
    delete add;

    return 0;
}
