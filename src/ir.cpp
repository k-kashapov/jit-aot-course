#include "ir.h"
#include "operations.h"

namespace IR {

std::ostream &Op::printNameAndType(std::ostream &os) const {
    return os << '$' << _bb->getName() << '.' << _globalId << _type;
}

void Function::addBB(BasicBlock *bb) {
    _bbs.insert(bb);
    for (const auto &op : bb->getOps()) {
        // std::cerr << "Adding bb. Op: " << *op << "\n";
        // std::cerr << "\tIs ret = " << op->is<RetOp>() << "\n";
        if (op->is<RetOp>()) {
            _exits.insert(bb);
            break;
        }
    }
}

} // namespace IR
