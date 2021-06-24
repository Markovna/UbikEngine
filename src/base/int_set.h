#pragma once

#include <numeric>

template<class T, T SIZE>
class int_set {
private:
    static_assert(std::is_unsigned<T>::value, "The managed type must be an unsigned integral");

    static constexpr T invalid = SIZE;

public:
    int_set() : next_(0) {
        std::iota(std::begin(registry_), std::end(registry_), 1);
    }

    [[nodiscard]] bool has_free() const { return next_ != invalid; }

    T alloc() {
        T& idx = registry_[next_];
        std::swap(idx, next_);
        return idx;
    }

    void free(T idx) {
        registry_[idx] = next_;
        next_ = idx;
    }

private:
    T registry_[SIZE];
    T next_;
};


