#ifndef IR_H
#define IR_H

#include <algorithm>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "types.h"

namespace IR {

class BasicBlock;

class Op {
  protected:
    std::string _name;
    BasicBlock *_bb;
    Type _type;

    Op() : _type(Type::EType::SI32) {};
    Op(const std::string &name) : _name(name), _type(Type::EType::SI32) {}

    virtual std::ostream &stringify(std::ostream &os) const = 0;

  public:
    template <typename Ty, class... Args>
    // requires(std::is_base_of<Op, Ty>::value)
    static Ty *create(const std::string &name, Type ty, Args... args) {
        auto res = new Ty(args...);
        res->_name = name;
        res->_type = ty;
        if (!res->verify()) {
            std::cerr << "Invalid operation: " << res << "\n";
            delete res;
            return nullptr;
        }

        return res;
    }

    virtual bool verify() const = 0;

    friend std::ostream &operator<<(std::ostream &os, const Op &op) {
        return op.stringify(os << '$' << op._name << '[' << op._type << "] = ");
    }

    virtual void setBB(BasicBlock *bb) { _bb = bb; }

    BasicBlock *getBB() const { return _bb; }

    Type getType() const { return _type; }
    const std::string &getName() const { return _name; }

    virtual ~Op() {};
};

class BasicBlock {
    std::list<BasicBlock *> _preds;
    std::string _name;
    std::list<std::unique_ptr<Op>> _ops;
    std::list<BasicBlock *> _succs;

    BasicBlock(std::string_view name) : _name(name) {}
    BasicBlock(std::string_view name, std::initializer_list<Op *> ops) : _name(name) {
        for (auto *op : ops) {
            op->setBB(this);
            _ops.push_back(std::unique_ptr<Op>(op));
        }
    }

  public:
    static BasicBlock *create(std::string_view name) { return new BasicBlock(name); }

    static BasicBlock *create(std::string_view name, std::initializer_list<Op *> ops) {
        return new BasicBlock(name, ops);
    }

    auto &addOp(Op *op) {
        op->setBB(this);
        _ops.push_back(std::unique_ptr<Op>(op));
        return _ops.back();
    }

    BasicBlock *addPred(BasicBlock *bb) {
        _preds.push_back(bb);
        return _preds.back();
    }

    BasicBlock *addSucc(BasicBlock *bb) {
        _succs.push_back(bb);
        return _succs.back();
    }

    const std::string_view getName() const { return _name; }

    const std::list<BasicBlock *> &getSuccessors() const { return _succs; }

    const std::list<BasicBlock *> &getPreds() const { return _preds; }

    friend std::ostream &operator<<(std::ostream &os, const BasicBlock &bb) {
        auto &stream = os << bb._name << " (Preds: ";

        for (auto pred : bb._preds) {
            stream << pred->getName() << ", ";
        }

        stream << "\b\b) (Succs: ";

        for (auto succ : bb._succs) {
            stream << succ->getName() << ", ";
        }

        stream << "\b\b):\n";

        for (auto &op : bb._ops) {
            stream << '\t' << *op << '\n';
        }

        return stream;
    }
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
