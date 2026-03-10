#ifndef LINEAR_SCAN_H
#define LINEAR_SCAN_H

#include <algorithm>
#include <cstdint>
#include <limits>
#include <list>
#include <unordered_map>
#include <vector>

#include "ir.h"

namespace IR {

struct LiveRange {
    int64_t start;
    int64_t end;
};

class LiveInterval {
  public:
    Op *vreg;
    std::vector<LiveRange> ranges;
    int reg;
    int spillSlot;

    LiveInterval(Op *vreg) : vreg(vreg), reg(-1), spillSlot(-1) {}

    int64_t start() const { return ranges.empty() ? 0 : ranges.front().start; }
    int64_t end() const { return ranges.empty() ? 0 : ranges.back().end; }

    bool covers(int64_t pos) const {
        for (const auto &r : ranges)
            if (pos >= r.start && pos <= r.end)
                return true;
        return false;
    }

    int64_t nextIntersection(const LiveInterval &other, int64_t pos) const {
        int64_t best = std::numeric_limits<int64_t>::max();
        auto it1 = ranges.begin();
        auto it2 = other.ranges.begin();
        while (it1 != ranges.end() && it2 != other.ranges.end()) {
            if (it1->end < it2->start) {
                ++it1;
            } else if (it2->end < it1->start) {
                ++it2;
            } else {
                int64_t overlapStart = std::max(it1->start, it2->start);
                int64_t overlapEnd = std::min(it1->end, it2->end);
                if (overlapEnd >= pos) {
                    int64_t candidate = std::max(overlapStart, pos);
                    if (candidate < best)
                        best = candidate;
                }
                if (it1->end < it2->end)
                    ++it1;
                else
                    ++it2;
            }
        }
        return best;
    }

    int64_t nextUse(int64_t pos) const {
        for (const auto &r : ranges) {
            if (r.end >= pos)
                return std::max(r.start, pos);
        }
        return std::numeric_limits<int64_t>::max();
    }
};

class LinearScan {
    std::list<LiveInterval> pool;
    std::list<LiveInterval *> unhandled;
    std::list<LiveInterval *> active;
    std::list<LiveInterval *> inactive;
    std::list<LiveInterval *> handled;
    int numRegs;
    std::vector<int64_t> freeUntilPos;
    std::vector<int64_t> nextUsePos;

    LiveInterval *split(LiveInterval *iv, int64_t pos) {
        auto newIv = pool.emplace(pool.end(), iv->vreg);
        newIv->spillSlot = iv->spillSlot;
        newIv->reg = -1;

        std::vector<LiveRange> oldRanges = std::move(iv->ranges);
        iv->ranges.clear();
        for (auto &r : oldRanges) {
            if (r.end < pos) {
                iv->ranges.push_back(r);
            } else if (r.start < pos) {
                iv->ranges.push_back({r.start, pos - 1});
                newIv->ranges.push_back({pos, r.end});
            } else {
                newIv->ranges.push_back(r);
            }
        }
        return &*newIv;
    }

    bool tryAllocateFreeReg(LiveInterval *current, int64_t pos) {
        for (int r = 0; r < numRegs; ++r)
            freeUntilPos[r] = std::numeric_limits<int64_t>::max();

        for (auto *iv : active) {
            freeUntilPos[iv->reg] = 0;
        }
        for (auto *iv : inactive) {
            int64_t next = iv->nextIntersection(*current, pos);
            if (next < freeUntilPos[iv->reg])
                freeUntilPos[iv->reg] = next;
        }

        int bestReg = -1;
        int64_t bestFree = -1;
        for (int r = 0; r < numRegs; ++r) {
            if (freeUntilPos[r] > bestFree) {
                bestFree = freeUntilPos[r];
                bestReg = r;
            }
        }

        if (bestFree == 0)
            return false;

        if (current->end() < bestFree) {
            current->reg = bestReg;
            return true;
        } else {
            current->reg = bestReg;
            LiveInterval *newIv = split(current, bestFree);
            auto it = unhandled.begin();
            while (it != unhandled.end() && (*it)->start() < newIv->start())
                ++it;
            unhandled.insert(it, newIv);
            return true;
        }
    }

