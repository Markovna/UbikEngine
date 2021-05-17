#pragma once

#include "base/sparse_set.h"

#include <array>
#include <assert.h>

namespace ecs {

using entity = uint32_t;

namespace details {

struct entity_traits {
    static constexpr uint8_t  gen_offset = 20u;
    static constexpr entity index_mask       = 0x000FFFFF;
    static constexpr entity generation_mask  = 0xFFFu << gen_offset;

    static constexpr entity get_index(entity entity) { return entity & index_mask; }
    static constexpr entity get_generation(entity entity) { return (entity & generation_mask) >> gen_offset; }

    static constexpr void set_index(entity& entity, ::ecs::entity idx) { entity = (entity & generation_mask) | idx; }
    static constexpr void set_generation(entity& entity, ::ecs::entity gen) { entity = (entity & index_mask) | (gen << gen_offset); }
};

template<class Component>
class component_pool : public sparse_set<entity> {
private:
    using iterator       = typename std::vector<Component>::iterator;
    using const_iterator = typename std::vector<Component>::const_iterator;

public:
    using entities = sparse_set<entity>;

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
        assert(!entities::contains(entity));
        components_.push_back(component);
        entities::insert(entity);
        return components_.back();
    }

    Component& push(entity entity, Component&& component) {
        assert(!entities::contains(entity));
        components_.push_back(std::move(component));
        entities::insert(entity);
        return components_.back();
    }

    template<class ...Args>
    Component& emplace(entity entity, Args&&... args) {
        assert(!entities::contains(entity));
        Component& comp = components_.emplace_back(std::forward<Args>(args)...);
        entities::insert(entity);
        return comp;
    }

    void erase(entity entity) {
        assert(entities::contains(entity));
        auto other = std::move(components_.back());
        components_[entities::index(entity)] = std::move(other);
        components_.pop_back();
        entities::erase(entity);
    }

    [[nodiscard]] Component& get(entity entity) { return components_[entities::index(entity)]; }
    [[nodiscard]] const Component& get(entity entity) const { return components_[entities::index(entity)]; }

    [[nodiscard]] Component* try_get(entity entity) { return entities::contains(entity) ? &get(entity) : nullptr; }
    [[nodiscard]] const Component* try_get(entity entity) const  { return entities::contains(entity) ? &get(entity) : nullptr; }

private:
    std::vector<Component> components_;
};

uint32_t next_index();

}

static constexpr entity invalid = details::entity_traits::index_mask | details::entity_traits::generation_mask;

template<class Component>
struct component_info {
public:
    static uint32_t index() {
        static uint32_t index = details::next_index();
        return index;
    }
};

template<class ...>
class component_view;

template<class Component>
class component_view<Component> {
private:
    using pool_t = details::component_pool<Component>;
    using entities_t = sparse_set<entity>;
public:
    explicit component_view(pool_t& pool) : pool_(&pool) {}

    auto begin() const { return pool_->entities_t::begin(); }
    auto end() const { return pool_->entities_t::end(); }

    size_t size() const { return pool_->size(); }

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
    using pool_t = details::component_pool<Comp>;
    using entities_t = sparse_set<entity>;
    using unchecked_array = std::array<const entities_t*, sizeof...(Components) - 1>;

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
            return std::all_of(unchecked_.begin(), unchecked_.end(), [entity = *curr_](const entities_t* pool) { return pool->contains(entity); });
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

    static const entities_t* shortest(const pool_t<Components>&... pools) {
        return std::min({static_cast<const entities_t*>(&pools)...}, [](const entities_t* lhs, const entities_t* rhs) {
            return lhs->size() < rhs->size();
        });
    }

    static unchecked_array except(const entities_t* exception, const pool_t<Components>*... pools) {
        unchecked_array arr;
        size_t idx = 0;
        ((static_cast<const entities_t*>(pools) != exception ? arr[idx++] = pools : nullptr), ...);
        return arr;
    }

    unchecked_array unchecked() const { return except(entities_, std::get<pool_t<Components>*>(pools_)...); }

public:
    using iterator = view_iterator<typename entities_t::const_iterator>;

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
    Component& get(entity entity) const {
      return std::get<pool_t<Component>*>(pools_)->get(entity);
    }

