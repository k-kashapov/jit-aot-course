#include <ir.h>
#include <operations.h>
#include <types.h>

namespace IR {

Function *buildFibonacci() {
    auto *func = new Function("fibonacci");

    // Create blocks
    auto *entry = BasicBlock::create("entry");
    auto *header = BasicBlock::create("loop_header");
    auto *body = BasicBlock::create("loop_body");
    auto *exit = BasicBlock::create("exit");

    func->addBB(entry);
    func->addBB(header);
    func->addBB(body);
    func->addBB(exit);

    // ---------- Entry ----------
    auto *n = Op::create<ParamOp>(EType::SI32);
    entry->addOp(n);
    auto *zero = Op::create<ConstOp>(EType::SI32, 0);
    entry->addOp(zero);
    auto *one = Op::create<ConstOp>(EType::SI32, 1);
    entry->addOp(one);
    auto *two = Op::create<ConstOp>(EType::SI32, 2);
    entry->addOp(two);
    auto *jmp = Op::create<JumpOp>(EType::None, header);
    entry->addOp(jmp);

    // ---------- Loop header ----------
    auto *i_phi = Op::create<PhiNode>(EType::SI32);
    header->addOp(i_phi);
    auto *prev_phi = Op::create<PhiNode>(EType::SI32);
    header->addOp(prev_phi);
    auto *curr_phi = Op::create<PhiNode>(EType::SI32);
    header->addOp(curr_phi);

    auto *cmp = Op::create<GreaterOp>(EType::BOOL, i_phi, n);
    header->addOp(cmp);
    auto *br = Op::create<CondBrOp>(EType::None, cmp, exit);
    header->addOp(br);
    header->linkFalse(body); // set false successor explicitly

    // ---------- Loop body ----------
    auto *new_val = Op::create<AddOp>(EType::SI32, prev_phi, curr_phi);
    body->addOp(new_val);
    auto *i_next = Op::create<AddOp>(EType::SI32, i_phi, one);
    body->addOp(i_next);
    auto *back = Op::create<JumpOp>(EType::None, header);
    body->addOp(back);

    // ---------- Exit ----------
    auto *ret = Op::create<RetOp>(EType::None, curr_phi);
    exit->addOp(ret);

    // ---------- Set phi sources ----------
    // i_phi:   from entry: 2 ; from body: i_next
    i_phi->addSource(entry, two);
    i_phi->addSource(body, i_next);

    // prev_phi: from entry: 0 ; from body: curr_phi
    prev_phi->addSource(entry, zero);
    prev_phi->addSource(body, curr_phi);

    // curr_phi: from entry: 1 ; from body: new_val
    curr_phi->addSource(entry, one);
    curr_phi->addSource(body, new_val);

    return func;
}

} // namespace IR

int main() {
    auto fib = std::unique_ptr<IR::Function>(IR::buildFibonacci());
    std::cout << *fib << std::endl;
    return 0;
}
