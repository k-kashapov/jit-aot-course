#include "inlining.h"
#include <cassert>
#include <vector>

namespace IR {

static const size_t MAX_SIZE = 10;

bool inlineCall(Function *func, CallOp *callOp) {
    Function *dest = callOp->getDest();
    if (!dest) {
        // std::cerr << "Dest empty\n";
        return false;
    }
    if (dest->getNumOps() > MAX_SIZE) {
        // std::cerr << "Size is too big\n";
        return false;
    }

    // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";

    auto operands = callOp->getOperands();
    auto entry = dest->getEntry();
    auto &entryOps = entry->getOps();
    auto opIt = operands.begin();

    std::vector<ParamOp *> params;

    // Replace parameters with arguments
    for (auto &opPtr : entryOps) {
        auto param = opPtr.get();
        // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";
        if (param->is<ParamOp>()) {
            if (opIt == operands.end()) {
                // std::cerr << "Op it ended\n";
                return false;
            }
            param->replaceAllUsesWith(*opIt);
            ++opIt;
        }
    }

    for (auto *param : params) {
        entry->removeOp(param);
    }

    // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";

    if (opIt != operands.end()) {
        // std::cerr << "Operands unused\n";
        return false;
    }

    BasicBlock *caller = callOp->getBB();
    BasicBlock *afterCall = caller->splitAt(callOp);
    func->addBB(afterCall);

    // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";

    // Update caller's terminator to jump to callee entry
    auto &callerOps = caller->getOps();
    if (callerOps.empty()) {
        // std::cerr << "Caller empty\n";
        return false;
    }

    // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";

    // Collect return values from callee exits
    auto &exits = dest->getExits();
    // std::cerr << "Exits size = " << exits.size() << "\n";

    std::vector<std::pair<BasicBlock *, Op *>> retSources;
    for (auto *exit : exits) {
        auto &ops = exit->getOps();
        for (auto it = ops.begin(); it != ops.end(); ++it) {
            // std::cerr << "\tProcessing op : " << **it << "\n";
            if ((*it)->is<RetOp>()) {
                auto *ret = static_cast<RetOp *>(it->get());
                retSources.emplace_back(exit, ret->getValue());
                exit->removeOp(ret);
                break;
            }
        }
        // Ensure exit block has a jump to afterCall
        if (exit->getOps().empty() || !exit->getOps().back()->is<JumpOp>()) {
            auto *jmp = Op::create<JumpOp>(EType::None, afterCall);
            exit->addOp(jmp);
        } else {
            auto *jmp = static_cast<JumpOp *>(exit->getOps().back().get());
            jmp->setDest(afterCall);
        }
        exit->linkTrue(afterCall);
    }

    // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";

    // std::cerr << "Ret sources size = " << retSources.size() << "\n";

    // Create phi node for the call result
    if (callOp->getType() != EType::None && !retSources.empty()) {
        auto *phi = Op::create<PhiNode>(callOp->getType());
        for (auto [bb, val] : retSources) {
            phi->addSource(bb, val);
        }
        afterCall->insertOpFront(phi);
        callOp->replaceAllUsesWith(phi);
    } else {
        // std::cerr << "callOp is empty\n";
        callOp->replaceAllUsesWith(nullptr);
    }

    // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";

    // std::cerr << "\nCaller:\n";
    // std::cerr << *caller << "\n";
    // std::cerr << "\nAfterCall:\n";
    // std::cerr << *afterCall << "\n";

    // Remove the call instruction
    afterCall->removeOp(callOp);

    // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";

    // Transfer callee blocks to caller function
    for (auto *block : dest->getBBs()) {
        func->addBB(block);
    }

    if (caller->getOps().empty() || !caller->getOps().back()->is<JumpOp>()) {
        auto *jmp = Op::create<JumpOp>(EType::None, entry);
        caller->addOp(jmp);
    } else {
        auto *jmp = static_cast<JumpOp *>(caller->getOps().back().get());
        jmp->setDest(entry);
    }

    caller->linkTrue(entry);

    // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";

    dest->getBBs().clear();

    // Recompute block IDs and global IDs
    func->assignBlockIds();
    func->assignGlobalIds();

    // std::cerr << __PRETTY_FUNCTION__ << " (" << __LINE__ << ")\n";

    return true;
}

} // namespace IR
