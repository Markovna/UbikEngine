#pragma once

#include <string>

template <typename It>
class iterator_range {
public:
    iterator_range(It first, It last)
        : first_(first), last_(last) {}

  It begin() const { return first_; }
  It end() const { return last_; }

  [[nodiscard]] size_t size() const { return last_ - first_; }

private:
  It first_, last_;
};
