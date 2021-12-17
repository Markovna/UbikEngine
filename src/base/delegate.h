#pragma once

#include <new>
#include <type_traits>
#include <assert.h>

namespace delegate_details {

template <class T, class R, class... A>
struct member_pair {
    T* const object_ptr;
    R (T::* const method_ptr)(A...);

    R operator()(A... args) const {
        return (object_ptr->*method_ptr)(std::forward<A>(args)...);
    }

    inline bool operator ==(const member_pair& other) {
        return object_ptr == other.object_ptr && method_ptr == other.method_ptr;
    }
};

template <class T, class R, class... A>
struct const_member_pair {
    const T* const object_ptr;
    R (T::* const method_ptr)(A...) const;

    R operator()(A... args) const {
        return (object_ptr->*method_ptr)(std::forward<A>(args)...);
    }

    inline bool operator ==(const const_member_pair& other) {
        return object_ptr == other.object_ptr && method_ptr == other.method_ptr;
    }
};

class undefined;

union locally_stored_types {
    member_pair<undefined, void> member_function_ptr_pair;
    void (*function_ptr)();
    void* object_ptr;
};

constexpr static const size_t max_size  = sizeof(locally_stored_types);
constexpr static const size_t max_align = alignof(locally_stored_types);

using storage = std::aligned_storage_t<max_size, max_align>;

template<class R, class ...A>
struct dispatcher {
    template<class T>
    constexpr static const bool stored_locally =
            std::is_trivially_copyable<T>::value &&
            sizeof(T) <= max_size && alignof(T) <= max_align && (max_align % alignof(T) == 0);

    template<class T>
    using local_storage = std::integral_constant<bool, stored_locally<T>>;
    template<class T>
    using is_copiable = typename std::is_copy_constructible<T>::type;

    template <class>
    struct is_member_pair : std::false_type {};

    template <class T>
    struct is_member_pair<member_pair<T, R, A...>> : std::true_type {};

    template <class T>
    struct is_member_pair<const_member_pair<T, R, A...>> : std::true_type {};

    template <class T>
    using is_function_pointer =
    std::integral_constant<bool, std::is_pointer<T>::value &&
                                 std::is_function<typename std::remove_pointer<T>::type>::value>;

    template <class T>
    using is_member_pair_or_function_pointer =
    std::integral_constant<bool, is_function_pointer<T>::value || is_member_pair<T>::value>;


    template<class T>
    static T& access(void* const object_ptr) {
        return access_impl<T>(object_ptr, local_storage<T>());
    }

    template<class T>
    static R invoke(void* const object_ptr, A... args) {
        return access<T>(object_ptr)(std::forward<A>(args)...);
    }

    template<class T>
    static void copy(void* const dst_ptr, void* const src_ptr) {
        copy_impl<T>(dst_ptr, src_ptr, is_copiable<T>());
    }

    template<class T>
    static void destroy(void* const object_ptr) {
        destroy_impl<T>(object_ptr, local_storage<T>());
    }

    template<class T>
    static void create(T&& object, void* store_ptr) {
        using F = std::decay_t<T>;
        create_impl(std::forward<T>(object), store_ptr, local_storage<F>());
    }

    template<class T>
    static bool compare(void* const lhs_ptr, void* const rhs_ptr) {
        return compare_impl<T>(lhs_ptr, rhs_ptr, is_member_pair_or_function_pointer<T>());
    }

    template<class T>
    static bool compare_impl(void* const lhs_ptr, void* const rhs_ptr, std::true_type) {
        return access<T>(lhs_ptr) == access<T>(rhs_ptr);
    }

    template<class T>
    static bool compare_impl(void* const lhs_ptr, void* const rhs_ptr, std::false_type) {
        return &access<T>(lhs_ptr) == &access<T>(rhs_ptr);
    }

    template<class T>
    static T& access_impl(void* const object_ptr, std::true_type) {
        return *static_cast<T*>(object_ptr);
    }

    template<class T>
    static T& access_impl(void* const object_ptr, std::false_type) {
        return **static_cast<T**>(object_ptr);
    }

    template<class T>
    static void copy_impl(void* const dst_ptr, void* const src_ptr, std::true_type /* is_copiable */) {
        create_impl(access<T>(src_ptr), dst_ptr, local_storage<T>());
    }

    template<class T>
    static void copy_impl(void* const dst_ptr, void* const src_ptr, std::false_type /* is_copiable */) {
        assert(false && "cannot copy delegate constructed from non-copyable type");
    }

    template<class T>
    static void create_impl(T&& object, void* store_ptr, std::true_type /* stored_locally */) {
        using F = std::decay_t<T>;
        new (store_ptr) F (std::forward<T>(object));
    }

    template<class T>
    static void create_impl(T&& object, void* store_ptr, std::false_type /* stored_locally */) {
        using F = std::decay_t<T>;
        new (store_ptr) F* (new F (std::forward<T>(object)));
    }

