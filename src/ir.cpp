#include "ir.h"

namespace IR {

std::ostream &Op::printNameAndType(std::ostream &os) const {
    return os << '$' << _bb->getName() << '.' << _globalId << _type;
}

} // namespace IR