    void allocateBlockedReg(LiveInterval *current, int64_t pos) {
        for (int r = 0; r < numRegs; ++r)
            nextUsePos[r] = std::numeric_limits<int64_t>::max();

        for (auto *iv : active) {
            int64_t use = iv->nextUse(pos);
            if (use < nextUsePos[iv->reg])
                nextUsePos[iv->reg] = use;
        }
        for (auto *iv : inactive) {
            int64_t use = iv->nextUse(pos);
            if (use < nextUsePos[iv->reg])
                nextUsePos[iv->reg] = use;
        }

        int bestReg = -1;
        int64_t bestNext = -1;
        for (int r = 0; r < numRegs; ++r) {
            if (nextUsePos[r] > bestNext) {
                bestNext = nextUsePos[r];
                bestReg = r;
            }
        }

        if (current->start() > bestNext) {
            current->spillSlot = 0;
            LiveInterval *newIv = split(current, current->nextUse(pos));
            unhandled.push_front(newIv);
        } else {
            current->reg = bestReg;
            for (auto *iv : active) {
                if (iv->reg == bestReg) {
                    LiveInterval *newIv = split(iv, pos);
                    auto it = unhandled.begin();
                    while (it != unhandled.end() && (*it)->start() < newIv->start())
                        ++it;
                    unhandled.insert(it, newIv);
                    break;
                }
            }
            for (auto *iv : inactive) {
                if (iv->reg == bestReg) {
                    int64_t nextStart = iv->nextUse(pos);
                    if (nextStart != std::numeric_limits<int64_t>::max()) {
                        LiveInterval *newIv = split(iv, nextStart);
                        auto it = unhandled.begin();
                        while (it != unhandled.end() && (*it)->start() < newIv->start())
                            ++it;
                        unhandled.insert(it, newIv);
                    }
                    break;
                }
            }
        }
    }

  public:
    LinearScan(int physRegs) : numRegs(physRegs) {
        freeUntilPos.resize(numRegs);
        nextUsePos.resize(numRegs);
    }

    void
    allocate(const std::unordered_map<Op *, std::vector<std::pair<int64_t, int64_t>>> &liveRanges) {
        for (const auto &entry : liveRanges) {
            Op *vreg = entry.first;
            LiveInterval iv(vreg);
            for (const auto &r : entry.second) {
                iv.ranges.push_back({r.first, r.second});
            }
            pool.push_back(std::move(iv));
        }

        for (auto &iv : pool) {
            unhandled.push_back(&iv);
        }
        unhandled.sort(
            [](const LiveInterval *a, const LiveInterval *b) { return a->start() < b->start(); });

        while (!unhandled.empty()) {
            LiveInterval *current = unhandled.front();
            unhandled.pop_front();
            int64_t pos = current->start();

            for (auto it = active.begin(); it != active.end();) {
                LiveInterval *iv = *it;
                if (iv->end() < pos) {
                    handled.push_back(iv);
                    it = active.erase(it);
                } else if (!iv->covers(pos)) {
                    inactive.push_back(iv);
                    it = active.erase(it);
                } else {
                    ++it;
                }
            }

            for (auto it = inactive.begin(); it != inactive.end();) {
                LiveInterval *iv = *it;
                if (iv->end() < pos) {
                    handled.push_back(iv);
                    it = inactive.erase(it);
                } else if (iv->covers(pos)) {
                    active.push_back(iv);
                    it = inactive.erase(it);
                } else {
                    ++it;
                }
            }

            bool success = tryAllocateFreeReg(current, pos);
            if (!success) {
                allocateBlockedReg(current, pos);
            }

            if (current->reg != -1) {
                active.push_back(current);
            }
        }
    }

    const auto &getIntervals() { return pool; }

    void printResult(std::ostream &os) const {
        for (const auto &iv : pool) {
            os << "Op $" << iv.vreg->getBB()->getName() << iv.vreg->getBlockId() << " (global "
               << iv.vreg->getGlobalId() << "): ";
            if (iv.reg != -1)
                os << "reg R" << iv.reg;
            else
                os << "spill slot " << iv.spillSlot;
            os << " ranges: ";
            for (const auto &r : iv.ranges) {
                os << "[" << r.start << "," << r.end << "] ";
            }
            os << "\n";
        }
    }
};

} // namespace IR

#endif // LINEAR_SCAN_H
