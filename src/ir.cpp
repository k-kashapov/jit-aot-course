#include "ir.h"

namespace IR {

std::ostream &Op::printNameAndType(std::ostream &os) const {
    return os << '$' << _bb->getName() << '.' << _blockId << _type;
}

} // namespace IR
