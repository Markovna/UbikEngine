#pragma once

#include <vector>
#include <assert.h>

template<class T, size_t PAGE_SIZE = 1u << 15u>
class sparse_set {
    static_assert(std::is_unsigned<T>::value, "The managed type must be an unsigned integral");
    static_assert(PAGE_SIZE && (PAGE_SIZE & (PAGE_SIZE - 1)) == 0, "PAGE_SIZE must be a power of two");

private:
    constexpr static const T invalid_idx = std::numeric_limits<T>::max();

    using page_t = std::unique_ptr<T[]>;

    size_t get_page(T value) const { return value / PAGE_SIZE; }
    size_t get_offset(T value) const { return value & (PAGE_SIZE - 1); }

    page_t& assure(size_t page) {
        if (page >= sparse_.size()) {
            sparse_.resize(page + 1);
        }

        if (!sparse_[page]) {
            sparse_[page].reset(new T[PAGE_SIZE]);
            for (size_t i = 0; i < PAGE_SIZE; i++) {
                sparse_[page][i] = invalid_idx;
            }
        }

        return sparse_[page];
    }

public:
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using reverse_iterator = typename std::vector<T>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

public:
    virtual ~sparse_set() = default;

    [[nodiscard]] bool contains(T value) const {
        size_t page = get_page(value);
        return page < sparse_.size() && sparse_[page] && sparse_[page][get_offset(value)] != invalid_idx;
    }

    [[nodiscard]] size_t index(T value) const {
        assert(contains(value));
        return sparse_[get_page(value)][get_offset(value)];
    }

    [[nodiscard]] size_t size() const { return dense_.size(); }
    [[nodiscard]] bool empty() const { return dense_.empty(); }

    iterator begin() { return dense_.begin(); }
    iterator end() { return dense_.end(); }

    const_iterator begin() const { return dense_.begin(); }
    const_iterator end() const { return dense_.end(); }

    reverse_iterator rbegin() { return dense_.rbegin(); }
    reverse_iterator rend() { return dense_.rend(); }

    const_reverse_iterator rbegin() const { return dense_.rbegin(); }
    const_reverse_iterator rend() const { return dense_.rend(); }

    void insert(T value) {
        assert(!contains(value));
        assure(get_page(value))[get_offset(value)] = static_cast<T>(dense_.size());
        dense_.push_back(value);
    }

    void erase(T value) {
        assert(contains(value));
        T& index = sparse_[get_page(value)][get_offset(value)];
        T back = dense_.back();

        dense_[index] = back;
        sparse_[get_page(back)][get_offset(back)] = index;
        index = invalid_idx;
        dense_.pop_back();
    }

private:
    std::vector<page_t> sparse_;
    std::vector<T> dense_;
};

