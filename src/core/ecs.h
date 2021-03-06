#pragma once

#include "base/sparse_set.h"
#include "base/iterator_range.h"
#include "core/meta/type.h"

#include <unordered_map>
#include <cassert>

namespace ecs {

using entity = uint32_t;

namespace details {

struct entity_traits {
    static constexpr uint8_t gen_offset = 20u;
    static constexpr entity index_mask       = 0x000FFFFF;
    static constexpr entity generation_mask  = 0xFFFu << gen_offset;

    static constexpr entity get_index(entity entity) { return entity & index_mask; }
    static constexpr entity get_generation(entity entity) { return (entity & generation_mask) >> gen_offset; }

    static constexpr void set_index(entity& entity, ::ecs::entity idx) { entity = (entity & generation_mask) | idx; }
    static constexpr void set_generation(entity& entity, ::ecs::entity gen) { entity = (entity & index_mask) | (gen << gen_offset); }
};

using component_pool_base = sparse_set<entity>;

template<class Component>
class component_pool : public component_pool_base {
private:
    using iterator       = typename std::vector<Component>::iterator;
    using const_iterator = typename std::vector<Component>::const_iterator;

public:
    using base_type = sparse_set<entity>;

public:
    auto begin() { return components_.begin(); }
    auto end() { return components_.end(); }

    auto begin() const { return components_.begin(); }
    auto end() const { return components_.end(); }

    auto rbegin() { return components_.rbegin(); }
    auto rend() { return components_.rend(); }

    auto rbegin() const { return components_.rbegin(); }
    auto rend() const { return components_.rend(); }

    size_t size() const { return components_.size(); }

    Component& push(entity entity, const Component& component) {
      assert(!base_type::contains(entity));
      components_.push_back(component);
      base_type::insert(entity);
      return components_.back();
    }

    Component& push(entity entity, Component&& component) {
      assert(!base_type::contains(entity));
      components_.push_back(std::move(component));
      base_type::insert(entity);
      return components_.back();
    }

    template<class ...Args>
    Component& emplace(entity entity, Args&&... args) {
      assert(!base_type::contains(entity));
      Component& comp = components_.emplace_back(std::forward<Args>(args)...);
      base_type::insert(entity);
      return comp;
    }

    void erase(entity entity) {
      assert(base_type::contains(entity));
      auto other = std::move(components_.back());
      components_[base_type::index(entity)] = std::move(other);
      components_.pop_back();
      base_type::erase(entity);
    }

    void clear() {
      components_.clear();
      base_type::clear();
    }

    [[nodiscard]] Component& get(entity entity) { return components_[base_type::index(entity)]; }
    [[nodiscard]] const Component& get(entity entity) const { return components_[base_type::index(entity)]; }

    [[nodiscard]] Component* try_get(entity entity) { return base_type::contains(entity) ? &get(entity) : nullptr; }
    [[nodiscard]] const Component* try_get(entity entity) const  { return base_type::contains(entity) ? &get(entity) : nullptr; }

private:
    std::vector<Component> components_;
};

}

static constexpr entity invalid = details::entity_traits::index_mask | details::entity_traits::generation_mask;

template<class ...>
class component_view;

template<class Component>
class component_view<Component> {
private:
    using pool_t = typename std::conditional<std::is_const_v<Component>, const details::component_pool<std::remove_cv_t<Component>>, details::component_pool<std::remove_cv_t<Component>>>::type;
    using pool_base_t = typename pool_t::base_type;
public:
    explicit component_view(pool_t& pool) : pool_(&pool) {}

    auto begin() const { return pool_->pool_base_t::begin(); }
    auto end() const { return pool_->pool_base_t::end(); }

    [[nodiscard]] size_t size() const { return pool_->size(); }

    [[nodiscard]] Component &get(entity entity) { return pool_->get(entity); }
    [[nodiscard]] const Component &get(entity entity) const { return pool_->get(entity); }

    [[nodiscard]] Component* try_get(entity entity) { return pool_->contains(entity) ? &get(entity) : nullptr; }
    [[nodiscard]] const Component* try_get(entity entity) const { return pool_->contains(entity) ? &get(entity) : nullptr; }

