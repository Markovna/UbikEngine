#include "world.h"

void world::set_parent_impl(entity ent, entity parent, entity next) {
  link_component& comp = component<link_component>(ent);
  if (comp.parent) {
    link_component& parent_comp = component<link_component>(comp.parent);
    parent_comp.children_size--;

    if (parent_comp.child == ent)
      parent_comp.child = comp.next;

    if (parent_comp.child_last == ent)
      parent_comp.child_last = comp.prev;
  }

  if (comp.prev)
    component<link_component>(comp.prev).next = comp.next;

  if (comp.next)
    component<link_component>(comp.next).prev = comp.prev;

  comp.parent = parent;
  comp.next = next;
  comp.prev = entity::invalid();

  if (comp.parent) {
    link_component& parent_comp = component<link_component>(comp.parent);
    parent_comp.children_size++;

    if (!comp.next) {
      if (parent_comp.child_last) {
        link_component& last_child_comp = component<link_component>(parent_comp.child_last);
        last_child_comp.next = ent;
      }

      comp.prev = parent_comp.child_last;
      parent_comp.child_last = ent;
    }

    if (comp.next == parent_comp.child)
      parent_comp.child = ent;
  }

  if (comp.next) {
    link_component& next_comp = component<link_component>(comp.next);

    if (next_comp.prev) {
      link_component &prev_comp = component<link_component>(next_comp.prev);
      prev_comp.next = ent;
      comp.prev = next_comp.prev;
    }

    next_comp.prev = ent;
  }
}