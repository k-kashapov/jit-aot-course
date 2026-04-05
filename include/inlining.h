#ifndef INLINING_H
#define INLINING_H

#include "ir.h"
#include "operations.h"

namespace IR {

bool inlineCall(Function *func, CallOp *callOp);

} // namespace IR

#endif // INLINING_H