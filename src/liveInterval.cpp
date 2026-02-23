#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "loop.h"
#include "operations.h"

namespace IR {

class LiveIntervalAnalyzer {
public:
    using Interval = std::pair<int64_t, int64_t>; // [start, end]
    std::unordered_map<Op*, std::vector<Interval>> intervals;

    void analyze(Function* func, BasicBlock* entry) {
        // Assign global IDs in reverse postorder
        func->assignGlobalIds(entry);

        // Compute reverse postorder again for processing (could reuse but we need order vector)
        std::vector<BasicBlock*> order;
        std::unordered_set<BasicBlock*> visited;
        std::function<void(BasicBlock*)> dfs = [&](BasicBlock* bb) {
            visited.insert(bb);
            auto succs = bb->getSuccessors();
            if (succs.first && !visited.count(succs.first)) dfs(succs.first);
            if (succs.second && !visited.count(succs.second)) dfs(succs.second);
            order.push_back(bb);
        };
        dfs(entry);
        std::reverse(order.begin(), order.end()); // order = reverse postorder (entry → exit)

        // Get loop information using existing detection
        auto loopMap = IR::FindAllLoops(*func, entry);

        // Helper to get first/last global ID of a block
        auto blockStart = [](BasicBlock* bb) -> int64_t {
            if (bb->getOps().empty()) return -1;
            return bb->getOps().front()->getGlobalId();
        };
        auto blockEnd = [](BasicBlock* bb) -> int64_t {
            if (bb->getOps().empty()) return -1;
            return bb->getOps().back()->getGlobalId();
        };

        // Process blocks in reverse order (exit → entry)
        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            BasicBlock* b = *it;

            // live = union of successors' liveIn
            std::set<Op*> live;
            auto succs = b->getSuccessors();
            for (auto* succ : {succs.first, succs.second}) {
                if (succ) live.insert(succ->liveIn.begin(), succ->liveIn.end());
            }

            // Add phi inputs from successors
            for (auto* succ : {succs.first, succs.second}) {
                if (!succ) continue;
                for (auto& opPtr : succ->getOps()) {
                    Op* op = opPtr.get();
                    if (op->is<PhiNode>()) {
                        PhiNode* phi = static_cast<PhiNode*>(op);
                        Op* input = phi->getInputForPredecessor(b);
                        if (input) live.insert(input);
                    }
                }
            }

            int64_t b_from = blockStart(b);
            int64_t b_to   = blockEnd(b);
            if (b_from == -1) continue; // empty block (should not happen)

            // Extend intervals for currently live variables across whole block
            for (Op* opd : live) {
                intervals[opd].emplace_back(b_from, b_to);
            }

            // Process operations in reverse order (within block)
            auto& ops = b->getOps();
            for (auto rit = ops.rbegin(); rit != ops.rend(); ++rit) {
                Op* op = rit->get();
                int64_t opId = op->getGlobalId();

                // Output operand (the op itself)
                intervals[op].emplace_back(opId, opId);
                live.erase(op);

                // Input operands
                for (Op* input : op->getOperands()) {
                    if (!input) continue;
                    intervals[input].emplace_back(b_from, opId);
                    live.insert(input);
                }
            }

            // Remove phi outputs of this block
            for (auto& opPtr : ops) {
                Op* op = opPtr.get();
                if (op->isPhi()) {
                    live.erase(op);
                }
            }

            // If this block is a loop header, extend live variables across the entire loop
            if (loopMap.count(b)) {
                const auto& loop = loopMap[b];
                // Gather all blocks in the loop (header + innerBBs)
                std::vector<BasicBlock*> loopBlocks = {b};
                loopBlocks.insert(loopBlocks.end(), loop.innerBBs.begin(), loop.innerBBs.end());

                // Find the block with the maximum global end ID
                BasicBlock* loopEndBlock = b;
                int64_t maxEnd = b_to;
                for (auto* lb : loopBlocks) {
                    int64_t end = blockEnd(lb);
                    if (end > maxEnd) {
                        maxEnd = end;
                        loopEndBlock = lb;
                    }
                }

                // Extend live variables to the last instruction of the loop
                for (Op* opd : live) {
                    intervals[opd].emplace_back(b_from, maxEnd);
                }
            }

            // Save liveIn for this block
            b->getLiveIn() = live;
        }
    }

    void print(std::ostream& os) const {
        for (auto& [op, vec] : intervals) {
            os << "Op $" << op->getLocalId() << " (global " << op->getGlobalId()
               << ", type " << op->getType() << "): ";
            for (auto& [start, end] : vec) {
                os << "[" << start << ", " << end << "] ";
            }
            os << "\n";
        }
    }
};

} // namespace IR
