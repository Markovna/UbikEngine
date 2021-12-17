#pragma once

#include <iterator>
#include <functional>

template<class Iterator, class Operator>
class transform_iterator
    : public std::iterator<std::output_iterator_tag, void, void, void, void> {

 public:
  transform_iterator(Iterator it, Operator op)
      : iterator_(it) , operator_(op)
  {}

  using return_type = decltype(std::declval<Operator>()(std::declval<typename std::iterator_traits<Iterator>::reference>()));

  using value_type = return_type;
  using difference_type = value_type;
  using pointer = value_type*;
  using reference = value_type&;
  using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;

  auto operator*() {
    return std::invoke(operator_, *iterator_);
  }

  transform_iterator& operator++() {
    ++iterator_;
    return *this;
  }

  transform_iterator& operator--() {
    --iterator_;
    return *this;
  }

  const Iterator& base() const { return iterator_; }

 private:
  Operator operator_;
  Iterator iterator_;
};

template<typename Functor, typename Iterator>
bool operator==(const transform_iterator<Functor, Iterator>& lhs, const transform_iterator<Functor, Iterator>& rhs) {
  return lhs.base() == rhs.base();
}

template <typename Functor, typename Iterator>
bool operator!=(const transform_iterator<Functor, Iterator>& lhs, const transform_iterator<Functor, Iterator>& rhs) {
  return !(lhs == rhs);
}

template<class Iterator, class Operator>
auto make_transform_iterator(Iterator&& it, Operator&& op) {
  return
      transform_iterator<
          std::remove_const_t<std::remove_reference_t<Iterator>>,
  std::remove_const_t<std::remove_reference_t<Operator>>>
  (std::forward<Iterator>(it), std::forward<Operator>(op));
}