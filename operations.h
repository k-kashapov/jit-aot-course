#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "ir.h"

namespace IR {

class ParamOp : public Op {
    virtual std::ostream &stringify(std::ostream &os) const override {
        return os << "ParamOp" << '[' << _type << ']';
    }

  public:
    virtual bool verify() const override { return true; }
};

class BinaryOp : public Op {
  protected:
    Op *_lhs;
    Op *_rhs;

    std::ostream &printArgs(std::ostream &os) const {
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

#define ADD_BINARY_OP(OP_NAME)                                                                     \
    class OP_NAME : public BinaryOp {                                                              \
        virtual std::ostream &stringify(std::ostream &os) const override {                         \
            return printArgs(os << #OP_NAME" ");                                                    \
        }                                                                                          \
                                                                                                   \
      public:                                                                                      \
        OP_NAME(Op *lhs, Op *rhs) : BinaryOp(lhs, rhs) {}                                          \
        virtual bool verify() const override {                                                     \
            return _lhs && _rhs && _lhs->verify() && _rhs->verify() &&                             \
                   (_lhs->getType() == _rhs->getType());                                           \
        }                                                                                          \
    };

ADD_BINARY_OP(AddOp);
ADD_BINARY_OP(SubOp);
ADD_BINARY_OP(MulOp);
ADD_BINARY_OP(DivOp);
ADD_BINARY_OP(EqOp);
ADD_BINARY_OP(GreaterOp);
ADD_BINARY_OP(AndOp);
ADD_BINARY_OP(OrOp);
ADD_BINARY_OP(XorOp);

#undef ADD_BINARY_OP

class PhiNode : public Op {
    std::list<Op *> _sources;

  public:
    PhiNode(std::initializer_list<Op *> ops) : _sources(ops) {}

    virtual bool verify() const override {
        auto verifyOp = [*this](const Op *op) {
            return op && op->verify() && (op->getType() == this->_type);
        };

        return std::all_of(_sources.begin(), _sources.end(), verifyOp);
    }

    virtual std::ostream &stringify(std::ostream &os) const override {
        auto &stream = os << "PhiNode (";

        auto printSrc = [&stream](const Op *op) -> auto & {
            return stream << op->getBB()->getName() << "." << op->getName();
        };

        for (auto src = _sources.begin(); src != std::prev(_sources.end()); src++) {
            printSrc(*src) << ", ";
        }

        return printSrc(_sources.back()) << ")";
    }
};

class JumpOp : public Op {
    BasicBlock *_dest = nullptr;
    virtual std::ostream &stringify(std::ostream &os) const override {
        return os << "Jmp to " << _dest->getName();
    }

  public:
    JumpOp(BasicBlock *dest) : _dest(dest) {}

    BasicBlock *getDest() const { return _dest; }

    void setDest(BasicBlock *bb) { _dest = bb; }

    virtual void setBB(BasicBlock *bb) override {
        _bb = bb;
        _dest->addPred(_bb);
        _bb->addSucc(_dest);
    }

    virtual bool verify() const override { return _dest != nullptr; }
};

class CondBrOp : public Op {
    Op *_cond = nullptr;
    BasicBlock *_dest = nullptr;

    virtual std::ostream &stringify(std::ostream &os) const override {
        return os << "Jmp to " << _dest->getName() << " if " << '$' << _cond->getName();
    }

  public:
    CondBrOp(Op *cond, BasicBlock *dest) : _cond(cond), _dest(dest) {}

    BasicBlock *getDest() const { return _dest; }
    void setDest(BasicBlock *bb) { _dest = bb; }

    Op *getCond() const { return _cond; }
    void setCond(Op *cond) { _cond = cond; }

    virtual void setBB(BasicBlock *bb) override {
        _bb = bb;
        _dest->addPred(_bb);
        _bb->addSucc(_dest);
    }

    virtual bool verify() const override {
        return _dest != nullptr && _cond != nullptr && _cond->getType() == Type::EType::BOOL &&
               _cond->verify();
    }
};

class ConstOp : public Op {
    int64_t _value = 0;

    virtual std::ostream &stringify(std::ostream &os) const override {
        return os << "Const " << '(' << _value << ')';
    }

public:
    ConstOp(int64_t val) : _value(val) {}

    int64_t getValue() const { return _value; }
    void setValue(int64_t val) { _value = val; }

    virtual bool verify() const override {
        return true;
    }
};

class CallOp : public Op {
    BasicBlock *_dest = nullptr;
    std::list<Op *> _params;

    virtual std::ostream &stringify(std::ostream &os) const override {
        os << "Call " << _dest->getName();
        if (_params.empty()) return os;

        os << " (";

        for (auto p : _params) {
            os << '$' << p->getName() << ", ";
        }

        return os << "\b\b)";
    }

  public:
    CallOp(BasicBlock *dest) : _dest(dest) {}
    CallOp(BasicBlock *dest, std::initializer_list<Op *> params) : _dest(dest), _params(params) {}

    BasicBlock *getDest() const { return _dest; }

    void setDest(BasicBlock *bb) { _dest = bb; }

    virtual void setBB(BasicBlock *bb) override {
        _bb = bb;
        _dest->addPred(_bb);
        _bb->addSucc(_dest);
    }

    virtual bool verify() const override {
        auto verifyOp = [](Op *p){ return p->verify(); };
        return _dest != nullptr && std::all_of(_params.begin(), _params.end(), verifyOp);
    }
};

class RetOp : public Op {
    Op *_val = nullptr;

    virtual std::ostream &stringify(std::ostream &os) const override {
        return os << "Ret " << '$' << _val->getName() << '[' << _val->getType() << ']';
    }

public:
    RetOp(Op *val) : _val(val) {}

    Op* getValue() const { return _val; }
    void setValue(Op* val) { _val = val; }

    virtual bool verify() const override {
        return _val->verify();
    }
};

} // namespace IR

#endif // OPERATIONS_H
