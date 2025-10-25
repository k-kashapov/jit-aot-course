#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "ir.h"
#include <algorithm>

namespace IR {

class ParamOp : public Op {
    virtual std::ostream &stringify(std::ostream &os) const override {
        return os << "ParamOp " << _type;
    }

  public:
    virtual bool verify() const override { return true; }
};

class BinaryOp : public Op {
  protected:
    Op *_lhs;
    Op *_rhs;

    std::ostream &printArgs(std::ostream &os) const {
        os << '(';
        if (_lhs) {
            _lhs->printNameAndType(os);
        } else {
            os << "nullptr";
        }

        os << ", ";

        if (_rhs) {
            _rhs->printNameAndType(os);
        } else {
            os << "nullptr";
        }

        return os << ')';
    }

  public:
    BinaryOp(Op *lhs, Op *rhs) : _lhs(lhs), _rhs(rhs) {}

    auto getInputs() const { return std::pair{_lhs, _rhs}; }

    Op *getLhs() const { return _lhs; }

    Op *getRhs() const { return _rhs; }
};

#define ADD_BINARY_OP(OP_NAME)                                                                     \
    class OP_NAME : public BinaryOp {                                                              \
        virtual std::ostream &stringify(std::ostream &os) const override {                         \
            return printArgs(os << #OP_NAME " ");                                                  \
        }                                                                                          \
                                                                                                   \
      public:                                                                                      \
        OP_NAME(Op *lhs, Op *rhs) : BinaryOp(lhs, rhs) {}                                          \
        virtual bool verify() const override {                                                     \
            return _lhs && _rhs &&                             \
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

class PhiNode : public Op {   // TODO: check recursion on verify
    std::list<Op *> _sources; // TODO: add operation and bb links

  public:
    PhiNode(std::initializer_list<Op *> ops) : _sources(ops) {}

    virtual bool verify() const override {
        auto verifyOp = [*this](const Op *op) {
            return op && (op->getType() == this->_type);
        };

        return std::all_of(_sources.begin(), _sources.end(), verifyOp);
    }

    virtual std::ostream &stringify(std::ostream &os) const override {
        auto &stream = os << "PhiNode (";

        auto printSrc = [&stream](const Op *op) -> auto & {
            return stream << op->getBB()->getName() << "." << op->getId();
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
        _bb->linkSucc(_dest);
    }

    virtual bool verify() const override { return _dest != nullptr; }
};

class CondBrOp : public Op { // TODO: check successors number and branch to successors
    Op *_cond = nullptr; // TODO: check that if successors number is consistent with last op in bb
    BasicBlock *_dest = nullptr;

    virtual std::ostream &stringify(std::ostream &os) const override {
        os << "Jmp to " << _dest->getName() << " if ";
        return _cond->printNameAndType(os);
    }

  public:
    CondBrOp(Op *cond, BasicBlock *dest) : _cond(cond), _dest(dest) {}

    BasicBlock *getDest() const { return _dest; }
    void setDest(BasicBlock *bb) { _dest = bb; }

    Op *getCond() const { return _cond; }
    void setCond(Op *cond) { _cond = cond; }

    virtual void setBB(BasicBlock *bb) override {
        _bb = bb;
        _bb->linkSucc(_dest);
    }

    virtual bool verify() const override {
        return _dest != nullptr && _cond != nullptr && _cond->getType() == EType::BOOL;
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

    virtual bool verify() const override { return true; }
};

class CallOp : public Op {
    BasicBlock *_dest = nullptr;
    std::list<Op *> _params;

    virtual std::ostream &stringify(std::ostream &os) const override {
        os << "Call " << _dest->getName();
        if (_params.empty())
            return os;

        os << " (";

        for (auto p : _params) {
            p->printNameAndType(os);
            os << ", ";
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
        _bb->linkSucc(_dest);
    }

    virtual bool verify() const override {
        return _dest != nullptr;
    }
};

class RetOp : public Op {
    Op *_val = nullptr;

    virtual std::ostream &stringify(std::ostream &os) const override {
        os << "Ret ";
        return _val->printNameAndType(os);
    }

  public:
    RetOp(Op *val) : _val(val) {}

    Op *getValue() const { return _val; }
    void setValue(Op *val) { _val = val; }

    virtual bool verify() const override { return _val->verify(); }
};

} // namespace IR

#endif // OPERATIONS_H
