#include <algorithm>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "loop.h"
#include "operations.h"

namespace IR {

class LiveIntervalAnalyzer {
  public:
    using Interval = std::pair<int64_t, int64_t>;
    using LiveInsMap = std::unordered_map<Op *, std::vector<Interval>>;
    LiveInsMap intervals;

    void analyze(Function *func, BasicBlock *entry) {
        func->assignGlobalIds(entry);

        std::vector<BasicBlock *> order;
        auto savePO = [&order](BasicBlock *bb) { order.push_back(bb); };
        postorder(entry, savePO);

        auto loopMap = IR::FindAllLoops(*func, entry);

        for (auto it = order.begin(); it != order.end(); ++it) {
            BasicBlock *b = *it;

            std::set<Op *> live;
            auto succs = b->getSuccessors();
            for (auto *succ : {succs.first, succs.second}) {
                if (succ) {
                    live.insert(succ->getLiveIn().begin(), succ->getLiveIn().end());
                }
            }
            for (auto *succ : {succs.first, succs.second}) {
                if (!succ)
                    continue;
                for (auto &opPtr : succ->getOps()) {
                    Op *op = opPtr.get();
                    if (op->is<PhiNode>()) {
                        PhiNode *phi = static_cast<PhiNode *>(op);
                        Op *input = phi->getInputForPredecessor(b);
                        if (input) {
                            live.insert(input);
                        }
                    }
                }
            }

            int64_t b_from = b->blockStart();
            int64_t b_to = b->blockEnd();
            if (b_from == -1)
                continue;

            for (Op *opd : live) {
                intervals[opd].emplace_back(b_from, b_to);
            }

            auto &ops = b->getOps();
            for (auto rit = ops.rbegin(); rit != ops.rend(); ++rit) {
                Op *op = rit->get();
                int64_t opId = op->getGlobalId();

                auto &vec = intervals[op];
                bool found = false;
                for (auto &range : vec) {
                    if (range.first == b_from) {
                        range.first = opId;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    vec.emplace_back(opId, opId);
                }
                live.erase(op);

                // if (!op->is<PhiNode>()) {
                for (Op *input : op->getOperands()) {
                    if (!input)
                        continue;
                    if (live.find(input) == live.end()) {
                        intervals[input].emplace_back(b_from, opId);
                        live.insert(input);
                    }
                }
                // }
            }

            if (loopMap.count(b)) {
                const auto &loop = loopMap[b];
                std::vector<BasicBlock *> loopBlocks = {b};
                loopBlocks.insert(loopBlocks.end(), loop.innerBBs.begin(), loop.innerBBs.end());

                int64_t maxEnd = b_to;
                for (auto *lb : loopBlocks) {
                    int64_t end = lb->blockEnd();
                    if (end > maxEnd)
                        maxEnd = end;
                }

                for (Op *opd : live) {
                    intervals[opd].emplace_back(b_from, maxEnd);
                }
            }

            b->getLiveIn() = live;
        }
    }

    void print(std::ostream &os) const {
        for (auto &[op, vec] : intervals) {
            os << "Op $" << op->getBB()->getName() << "." << op->getBlockId() << " (global "
               << op->getGlobalId() << ", type " << op->getType() << "): ";
            for (auto &[start, end] : vec) {
                os << "[" << start << ", " << end << "] ";
            }
            os << "\n";
        }
    }
};

} // namespace IR