    template<class T1, class T2, class ...Comp>
    std::tuple<T1&, T2&, Comp&...> get(entity entity) const {
      return std::tuple<T1&, T2&, Comp&...>{get<T1>(entity), get<T2>(entity), get<Comp>(entity)...};
    }

//    template<class ...Comp>
//    auto get(entity entity) const {
//        assert(contains(entity));
//
//        if constexpr (sizeof...(Comp) == 0) {
//            return get<Components...>(entity);
//        }
//        else if constexpr (sizeof...(Comp) == 1) {
//            return (std::get<pool_t<Comp> *>(pools_)->get(entity), ...);
//        }
//        else {
//            return std::tuple<Comp&...>{get<Comp>(entity)...};
//        }
//    }

private:
    const std::tuple<pool_t<Components>*...> pools_;
    const sparse_set<entity>* entities_;
};

struct registry {
private:
    static constexpr size_t invalid_idx = details::entity_traits::get_index(invalid);

    template<class Component>
    using pool_t = details::component_pool<Component>;
    using pool_ptr = std::unique_ptr<sparse_set<entity>>;
    using entity_traits = details::entity_traits;

    struct pool_info {
        using remove_ptr_t = void (*)(sparse_set<entity>*, ecs::entity);
        pool_ptr ptr;
//        meta::TypeId type_id;
        remove_ptr_t remove_ptr;

        void remove(ecs::entity entity) {
            remove_ptr(ptr.get(), entity);
        }
    };

    template<class Component>
    pool_t<Component>& assure() {
        uint32_t idx = component_info<Component>::index();
        if (idx >= pools_.size()) {
            pools_.resize(idx+1);
        }

        auto& [ptr, remove_ptr] = pools_[idx];

        if (!ptr) {
            ptr.reset(new pool_t<Component>);
//            id = meta::GetType<Component>().ID();
            remove_ptr = [] (sparse_set<entity>* ptr, ecs::entity entity) {
                static_cast<pool_t<Component>*>(ptr)->erase(entity);
            };
        }

        return *static_cast<pool_t<Component>*>(ptr.get());
    }

    template<class Component>
    const pool_t<Component>* get_pool() const {
        uint32_t idx = component_info<Component>::index();
        assert(idx < pools_.size());
        return pools_[idx].ptr ? static_cast<pool_t<Component>*>(pools_[idx].ptr.get()) : nullptr;
    }

    template<class Component>
    pool_t<Component>* get_pool() {
        uint32_t idx = component_info<Component>::index();
        assert(idx < pools_.size());
        return pools_[idx].ptr ? static_cast<pool_t<Component>*>(pools_[idx].ptr.get()) : nullptr;
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

//    template<typename Func>
//    void visit(entity_t entity, Func func) const {
//        for (auto& pool : pools_) {
//            if (pool.ptr && pool.ptr->contains(entity)) {
//                func(pool.type_id);
//            }
//        }
//    }

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
                pool.remove(entity);
            }
        }
    }

    template<class Component>
    Component& get(entity entity) {
        assert(has<Component>(entity));
        pool_t<Component>* pool_ptr = get_pool<Component>();
        assert(pool_ptr);
        return pool_ptr->get(entity);
    }

    template<class Component>
    const Component& get(entity entity) const {
        assert(has<Component>(entity));
        const pool_t<Component>* pool_ptr = get_pool<Component>();
        assert(pool_ptr);
        return pool_ptr->get(entity);
    }

    template<class Component>
    Component* try_get(entity entity) {
        assert(valid(entity));
        if (auto pool = get_pool<Component>(); pool) {
            return pool->try_get(entity);
        }
        return nullptr;
    }

    template<class Component>
    const Component* try_get(entity entity) const {
        assert(valid(entity));
        if (auto pool = get_pool<Component>(); pool) {
            return pool->try_get(entity);
        }
        return nullptr;
    }

    template<class Component>
    [[nodiscard]] bool has(entity entity) const {
        assert(valid(entity));
        auto idx = component_info<Component>::index();
        return idx < pools_.size() && pools_[idx].ptr && pools_[idx].ptr->contains(entity);
    }

    template<class ...Component>
    component_view<Component...> view() {
        return component_view<Component...>(assure<Component>()...);
    }

    template<class Component>
    size_t size() const {
        auto idx = component_info<Component>::index();
        return idx < pools_.size() && pools_[idx].ptr && pools_[idx].ptr->size();
    }

private:
    std::vector<pool_info> pools_{};
    std::vector<entity> entities_{};
    size_t free_idx_{invalid_idx};
};

}
