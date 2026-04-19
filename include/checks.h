#ifndef CHECKS_H
#define CHECKS_H

#include "domination.h"
#include "ir.h"
#include <map>

namespace IR {

class CheckOp : public Op {};

class NullCheck : public CheckOp {
    Op *_target = nullptr;

    virtual std::ostream &stringify(std::ostream &os) const override {
        os << "NullCheck (";
        _target->printNameAndType(os);
        os << ")";
        return os;
    }

  public:
    virtual bool verify() const override { return _target != nullptr; }

    NullCheck(Op *target) : _target(target) {}

    std::vector<Op *> getOperands() const override { return {_target}; }
};

class ZeroCheck : public CheckOp {
    Op *_target = nullptr;

    virtual std::ostream &stringify(std::ostream &os) const override {
        os << "ZeroCheck (";
        _target->printNameAndType(os);
        os << ")";
        return os;
    }

  public:
    virtual bool verify() const override { return _target != nullptr; }

    ZeroCheck(Op *target) : _target(target) {}

    std::vector<Op *> getOperands() const override { return {_target}; }
};

class BoundCheck : public CheckOp {
    Op *_target = nullptr;
    int64_t _lower = 0;
    int64_t _upper = 0;

    virtual std::ostream &stringify(std::ostream &os) const override {
        os << "BoundCheck (";
        _target->printNameAndType(os);
        os << ") in bounds [" << _lower << ", " << _upper << ")";
        return os;
    }

  public:
    virtual bool verify() const override { return _target != nullptr; }

    BoundCheck(Op *target, int64_t lower, int64_t upper)
        : _target(target), _lower(lower), _upper(upper) {}

    std::vector<Op *> getOperands() const override { return {_target}; }

    auto getLower() { return _lower; }
    auto getUpper() { return _upper; }

    auto setLower(int64_t lower) { _lower = lower; }
    auto setUpper(int64_t upper) { _upper = upper; }
};

enum class CheckResult {
    KEEP_CHECK_1,
    KEEP_CHECK_2,
    KEEP_BOTH,
};

template <typename CheckTy>
static CheckResult pickCheck(CheckOp *check1, CheckOp *check2, dominatorMap &doms) {
    CheckResult res = CheckResult::KEEP_BOTH;

    BasicBlock *bb1 = check1->getBB();
    BasicBlock *bb2 = check2->getBB();

    if (bb1 == bb2) {
        if (check1->getBlockId() < check2->getBlockId()) {
            res = CheckResult::KEEP_CHECK_1;
        } else {
            res = CheckResult::KEEP_CHECK_2;
        }
    } else {
        const auto &domsFor1 = doms[bb1];
        const auto &domsFor2 = doms[bb2];
        if (std::find(domsFor1.begin(), domsFor1.end(), bb2) != domsFor1.end()) {
            res = CheckResult::KEEP_CHECK_2;
        } else if (std::find(domsFor2.begin(), domsFor2.end(), bb1) != domsFor2.end()) {
            res = CheckResult::KEEP_CHECK_1;
        }
    }

    if constexpr (std::is_same_v<CheckTy, BoundCheck>) {
        if (res != CheckResult::KEEP_BOTH) {
            BoundCheck *bCh1 = static_cast<BoundCheck *>(check1);
            BoundCheck *bCh2 = static_cast<BoundCheck *>(check2);

            int64_t lower = std::max(bCh1->getLower(), bCh2->getLower());
            int64_t upper = std::min(bCh1->getUpper(), bCh2->getUpper());

            if (res == CheckResult::KEEP_CHECK_1) {
                bCh1->setLower(lower);
                bCh1->setUpper(upper);
            }

            if (res == CheckResult::KEEP_CHECK_2) {
                bCh2->setLower(lower);
                bCh2->setUpper(upper);
            }
        }
    }

    return res;
}

static bool optimizeChecks(Function *func) {
    auto bbs = func->getBBRPO();

    std::map<Op *, std::vector<CheckOp *>> checksRPO;

    for (auto *bb : bbs) {
        for (auto &op : bb->getOps()) {
            if (op->is<CheckOp>()) {
                CheckOp *check = static_cast<CheckOp *>(op.get());
                for (auto *operand : check->getOperands()) {
                    checksRPO[operand].push_back(check);
                }
            }
        }
    }

    auto dominators = find_dominators(func->getEntry(), func->getBBs());

    for (auto &[op, checks] : checksRPO) {
        if (checks.size() < 2) {
            continue;
        }

        CheckOp *zeroCh = nullptr;
        CheckOp *nullCh = nullptr;
        CheckOp *boundCh = nullptr;

        for (auto *check : checks) {
            if (check->is<ZeroCheck>()) {
                if (!zeroCh) {
                    zeroCh = check;
                } else {
                    auto res = pickCheck<ZeroCheck>(check, zeroCh, dominators);
                    if (res == CheckResult::KEEP_CHECK_1) {
                        zeroCh->getBB()->removeOp(zeroCh);
                        zeroCh = check;
                    } else if (res == CheckResult::KEEP_CHECK_2) {
                        check->getBB()->removeOp(check);
                    }
                }
            } else if (check->is<NullCheck>()) {
                if (!nullCh) {
                    nullCh = check;
                } else {
                    auto res = pickCheck<NullCheck>(check, nullCh, dominators);
                    if (res == CheckResult::KEEP_CHECK_1) {
                        nullCh->getBB()->removeOp(nullCh);
                        nullCh = check;
                    } else if (res == CheckResult::KEEP_CHECK_2) {
                        check->getBB()->removeOp(check);
                    }
                }
            } else if (check->is<BoundCheck>()) {
                if (!boundCh) {
                    boundCh = check;
                } else {
                    auto res = pickCheck<BoundCheck>(check, boundCh, dominators);
                    if (res == CheckResult::KEEP_CHECK_1) {
                        boundCh->getBB()->removeOp(boundCh);
                        boundCh = check;
                    } else if (res == CheckResult::KEEP_CHECK_2) {
                        check->getBB()->removeOp(check);
                    }
                }
            } else {
                throw std::runtime_error("Checks type invalid");
            }
        }
    }
    return true;
}

} // namespace IR

#endif // CHECKS_H