    [[nodiscard]] bool contains(entity entity) const { return pool_->contains(entity); }

private:
    pool_t* pool_;
};

template<class ...Components>
class component_view {
private:
  static_assert(sizeof...(Components) > 1, "Invalid components");

  template<class Comp>
  using pool_t = typename std::conditional<std::is_const_v<Comp>, const details::component_pool<std::remove_cv_t<Comp>>, details::component_pool<std::remove_cv_t<Comp>>>::type;
  using pool_base_t = typename details::component_pool_base ;

//    template<class Comp>
//    using pool_t = details::component_pool<std::remove_cv_t<Comp>>;
//    using entities_t = sparse_set<entity>;
  using unchecked_array = std::array<const pool_base_t*, sizeof...(Components) - 1>;

private:
    template<class It>
    struct view_iterator {
    public:
        using difference_type = typename std::iterator_traits<It>::difference_type;
        using value_type = typename std::iterator_traits<It>::value_type;
        using pointer = typename std::iterator_traits<It>::pointer;
        using reference = typename std::iterator_traits<It>::reference;
        using iterator_category = std::bidirectional_iterator_tag;

    private:
        [[nodiscard]] bool check() const {
            return std::all_of(unchecked_.begin(), unchecked_.end(), [entity = *curr_](const pool_base_t* pool) { return pool->contains(entity); });
        }

    public:
        view_iterator(It first, It last, It curr, unchecked_array unchecked)
            : first_(first), last_(last), curr_(curr), unchecked_(unchecked) {
            if (curr_ != last_ && !check()) ++(*this);
        }

        view_iterator& operator++() {
            while (++curr_ != last_ && !check());
            return *this;
        };

        view_iterator& operator--() {
            while (--curr_ != first_ && !check());
            return *this;
        };

        view_iterator operator++(int) {
            view_iterator orig = *this;
            return ++(*this), orig;
        };

        view_iterator operator--(int) {
            view_iterator orig = *this;
            return --(*this), orig;
        };

        [[nodiscard]] bool operator==(const view_iterator& other) const {
            return other.curr_ == curr_;
        }

        [[nodiscard]] bool operator!=(const view_iterator& other) const {
            return !(*this == other);
        }

        [[nodiscard]] pointer operator->() const {
            return curr_.operator->();
        }

        [[nodiscard]] reference operator*() const {
            return curr_.operator*();
        }

    private:
        It first_, last_, curr_;
        unchecked_array unchecked_;
    };

    static const pool_base_t* shortest(const pool_t<Components>&... pools) {
        return std::min({static_cast<const pool_base_t*>(&pools)...}, [](const pool_base_t* lhs, const pool_base_t* rhs) {
            return lhs->size() < rhs->size();
        });
    }

    static unchecked_array except(const pool_base_t* exception, const pool_t<Components>*... pools) {
        unchecked_array arr;
        size_t idx = 0;
        ((static_cast<const pool_base_t*>(pools) != exception ? arr[idx++] = pools : nullptr), ...);
        return arr;
    }

    unchecked_array unchecked() const { return except(entities_, std::get<pool_t<Components>*>(pools_)...); }

public:
    using iterator = view_iterator<typename pool_base_t::const_iterator>;

public:
    explicit component_view(pool_t<Components>&... pools)
        : pools_{&pools...}, entities_{shortest(pools...)} {}

    [[nodiscard]] bool contains(entity entity) const {
        return (std::get<pool_t<Components>*>(pools_)->contains(entity) && ...);
    }

    iterator begin() const {
        return iterator { entities_->begin(), entities_->end(), entities_->begin(), unchecked() };
    }

    iterator end() const {
        return iterator { entities_->begin(), entities_->end(), entities_->end(), unchecked() };
    }

    std::tuple<Components&...> get(entity entity) const {
      return get<Components...>(entity);
    }

  template<class Component>
  const Component& get(entity entity) const {
    return std::get<pool_t<Component>*>(pools_)->get(entity);
  }

