#ifndef IR_H
#define IR_H

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include <types.h>

namespace IR {

class BasicBlock;

template <typename IteratorT, std::invocable<IteratorT> FuncT>
static inline void printWithSeparator(std::ostream &os, IteratorT begin, IteratorT end, FuncT fn,
                               std::string_view prefix = "", std::string_view separator = "",
                               std::string_view suffix = "") {
    if (begin == end) {
        os << prefix;
        os << suffix;
        return;
    }

    os << prefix;

    auto last = std::prev(end);
    for (auto i = begin; i != last; i++) {
        os << fn(i) << separator;
    }

    os << fn(last) << suffix;
}

class Op {
  protected:
    int64_t _id = -1;
    BasicBlock *_bb = nullptr;
    Type _type = EType::None;

    Op() : _type(EType::SI32) {};
    Op(int64_t id) : _id(id), _type(EType::SI32) {}

    virtual std::ostream &stringify(std::ostream &os) const = 0;

  public:
    using Range = std::initializer_list<Op*>;

    template <typename Ty, class... Args>
    requires std::is_base_of_v<Op, Ty>
    static Ty *create(Type ty, Args... args) {
        auto res = new Ty(args...);
        res->_type = ty;
        return res;
    }

    virtual bool verify() const = 0;

    std::ostream &printNameAndType(std::ostream &os) const { return os << '$' << _id << _type; }

    friend std::ostream &operator<<(std::ostream &os, const Op &op) {
        op.printNameAndType(os) << " = ";
        return op.stringify(os);
    }

    virtual void setBB(BasicBlock *bb) { _bb = bb; }

    BasicBlock *getBB() const { return _bb; }

    Type getType() const { return _type; }
    
    void setId(int64_t id) { _id = id; }
    auto getId() const { return _id; }

    virtual ~Op() {};
};

class BasicBlock {
    using opPtr = std::unique_ptr<Op>;

    int64_t _ops_free_id = 0;
    std::vector<BasicBlock *> _preds;
    std::string _name;
    std::list<opPtr> _ops;
    BasicBlock * _cond_true_succ = nullptr;
    BasicBlock * _cond_fail_succ = nullptr;

    BasicBlock(std::string_view name) : _name(name) {}
    BasicBlock(std::string_view name, std::initializer_list<Op *> ops) : _name(name) {
        for (auto *op : ops) {
            addOp(op);
        }
    }

    void addPred(BasicBlock *bb) { _preds.push_back(bb); }
    void setSucc(BasicBlock *bb) { _cond_true_succ = bb; }
    void setCondFailSucc(BasicBlock *bb) { _cond_fail_succ = bb; }

  public:
    static BasicBlock *create(std::string_view name) { return new BasicBlock(name); }

    static BasicBlock *create(std::string_view name, std::initializer_list<Op *> ops) {
        return new BasicBlock(name, ops);
    }

    auto insertOp(std::list<opPtr >::const_iterator pos, Op *op) {
        auto ret = _ops.insert(pos, opPtr(op));
        op->setBB(this);
        op->setId(_ops_free_id++);
        return ret;
    }

    auto insertOp(Op *pos, Op *op) {
        auto iter = _ops.begin();
        for (; iter != _ops.end(); iter++) {
            if (iter->get()->getId() == pos->getId()) {
                break;
            }
        }

        return insertOp(iter, op);
    }

    std::list<opPtr>::iterator addOp(Op *op) {
        auto ret = insertOp(_ops.end(), op);
        return ret;
    }

    void linkTrue(BasicBlock *bb) {
        setSucc(bb);
        bb->addPred(this);
    }

    void linkFalse(BasicBlock *bb) {
        setCondFailSucc(bb);
        bb->addPred(this);
    }

    const std::string_view getName() const { return _name; }

    const std::pair<BasicBlock *, BasicBlock *> getSuccessors() const { return {_cond_true_succ, _cond_fail_succ}; }

    const std::vector<BasicBlock *> &getPreds() const { return _preds; }

    friend std::ostream &operator<<(std::ostream &os, const BasicBlock &bb) {
        auto printName = [](decltype(bb._preds.begin()) i) { return (*i)->getName(); };

        os << bb.getName();
        printWithSeparator(os, bb._preds.begin(), bb._preds.end(), printName, " (Preds: ", ", ",
                           ") ");

        os << " (Succs: ";
        os << (bb._cond_true_succ ? (bb._cond_true_succ->getName()) : "none") << ", ";
        os << (bb._cond_fail_succ ? (bb._cond_fail_succ->getName()) : "none") << "):\n";

        for (auto &op : bb._ops) {
            os << '\t' << *op << '\n';
        }

        return os;
    }

    const std::list<opPtr> &getOps() {
        return _ops;
    }

    void sort() {
        auto cmp = [](opPtr& a, opPtr& b) {return a->getId() < b->getId(); };
        _ops.sort(cmp);
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

class Rewriter {
    std::unique_ptr<BasicBlock> _bb;
    std::list<std::unique_ptr<Op> >::const_iterator _insertPoint;

public:
    Rewriter(std::string_view name, std::initializer_list<IR::Op *> ops) : _bb(BasicBlock::create(name, ops)), _insertPoint(_bb->getOps().end()) { }

    template <typename OpTy, typename... Args>
    requires requires (Type ty, Args... args) { Op::create<OpTy>(ty, args...); }
    auto createOp(Type ty, Args... args) {
        auto *op = Op::create<OpTy>(ty, args...);
        _insertPoint = _bb->insertOp(_insertPoint, op);
        return op;
    }

    auto operator->() {
        return _bb.get();
    }

    BasicBlock *bb() {
        return _bb.get();
    }

    auto &operator*() {
        return *_bb;
    }

    friend std::ostream &operator<<(std::ostream &os, const Rewriter& r) {
        return os << *r._bb;
    }
};

} // namespace IR

#endif // IR_H
