#include <inlining.h>
#include <iostream>
#include <ir.h>
#include <operations.h>

using namespace IR;

namespace test1 {

Function buildAddFunction() {
    Function func("add");
    int64_t id = 0;

    auto *entry = BasicBlock::create(id++, "add_entry");

    auto *a = Op::create<ParamOp>(EType::SI32);
    auto *b = Op::create<ParamOp>(EType::SI32);
    entry->addOp(a);
    entry->addOp(b);

    auto *sum = Op::create<AddOp>(EType::SI32, a, b);
    entry->addOp(sum);

    auto *ret = Op::create<RetOp>(EType::None, sum);
    entry->addOp(ret);
    // entry->linkTrue(exit);

    func.addBB(entry);
    func.setEntry(entry);
    func.assignGlobalIds();
    return func;
}

Function buildCaller(Function *callee) {
    Function func("caller");
    int64_t id = 0;

    auto *entry = BasicBlock::create(id++, "caller_entry");
    func.addBB(entry);
    func.setEntry(entry);

    auto *c5 = Op::create<ConstOp>(EType::SI32, 5);
    auto *c3 = Op::create<ConstOp>(EType::SI32, 3);
    entry->addOp(c5);
    entry->addOp(c3);

    auto *call = Op::create<CallOp>(EType::SI32, callee, IR::OpRange{c5, c3});
    entry->addOp(call);

    auto *ret = Op::create<RetOp>(EType::None, call);
    entry->addOp(ret);

    func.assignGlobalIds(entry);
    return func;
}

int test1() {
    auto addFunc = buildAddFunction();
    auto caller = buildCaller(&addFunc);

    std::cout << "Before inlining:\n" << caller << std::endl;
    std::cout << addFunc << std::endl;

    std::cout << "Start inlining:\n";

    // Find the CallOp in caller's entry block
    BasicBlock *entry = caller.getEntry();
    CallOp *callOp = nullptr;
    for (auto &opPtr : entry->getOps()) {
        if (opPtr->is<CallOp>()) {
            callOp = static_cast<CallOp *>(opPtr.get());
            break;
        }
    }
    if (!callOp) {
        std::cerr << "No CallOp found!" << std::endl;
        return 1;
    }

    bool success = inlineCall(&caller, callOp);
    if (!success) {
        std::cerr << "Inlining failed!" << std::endl;
        return 1;
    }

    std::cout << "\nAfter inlining:\n" << caller << std::endl;
    return 0;
}

} // namespace test1

namespace test2 {

Function buildMaxFunction() {
    Function func("max");
    int64_t id = 0;

    auto *entry = BasicBlock::create(id++, "entry");
    auto *thenBlock = BasicBlock::create(id++, "thenBlock");
    auto *elseBlock = BasicBlock::create(id++, "elseBlock");

    func.setEntry(entry);
    func.getExits().insert(thenBlock);
    func.getExits().insert(elseBlock);

    auto *a = Op::create<ParamOp>(EType::SI32);
    auto *b = Op::create<ParamOp>(EType::SI32);
    entry->addOp(a);
    entry->addOp(b);

    auto *cmp = Op::create<GreaterOp>(EType::BOOL, a, b);
    entry->addOp(cmp);
    auto *br = Op::create<CondBrOp>(EType::None, cmp, thenBlock);
    entry->addOp(br);
    entry->linkFalse(elseBlock);

    auto *retA = Op::create<RetOp>(EType::None, a);
    thenBlock->addOp(retA);

    auto *retB = Op::create<RetOp>(EType::None, b);
    elseBlock->addOp(retB);

    func.addBB(entry);
    func.addBB(thenBlock);
    func.addBB(elseBlock);
    func.assignGlobalIds();
    return func;
}

Function buildCaller(Function *callee) {
    Function func("caller");
    int64_t id = 0;

    auto *entry = BasicBlock::create(id++, "callerEntry");
    func.addBB(entry);
    func.setEntry(entry);

    auto *c5 = Op::create<ConstOp>(EType::SI32, 5);
    auto *c3 = Op::create<ConstOp>(EType::SI32, 3);
    entry->addOp(c5);
    entry->addOp(c3);

    auto *call = Op::create<CallOp>(EType::SI32, callee, IR::OpRange{c5, c3});
    entry->addOp(call);

    auto *ret = Op::create<RetOp>(EType::None, call);
    entry->addOp(ret);

    func.assignGlobalIds();
    return func;
}

int test2() {
    auto maxFunc = buildMaxFunction();
    auto caller = buildCaller(&maxFunc);

    std::cout << "Before inlining:\n" << caller << "\nCallee:\n" << maxFunc << std::endl;

    BasicBlock *entry = caller.getEntry();
    CallOp *callOp = nullptr;
    for (auto &opPtr : entry->getOps())
        if ((callOp = dynamic_cast<CallOp *>(opPtr.get())))
            break;

    if (!callOp || !inlineCall(&caller, callOp))
        std::cerr << "Inlining failed!" << std::endl;
    else
        std::cout << "\nAfter inlining:\n" << caller << std::endl;

    return 0;
}

} // namespace test2

int main() {
    std::cerr << "TEST 1\n";
    test1::test1();
    std::cerr << "TEST 2\n";
    test2::test2();
}
