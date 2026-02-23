#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <algorithm>
#include <cassert>
#include <vector>

#include "ir.h"
#include "utils.h"

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

    std::vector<Op*> getOperands() const override { return {_lhs, _rhs}; }

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
            return _lhs && _rhs && (_lhs->getType() == _rhs->getType());                           \
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
    using Source = std::pair<BasicBlock *, Op *>;

    std::vector<Source> _sources;

  public:
    PhiNode() {}
    PhiNode(std::initializer_list<Source> srcs) : _sources(srcs) {}
    PhiNode(IR::OpRange ops) {
        _sources.reserve(ops.size());
        for (auto *op : ops) {
            assert(op->getBB() != nullptr &&
                   "Operation must have basic block assigned to be added to phi node\n");
            _sources.push_back(Source{op->getBB(), op});
        };
    }

    std::vector<Op*> getOperands() const override {
        std::vector<Op*> res;
        res.reserve(_sources.size());
        for (const auto &source: _sources) {
            res.push_back(source.second);
        }
        return res;
    }

    using const_iterator = std::vector<Source>::const_iterator;
    const_iterator begin() const { return _sources.begin(); }
    const_iterator end() const { return _sources.end(); }
    size_t size() const { return _sources.size(); }

    virtual bool verify() const override {
        auto verifyOp = [self = this](const Source &src) {
            const auto *bb = src.first;
            const auto *op = src.second;
            return bb && op && (op->getBB() == bb) && (op->getType() == self->_type);
        };

        return std::all_of(_sources.begin(), _sources.end(), verifyOp);
    }

    Op* getInputForPredecessor(BasicBlock* pred) const {
        for (auto& src : _sources)
            if (src.first == pred) return src.second;
        return nullptr;
    }

    virtual std::ostream &stringify(std::ostream &os) const override {
        auto &stream = os << "PhiNode (";

        auto printSrc = [&stream](const std::pair<BasicBlock *, Op *> &src) -> auto & {
            return stream << src.first->getName() << "." << src.second->getBlockId();
        };

        for (auto src = _sources.begin(); src != std::prev(_sources.end()); src++) {
            printSrc(*src) << ", ";
        }

        return printSrc(_sources.back()) << ")";
    }

    void addSource(BasicBlock *bb, Op *src) {
        assert(bb != nullptr && src != nullptr);
        _sources.push_back(Source{bb, src});
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
        _bb->linkTrue(_dest);
    }

    virtual bool verify() const override { return _dest != nullptr; }
};

class CondBrOp : public Op {
    Op *_cond = nullptr;
    BasicBlock *_dest = nullptr;

    virtual std::ostream &stringify(std::ostream &os) const override {
        os << "Jmp to " << _dest->getName() << " if ";
        _cond->printNameAndType(os);
        os << " else to " << getBB()->getSuccessors().second->getName();
        return os;
    }

  public:
    CondBrOp(Op *cond, BasicBlock *dest) : _cond(cond), _dest(dest) {}

    BasicBlock *getDest() const { return _dest; }
    void setDest(BasicBlock *bb) { _dest = bb; }

    Op *getCond() const { return _cond; }
    void setCond(Op *cond) { _cond = cond; }

    std::vector<Op*> getOperands() const override {
        return { _cond };
    }

    virtual void setBB(BasicBlock *bb) override {
        _bb = bb;
        _bb->linkTrue(_dest);
    }

    virtual bool verify() const override {
        const auto succs = getBB()->getSuccessors();
        return _dest != nullptr && _cond != nullptr && _cond->getType() == EType::BOOL &&
               succs.first && succs.second;
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

        return os << ")";
    }

  public:
    CallOp(BasicBlock *dest) : _dest(dest) {}
    CallOp(BasicBlock *dest, IR::OpRange params) : _dest(dest), _params(params) {}

    std::vector<Op*> getOperands() const override {
        return { _params.begin(), _params.end() };
    }

    BasicBlock *getDest() const { return _dest; }

    void setDest(BasicBlock *bb) { _dest = bb; }

    virtual void setBB(BasicBlock *bb) override {
        _bb = bb;
        _bb->linkTrue(_dest);
    }

    virtual bool verify() const override { return _dest != nullptr; }
};

class RetOp : public Op {
    Op *_val = nullptr;

    virtual std::ostream &stringify(std::ostream &os) const override {
        os << "Ret ";
        return _val->printNameAndType(os);
    }

  public:
    RetOp(Op *val) : _val(val) {}

    std::vector<Op*> getOperands() const override {
        return {_val};
    }

    Op *getValue() const { return _val; }
    void setValue(Op *val) { _val = val; }

    virtual bool verify() const override { return _val->verify(); }
};

} // namespace IR

#endif // OPERATIONS_H
