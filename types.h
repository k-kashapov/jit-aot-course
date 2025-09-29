#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace IR {

struct Type {
    enum class EType {
        None,
        SI64,
        SI32,
        SI16,
        SI8,
        UI64,
        UI32,
        UI16,
        UI8,
        F64,
        F32,
        F16,
    };

    EType _ety = EType::None;

    int64_t getWidth() {
        switch (_ety) {
        case EType::None:
            return 0;
        case EType::SI64:
            return 64;
        case EType::SI32:
            return 32;
        case EType::SI16:
            return 16;
        case EType::SI8:
            return 8;
        case EType::UI64:
            return 64;
        case EType::UI32:
            return 32;
        case EType::UI16:
            return 16;
        case EType::UI8:
            return 8;
        case EType::F64:
            return 64;
        case EType::F32:
            return 32;
        case EType::F16:
            return 16;
        default:
            throw std::runtime_error("unexpected type: " + std::to_string(static_cast<int>(_ety)));
        }
    }

    bool isUnsigned() {
        switch (_ety) {
        case EType::None:
            return false;
        case EType::SI64:
            return false;
        case EType::SI32:
            return false;
        case EType::SI16:
            return false;
        case EType::SI8:
            return false;
        case EType::UI64:
            return true;
        case EType::UI32:
            return true;
        case EType::UI16:
            return true;
        case EType::UI8:
            return true;
        case EType::F64:
            return false;
        case EType::F32:
            return false;
        case EType::F16:
            return false;
        default:
            throw std::runtime_error("unexpected type: " + std::to_string(static_cast<int>(_ety)));
        }
    }

    bool operator==(const Type &other) const { return _ety == other._ety; }

    friend std::ostream &operator<<(std::ostream &os, const Type &ty) {
        switch (ty._ety) {
        case EType::None:
            return os << "";
        case EType::SI64:
            return os << "SI64";
        case EType::SI32:
            return os << "SI32";
        case EType::SI16:
            return os << "SI16";
        case EType::SI8:
            return os << "SI8";
        case EType::UI64:
            return os << "UI64";
        case EType::UI32:
            return os << "UI32";
        case EType::UI16:
            return os << "UI16";
        case EType::UI8:
            return os << "UI8";
        case EType::F64:
            return os << "F64";
        case EType::F32:
            return os << "F32";
        case EType::F16:
            return os << "F16";
        default:
            throw std::runtime_error("unexpected type: " +
                                     std::to_string(static_cast<int>(ty._ety)));
        }
    }

    Type(EType ety) : _ety(ety) {}
};

} // namespace IR

#endif // TYPES_H
