#ifndef IR_H
#define IR_H

#include <algorithm>
#include <concepts>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_set>
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
    // function-wide ID
    int64_t _globalId = -1;

    // id inside basic block
    int64_t _blockId;

    BasicBlock *_bb;
    Type _type;

    Op() : _blockId(-1), _bb(nullptr), _type(EType::SI32) {};
    Op(int64_t id) : _blockId(id), _bb(nullptr), _type(EType::SI32) {}

    virtual std::ostream &stringify(std::ostream &os) const = 0;

  public:
    template <typename Ty, class... Args>
        requires std::is_base_of_v<Op, Ty>
    static Ty *create(Type ty, Args... args) {
        auto res = new Ty(args...);
        res->_type = ty;
        return res;
    }

    virtual bool verify() const = 0;

    std::ostream &printNameAndType(std::ostream &os) const;

    friend std::ostream &operator<<(std::ostream &os, const Op &op) {
        op.printNameAndType(os) << " = ";
        return op.stringify(os);
    }

    virtual void setBB(BasicBlock *bb) { _bb = bb; }

    BasicBlock *getBB() const { return _bb; }

    Type getType() const { return _type; }

    void setBlockId(int64_t id) { _blockId = id; }
    auto getBlockId() const { return _blockId; }
    void setGlobalId(int64_t id) { _globalId = id; }
    int64_t getGlobalId() const { return _globalId; }

    virtual std::vector<Op *> getOperands() const { return {}; }

    template <typename ConcreteOp> bool is() { return dynamic_cast<ConcreteOp *>(this) != nullptr; }

    virtual ~Op() {};
};

class BasicBlock {
    using opPtr = std::unique_ptr<Op>;

    int64_t _id;

    int64_t _ops_free_id = 0;
    std::vector<BasicBlock *> _preds;
    std::optional<std::string> _name;
    std::list<opPtr> _ops;
    struct Successors {
        BasicBlock *T = nullptr;
        BasicBlock *F = nullptr;
    } _cond_succ;

    BasicBlock(int64_t id) : _id(id) {}
    BasicBlock(int64_t id, std::initializer_list<Op *> ops) : _id(id) {
        for (auto *op : ops) {
            addOp(op);
        }
    }

    BasicBlock(int64_t id, std::string_view name) : _id(id), _name(name) {}
    BasicBlock(int64_t id, std::string_view name, std::initializer_list<Op *> ops) : _name(name) {
        BasicBlock(id, ops);
    }

    void addPred(BasicBlock *bb) { _preds.push_back(bb); }
    void removePred(BasicBlock *bb) {
        auto iter = std::find(_preds.begin(), _preds.end(), bb);
        if (iter != _preds.end()) {
            _preds.erase(iter);
        }
    }

    void setSucc(BasicBlock *bb) { _cond_succ.T = bb; }
    void setCondFailSucc(BasicBlock *bb) { _cond_succ.F = bb; }

    // live at entry of this block
    std::set<Op *> liveIn;

  public:
    static BasicBlock *create(int64_t id, std::string_view name) {
        return new BasicBlock(id, name);
    }

    static BasicBlock *create(int64_t id, std::string_view name, std::initializer_list<Op *> ops) {
        return new BasicBlock(id, name, ops);
    }

    auto insertOp(std::list<opPtr>::const_iterator pos, Op *op) {
        auto ret = _ops.insert(pos, opPtr(op));
        op->setBB(this);
        op->setBlockId(_ops_free_id++);
        return ret;
    }

    auto insertOp(Op *pos, Op *op) {
        auto iter = _ops.begin();
        for (; iter != _ops.end(); ++iter) {
            if (iter->get() == pos) {
                break;
            }
        }
        if (iter == _ops.end()) {
            throw std::runtime_error("insertOp: pos not found in this basic block");
        }
        return insertOp(iter, op);
    }

    std::list<opPtr>::iterator addOp(Op *op) {
        auto ret = insertOp(_ops.end(), op);
        return ret;
    }

    void linkTrue(BasicBlock *bb) {
        if (!bb) {
            throw std::runtime_error("Linking to nullptr basic block is not allowed");
        }

        if (_cond_succ.T != bb) {
            if (_cond_succ.T) {
                _cond_succ.T->removePred(this);
            }
            setSucc(bb);
            bb->addPred(this);
        }
    }

    void linkFalse(BasicBlock *bb) {
        if (!bb) {
            throw std::runtime_error("Linking to nullptr basic block is not allowed");
        }

        if (_cond_succ.F != bb) {
            if (_cond_succ.F) {
                _cond_succ.F->removePred(this);
            }
            setCondFailSucc(bb);
            bb->addPred(this);
        }
    }