  template<class Component>
  Component& get(entity entity) {
    return std::get<pool_t<Component>*>(pools_)->get(entity);
  }

    template<class Component1, class Component2, class ...Args>
    std::tuple<Component1&, Component2&, Args&...> get(entity entity) const {
      return std::tuple<Component1&, Component2&, Args&...>{get<Component1>(entity), get<Component2>(entity), get<Args>(entity)...};
    }

private:
    const std::tuple<pool_t<Components>*...> pools_;
    const sparse_set<entity>* entities_;
};

using pool_base_t = sparse_set<entity>;

template<class Component>
using pool_t = details::component_pool<std::remove_cv_t<Component>>;
using pool_ptr = std::unique_ptr<sparse_set<entity>>;
using entity_traits = details::entity_traits;
using component_id_t = meta::typeid_t;

struct pool_info {
  using remove_ptr_t = void (*)(sparse_set<entity>*, ecs::entity);
  using get_ptr_t = void* (*)(sparse_set<entity>*, ecs::entity);

  pool_ptr ptr;
  component_id_t id;
  remove_ptr_t remove_ptr;
  get_ptr_t get_ptr;
};

class component_ids {
 private:
  using pool_iterator = std::vector<pool_info>::const_iterator;

 public:
  class iterator {
   public:
    using value_type = std::pair<component_id_t, void*>;

   public:
    iterator(entity entity, pool_iterator curr, pool_iterator last);

    iterator& operator++();
    iterator operator++(int);

    [[nodiscard]] bool operator==(const iterator& other) const;
    [[nodiscard]] bool operator!=(const iterator& other) const;

    [[nodiscard]] value_type operator*() const;

   private:
    [[nodiscard]] bool check() const;

   private:
    pool_iterator curr_;
    pool_iterator last_;
    entity entity_;
  };

 public:
  component_ids(entity entity, pool_iterator curr, pool_iterator last)
    : entity_(entity), curr_(curr), last_(last) {}

  [[nodiscard]] iterator begin() const { return { entity_, curr_, last_ }; }
  [[nodiscard]] iterator end() const { return { entity_, last_, last_ }; }

 private:
  entity entity_;
  pool_iterator curr_;
  pool_iterator last_;
};

template<class Component>
static void remove_pool_impl(sparse_set<entity>* ptr, ecs::entity entity) {
  static_cast<pool_t<Component>*>(ptr)->erase(entity);
}

template<class Component>
static void* get_component_impl(sparse_set<entity>* ptr, ecs::entity entity) {
  return static_cast<pool_t<Component>*>(ptr)->try_get(entity);
}

struct registry {
private:
  static constexpr size_t invalid_idx = details::entity_traits::index_mask;

  template<class Component>
  [[nodiscard]] uint32_t component_index() const {
    auto it = component_id_index_.find(meta::get_typeid<Component>());
    return it != component_id_index_.end() ? it->second : std::numeric_limits<uint32_t>::max();
  }

  [[nodiscard]] uint32_t component_index(component_id_t id) const {
    auto it = component_id_index_.find(id);
    return it != component_id_index_.end() ? it->second : std::numeric_limits<uint32_t>::max();
  }

  template<class Component>
  pool_t<Component>& assure() {
    using component_t = std::remove_cv_t<Component>;

    component_id_t id = meta::get_typeid<component_t>();

    if (auto it = component_id_index_.find(id); it != component_id_index_.end())
      return *static_cast<pool_t<component_t>*>(pools_[it->second].ptr.get());

    component_id_index_[id] = pools_.size();

    pool_info& p = pools_.emplace_back();
    p.id = id;
    p.ptr = std::make_unique<pool_t<component_t>>();
    p.remove_ptr = &remove_pool_impl<component_t>;
    p.get_ptr = &get_component_impl<component_t>;

    return *static_cast<pool_t<component_t>*>(p.ptr.get());
  }

public:
  entity create() {
    entity entity{};
    if (free_idx_ == invalid_idx) {
      entity_traits::set_index(entity, entities_.size());
      entity_traits::set_generation(entity, {});
      entities_.push_back(entity);
    }
    else {
      ::ecs::entity free = entities_[free_idx_];
      entity_traits::set_index(entity, free_idx_);
      entity_traits::set_generation(entity, entity_traits::get_generation(free));
      entities_[free_idx_] = entity;
      free_idx_ = entity_traits::get_index(free);
    }
    assert(valid(entity));
    return entity;
  }