    template<class T>
    static void destroy_impl(void* const object_ptr, std::true_type /* stored_locally */) {
        access<T>(object_ptr).~T();
    }

    template<class T>
    static void destroy_impl(void* const object_ptr, std::false_type /* stored_locally */) {
        delete &access<T>(object_ptr);
    }
};

template<class R, class... A>
struct vtable {
    using invoke_ptr_t  = R (*)(void* const, A...);
    using destroy_ptr_t = void (*)(void* const);
    using copy_ptr_t    = void (*)(void* const, void* const);
    using compare_ptr_t = bool (*)(void* const, void* const);

    invoke_ptr_t  invoke_ptr;
    destroy_ptr_t destroy_ptr;
    copy_ptr_t    copy_ptr;
    compare_ptr_t compare_ptr;

    vtable(const vtable&) = delete;
    vtable(vtable&&) = delete;

    vtable& operator= (const vtable&) = delete;
    vtable& operator= (vtable&&) = delete;

    ~vtable() = default;
};

}

template <class T>
class delegate;

template <class R, class... A>
class delegate<R(A...)>
{
private:
    template<class T>
    using member_pair = delegate_details::member_pair<T, R, A...>;
    template<class T>
    using const_member_pair = delegate_details::const_member_pair<T, R, A...>;

    using vtable_t = delegate_details::vtable<R, A...>;
    using storage = delegate_details::storage;
    using dispatcher = delegate_details::dispatcher<R, A...>;

    using vtable_ptr_t = const vtable_t*;

private:
    mutable storage store_;
    vtable_ptr_t vtable_ptr_ = nullptr;

public:
    delegate() noexcept = default;

    template <
        class T,
        class = std::enable_if_t<!std::is_same<delegate, std::decay_t<T>>::value>
    >
    delegate(T&& functor) {
      using F = std::decay_t<T>;
      static const vtable_t vt {
        dispatcher::template invoke<F>,
        dispatcher::template destroy<F>,
        dispatcher::template copy<F>,
        dispatcher::template compare<F>
      };

      dispatcher::create(std::forward<T>(functor), std::addressof(store_));
      vtable_ptr_ = &vt;
    }

    delegate(const delegate& other) : vtable_ptr_(other.vtable_ptr_) {
        if (vtable_ptr_) {
            vtable_ptr_->copy_ptr(std::addressof(store_), std::addressof(other.store_));
        }
    }

    delegate(delegate&& other) noexcept {
        swap(other);
    }

    template <class T>
    delegate(T* const object_ptr, R (T::* const method_ptr)(A...))
        : delegate(member_pair<T>{object_ptr, method_ptr})
    {}

    template <class T>
    delegate(const T* const object_ptr, R (T::* const method_ptr)(A...) const)
        : delegate(const_member_pair<T>{object_ptr, method_ptr})
    {}

    template <class T>
    delegate(T& object_ptr, R (T::* const method_ptr)(A...))
            : delegate(member_pair<T>{&object_ptr, method_ptr})
    {}

    template <class T>
    delegate(const T& object_ptr, R (T::* const method_ptr)(A...) const)
            : delegate(const_member_pair<T>{&object_ptr, method_ptr})
    {}

    delegate(std::nullptr_t) noexcept : delegate() {}

    ~delegate() {
        if (vtable_ptr_)
            vtable_ptr_->destroy_ptr(std::addressof(store_));
    }

    delegate& operator= (const delegate& other) noexcept {
        delegate(other).swap(*this);
        return *this;
    }

    delegate& operator= (delegate&& other) noexcept {
        delegate(std::move(other)).swap(*this);
        return *this;
    }

    template <class T>
    delegate& operator= (T&& functor) {
        delegate(std::forward<T>(functor)).swap(*this);
        return *this;
    }

    delegate& operator= (std::nullptr_t) {
        if (vtable_ptr_) {
            vtable_ptr_->destroy_ptr(std::addressof(store_));
            vtable_ptr_ = nullptr;
        }
        return *this;
    }

    constexpr bool operator== (std::nullptr_t) const noexcept {
        return !operator bool();
    }

    constexpr bool operator!= (std::nullptr_t) const noexcept {
        return operator bool();
    }

    explicit constexpr operator bool() const noexcept {
        return vtable_ptr_ != nullptr;
    }

    bool operator== (const delegate& other) const noexcept {
        if (!vtable_ptr_)
            return other == nullptr;

        return vtable_ptr_->compare_ptr(std::addressof(store_), std::addressof(other.store_));
    }

    bool operator!= (const delegate& other) const noexcept {
        return !(*this == other);
    }

    R operator()(A... args) const {
        return vtable_ptr_->invoke_ptr(std::addressof(store_), std::forward<A>(args)...);
    }

private:
    void swap(delegate& other) noexcept {
        std::swap(store_,      other.store_);
        std::swap(vtable_ptr_, other.vtable_ptr_);
    }
};

template<class R, class ...Args>
auto make_delegate(R(* const method_ptr)(Args...)) {
  return delegate<R(Args...)>(method_ptr);
}