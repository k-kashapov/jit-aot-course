#ifndef IR_H
#define IR_H

#include <algorithm>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace IR {

class BasicBlock;

class Op {
  protected:
    std::string _name;
    Op *_parent;
    BasicBlock *_bb;
    Op() {}
    Op(const std::string& name) : _name(name) {}

  public:
    template <typename Ty, class... Args>
        // requires(std::is_base_of<Op, Ty>::value)
    static auto create(const std::string& name, Args... args) {
        auto res = new Ty(args...);
        res->_name = name;
        return res;
    }

    virtual std::ostream& stringify(std::ostream &os, const Op& op) const = 0;

    friend std::ostream& operator<<(std::ostream &os, const Op& op) {
        return op.stringify(os << '$' << op._name << " =", op);
    }

    const std::string& getName() {
        return _name;
    }

    virtual ~Op() {};
};

class ParamOp : public Op {
    virtual std::ostream& stringify(std::ostream &os, const Op& op) const override {
        return os << " ParamOp";
    }
};

class AddOp : public Op {
    Op *_lhs;
    Op *_rhs;

  public:
    AddOp(Op *lhs, Op *rhs) : _lhs(lhs), _rhs(rhs) {};

    const std::vector<Op *> getInputs() const { return std::vector<Op *>{_lhs, _rhs}; }

    Op *getLhs() const { return _lhs; }

    Op *getRhs() const { return _rhs; }

    virtual std::ostream& stringify(std::ostream &os, const Op& op) const override {
        return os << " AddOp ($" << (_lhs ? _lhs->getName() : "nullptr") << ", $" << (_rhs ? _rhs->getName() : "nullptr") << ')';
    }
};

class BasicBlock_impl {
    std::list<BasicBlock> _preds;
    std::list<Op*> _ops;
    std::list<BasicBlock> _succs;

  public:
    BasicBlock_impl() {}

    auto addOp(Op* op) {
        _ops.push_back(op);
        return _ops.back();
    }
};

class BasicBlock {
    std::unique_ptr<BasicBlock_impl> impl;

  public:
    BasicBlock() : impl(new BasicBlock_impl) {}

    bool operator==(const BasicBlock &other) const { return impl == other.impl; }

    auto addOp(Op* op) { return impl->addOp(op); }
};

// class Function {
//     std::list<BasicBlock> bbs;

//   public:
//     Function() {}
//     Function(std::initializer_list<BasicBlock> blocks) : bbs(blocks) {}

//     BasicBlock *addBB(BasicBlock &bb) {
//         bbs.push_back(bb);
//         return &bbs.back();
//     }

//     BasicBlock *insertBB(BasicBlock &pos, BasicBlock &bb) {
//         auto iter = std::find(bbs.begin(), bbs.end(), pos);
//         auto &res = *bbs.insert(iter, bb);
//         return &res;
//     }

//     BasicBlock *insertBBAfter(BasicBlock &pos, BasicBlock &bb) {
//         auto iter = std::find(bbs.begin(), bbs.end(), pos);
//         auto &res = *bbs.insert(iter++, bb);
//         return &res;
//     }
// };

} // namespace IR

#endif // IR_H
