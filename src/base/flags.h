#pragma once

#include <bitset>

template<class T, size_t Size = static_cast<size_t>(T::COUNT), class = void>
class flags;

template<class T, size_t Size>
class flags<T, Size, std::enable_if_t<std::is_enum_v<T>, void>> {
 public:
  using type = T;

  flags() : set_() {}
  flags(std::initializer_list<T> values) : set_() {
    for (T val : values) {
      set_[static_cast<size_t>(val)] = true;
    }
  }

  inline bool operator [](T val) const noexcept { return set_[static_cast<size_t>(val)]; }
  inline bool& operator [](T val) noexcept { return set_[static_cast<size_t>(val)]; }

  inline flags& operator |=(const flags& rhs) noexcept {
    set_ |= rhs.set_;
    return *this;
  }

  inline flags& operator &=(const flags& rhs) noexcept {
    set_ &= rhs.set_;
    return *this;
  }

  inline bool operator ==(const flags& rhs) const noexcept {
    return set_ == rhs.set_;
  }

  inline bool operator !=(const flags& rhs) const noexcept {
    return !(*this == rhs);
  }

  void add(T val) noexcept { set_[static_cast<size_t>(val)] = true; }
  void remove(T val) noexcept { set_[static_cast<size_t>(val)] = false; }
  void set(T val, bool bit) noexcept { set_.set(static_cast<size_t>(val), bit); }

 private:
  std::bitset<Size> set_;
};

template<class T, size_t Size>
inline flags<T, Size> operator|(const flags<T, Size>& x, const flags<T, Size>&y) noexcept {
  flags<T, Size> r = x;
  r |= y;
  return r;
}

template<class T, size_t Size>
inline flags<T, Size> operator&(const flags<T, Size>& x, const flags<T, Size>&y) noexcept {
  flags<T, Size> r = x;
  r &= y;
  return r;
}