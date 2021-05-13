#pragma once

#include "slot_map.h"
#include "delegate.h"

template <class ...A>
class event {
public:
    using delegate_t = delegate<void(A...)>;
    using container_t = stdext::slot_map<delegate_t>;
    using key_t = typename container_t::key_type;

    template <class T>
    key_t connect(T* const object_ptr, void (T::* const method_ptr)(A...)) {
        return delegates_.emplace(object_ptr, method_ptr);
    }

    template <class T>
    key_t connect(const T* const object_ptr, void (T::* const method_ptr)(A...) const) {
        return delegates_.emplace(object_ptr, method_ptr);
    }

    template <class T>
    key_t connect(T& object_ptr, void (T::* const method_ptr)(A...)) {
        return delegates_.emplace(object_ptr, method_ptr);
    }

    template <class T>
    key_t connect(const T& object_ptr, void (T::* const method_ptr)(A...) const) {
        return delegates_.emplace(object_ptr, method_ptr);
    }

    template <
        class T,
        class = std::enable_if_t<!std::is_same<event, std::decay_t<T>>::value>
    >
    key_t connect(T&& functor) {
        return delegates_.emplace(functor);
    }

    void disconnect_all() {
        delegates_.clear();
    }

    void disconnect(const key_t& key) {
        delegates_.erase(key);
    }

    template <class T>
    void disconnect(T* const object_ptr, void (T::* const method_ptr)(A...)) {
        disconnect_by_value(delegate_t(object_ptr, method_ptr));
    }

    template <class T>
    void disconnect(const T* const object_ptr, void (T::* const method_ptr)(A...) const) {
        disconnect_by_value(delegate_t(object_ptr, method_ptr));
    }

    template <class T>
    void disconnect(T& object_ptr, void (T::* const method_ptr)(A...)) {
        disconnect_by_value(delegate_t(object_ptr, method_ptr));
    }

    template <class T>
    void disconnect(const T& object_ptr, void (T::* const method_ptr)(A...) const) {
        disconnect_by_value(delegate_t(object_ptr, method_ptr));
    }

    template <
        class T,
        class = std::enable_if_t<!std::is_same<key_t, std::decay_t<T>>::value>,
        class = std::enable_if_t<!std::is_same<event, std::decay_t<T>>::value>
    >
    void disconnect(T&& functor) {
        disconnect_by_value(delegate_t(std::forward<T>(functor)));
    }

    void operator()(A... args) const {
        for (const delegate_t& delegate : delegates_) {
            delegate(std::forward<A>(args)...);
        }
    }
private:
    void disconnect_by_value(const delegate_t& delegate) {
        auto it = std::find(delegates_.begin(), delegates_.end(), delegate);
        if (it != delegates_.end()) {
            delegates_.erase(it);
        }
    }

private:
    stdext::slot_map<delegate<void(A...)>> delegates_;
};