    bool hasName() const { return _name.has_value(); }

    const std::string_view getName() const { return _name.value(); }

    int64_t getId() const { return _id; }

    const std::pair<BasicBlock *, BasicBlock *> getSuccessors() const {
        return std::pair(_cond_succ.T, _cond_succ.F);
    }

    const std::vector<BasicBlock *> &getPreds() const { return _preds; }

    friend std::ostream &operator<<(std::ostream &os, const BasicBlock &bb) {
        auto printName = [](decltype(bb._preds.begin()) i) { return (*i)->getName(); };

        if (bb.hasName()) {
            os << bb.getName();
        } else {
            os << bb.getId();
        }

        printWithSeparator(os, bb._preds.begin(), bb._preds.end(), printName, " (Preds: ", ", ",
                           ") ");

        os << " (Succs: ";
        os << (bb._cond_succ.T ? (bb._cond_succ.T->getName()) : "none") << ", ";
        os << (bb._cond_succ.F ? (bb._cond_succ.F->getName()) : "none") << "):\n";

        for (auto &op : bb._ops) {
            os << '\t' << *op << '\n';
        }

        return os;
    }

    const std::list<opPtr> &getOps() { return _ops; }

    void sort() {
        auto cmp = [](opPtr &a, opPtr &b) { return a->getBlockId() < b->getBlockId(); };
        _ops.sort(cmp);
    }

    std::set<Op *> &getLiveIn() { return liveIn; }
};

class Rewriter {
    BasicBlock *_bb;
    std::list<std::unique_ptr<Op>>::const_iterator _insertPoint;

  public:
    Rewriter(int64_t id, std::string_view name, std::initializer_list<IR::Op *> ops)
        : _bb(BasicBlock::create(id, name, ops)), _insertPoint(_bb->getOps().end()) {}

    template <typename OpTy, typename... Args>
        requires requires(Type ty, Args... args) { Op::create<OpTy>(ty, args...); }
    auto createOp(Type ty, Args... args) {
        auto *op = Op::create<OpTy>(ty, args...);
        _insertPoint = _bb->addOp(op);
        return op;
    }

    template <typename OpTy, typename... Args>
        requires requires(Type ty, Args... args) { Op::create<OpTy>(ty, args...); }
    auto createOp(std::list<std::unique_ptr<Op>>::const_iterator at, Type ty, Args... args) {
        auto *op = Op::create<OpTy>(ty, args...);
        _bb->insertOp(at, op);
        return op;
    }

    auto operator->() { return _bb; }

    BasicBlock *bb() { return _bb; }

    auto &operator*() { return *_bb; }

    friend std::ostream &operator<<(std::ostream &os, const Rewriter &r) { return os << *r._bb; }
};

void postorder(BasicBlock *bb, std::function<void(BasicBlock *)> fn);

class Function {
    std::string _name;
    std::set<BasicBlock *> _bbs;
    BasicBlock *_entry = nullptr;

  public:
    Function(const std::string_view name) : _name(name) {}

    const std::string &getName() const { return _name; }

    const std::set<BasicBlock *> &getBBs() const { return _bbs; }

    void setName(const std::string_view name) { _name = name; }

    void addBB(BasicBlock *bb) { _bbs.insert(bb); }

    friend std::ostream &operator<<(std::ostream &os, const Function &f) {
        os << "Function " << f._name << "\n";

        if (f._entry) {
            std::vector<BasicBlock *> rpo;
            auto savePO = [&rpo](BasicBlock *bb) { rpo.push_back(bb); };
            postorder(f._entry, savePO);

            std::reverse(rpo.begin(), rpo.end()); // now reverse postorder
            for (auto *bb : rpo) {
                os << *bb << "\n";
            }
        } else {
            for (auto *bb : f._bbs) {
                os << *bb << "\n";
            }
        }

        return os;
    }

    void assignGlobalIds(BasicBlock *entry) {
        _entry = entry;
        std::vector<BasicBlock *> rpo;
        auto savePO = [&rpo](BasicBlock *bb) { rpo.push_back(bb); };
        postorder(entry, savePO);

        std::reverse(rpo.begin(), rpo.end()); // now reverse postorder

        int64_t nextId = 0;
        for (auto *bb : rpo) {

            for (auto &opPtr : bb->getOps()) {
                // std::cerr << *opPtr << "\n";
                opPtr->setGlobalId(nextId++);
            }
        }
    }

    // FIXME: this is bullshit
    ~Function() {
        for (auto *bb : _bbs) {
            delete bb;
        }
    }
};

} // namespace IR

#endif // IR_H
