#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "ir.h"

namespace IR {

class ParamOp : public Op {
    virtual std::ostream &stringify(std::ostream &os, const Op &op [[maybe_unused]]) const override {
        return os << "ParamOp" << '[' << _type << ']';
    }

    virtual bool verify() const override {
        return true;
    }
};

class BinaryOp : public Op {
protected:
    Op *_lhs;
    Op *_rhs;

    std::ostream &printArgs(std::ostream &os, const Op& op [[maybe_unused]]) const {
        os << "($";
        if (_lhs) {
            os << _lhs->getName() << "[" << _lhs->getType() << "]";
        } else {
            os << "nullptr";
        }

        os << ", $";

        if (_rhs) {
            os << _rhs->getName() << "[" << _rhs->getType() << "]";
        } else {
            os << "nullptr";
        }

        return os << ')';
    }

public:
    BinaryOp(Op *lhs, Op *rhs) : _lhs(lhs), _rhs(rhs) {}

    const std::vector<Op *> getInputs() const { return std::vector<Op *>{_lhs, _rhs}; }

    Op *getLhs() const { return _lhs; }

    Op *getRhs() const { return _rhs; }
};

class AddOp : public BinaryOp {
  public:
    AddOp(Op *lhs, Op *rhs) : BinaryOp(lhs, rhs) {}

    virtual bool verify() const override {
        return _lhs && _rhs && _lhs->verify() && _rhs->verify() && (_lhs->getType() == _rhs->getType()) && (_rhs->getType() == _type);
    }

    virtual std::ostream &stringify(std::ostream &os, const Op &op) const override {
        return printArgs(os << "AddOp ", op);
    }
};

} // namespace IR

#endif // OPERATIONS_H
