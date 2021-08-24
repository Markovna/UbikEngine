#include "ecs.h"

namespace ecs {

component_ids::iterator &component_ids::iterator::operator++() {
  while (++curr_ != last_ && !check());
  return *this;
}

component_ids::iterator component_ids::iterator::operator++(int) {
  iterator orig = *this;
  return ++(*this), orig;
}

bool component_ids::iterator::operator==(const component_ids::iterator &other) const {
  return other.curr_ == curr_;
}

bool component_ids::iterator::operator!=(const component_ids::iterator &other) const {
  return !(*this == other);
}

bool component_ids::iterator::check() const { return curr_->ptr->contains(entity_); }

component_ids::iterator::value_type component_ids::iterator::operator*() const {
  return std::make_pair(curr_->id, curr_->get_ptr(curr_->ptr.get(), entity_));
}

component_ids::iterator::iterator(entity entity, pool_iterator curr, pool_iterator last)
  : entity_(entity), curr_(curr), last_(last)
{
  if (curr_ != last_ && !check()) ++(*this);
}

}