  void destroy(entity entity) {
    assert(valid(entity));
    remove_all(entity);
    auto index = details::entity_traits::get_index(entity);
    details::entity_traits::set_index(entities_[index], free_idx_);
    details::entity_traits::set_generation(entities_[index], entity_traits::get_generation(entities_[index]) + 1);
    free_idx_ = details::entity_traits::get_index(entity);
  }

  [[nodiscard]] bool valid(entity entity) const {
    auto index = entity_traits::get_index(entity);
    return index < entities_.size() && entities_[index] == entity;
  }

  template<class Component, class ...Args>
  Component& emplace(entity entity, Args &&... args) {
    assert(valid(entity));
    return assure<Component>().emplace(entity, std::forward<Args>(args)...);
  }

  template<class Component>
  void remove(entity entity) {
    assert(has<Component>(entity));
    get_pool<Component>()->erase(entity);
  }

  void remove_all(entity entity) {
    assert(valid(entity));
    for (auto& pool : pools_) {
        if (pool.ptr && pool.ptr->contains(entity)) {
            pool.remove_ptr(pool.ptr.get(), entity);
        }
    }
  }

  template<class Component>
  const Component& get(entity entity) const {
    assert(has<Component>(entity));
    return get_pool<Component>()->get(entity);
  }

  template<class Component>
  Component& get(entity entity) {
    return const_cast<Component&>(static_cast<const registry&>(*this).get<Component>(entity));
  }

  template<class Component>
  const Component* try_get(entity entity) const {
    assert(valid(entity));
    const pool_t<Component>* pool = get_pool<Component>();
    return pool ? pool->try_get(entity) : nullptr;
  }

  template<class Component>
  Component* try_get(entity entity) {
    return const_cast<Component*>(static_cast<const registry&>(*this).try_get<Component>(entity));
  }

  template<class Component>
  [[nodiscard]] bool has(entity entity) const {
    assert(valid(entity));
    const pool_t<Component>* pool = get_pool<Component>();
    return pool && pool->contains(entity);
  }

  template<class Component>
  [[nodiscard]] bool has() const {
    return get_pool<Component>() != nullptr;
  }

  bool has(component_id_t id) const {
    return component_id_index_.count(id);
  }

  template<class ...Component>
  component_view<Component...> view() {
      return component_view<Component...>(assure<Component>()...);
  }

  template<class ...Component>
  component_view<const Component...> view() const {
    return component_view<const Component...>(const_cast<registry*>(this)->assure<Component>()...);
  }

  template<class Component>
  [[nodiscard]] size_t size() const {
    pool_t<Component>* pool = get_pool<Component>();
    return pool ? pool->size() : 0;
  }

  component_ids get_components(entity entity) {
    return { entity, pools_.begin(), pools_.end() };
  }

  const sparse_set<entity>* pool_base(component_id_t id) const {
    uint32_t index = component_index(id);
    return index < pools_.size() ? pools_[index].ptr.get() : nullptr;
  }

  pool_base_t* pool_base(component_id_t id) {
    uint32_t index = component_index(id);
    return index < pools_.size() ? pools_[index].ptr.get() : nullptr;
  }

  template<class Component>
  const pool_t<Component>* get_pool() const {
    uint32_t index = component_index<Component>();
    return index < pools_.size() ?
           static_cast<pool_t<Component>*>(pools_[index].ptr.get()) : nullptr;
  }

  template<class Component>
  pool_t<Component>* get_pool() {
    return const_cast<pool_t<Component>*>(static_cast<const registry&>(*this).get_pool<Component>());
  }

private:
    std::vector<pool_info> pools_{};
    std::vector<entity> entities_{};
    std::unordered_map<component_id_t , uint32_t> component_id_index_{};

    size_t free_idx_{invalid_idx};
};

}