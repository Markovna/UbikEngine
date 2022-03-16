#pragma once

#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "base/crc32.h"
#include "base/iterator_range.h"
#include "base/guid.h"
#include "base/json.hpp"
#include "base/slot_map.h"
#include "base/detector.h"
#include "platform/file_system.h"
#include "base/log.h"

class asset;
class asset_value;
class asset_repository;

struct buffer_id {
  uint32_t idx;
};

struct array_id {
  uint32_t idx;
};

class asset_array {
 public:
  using container_t = std::vector<asset_value>;
  using iterator = container_t::iterator;
  using const_iterator = container_t::const_iterator;

  explicit asset_array(array_id id) : id_(id), owner_(nullptr) {}

  auto begin() const { return values_.begin(); }
  auto end() const { return values_.end(); }

  const asset_value& back() const { return values_.back(); }

  const asset_value& operator[](size_t) const;
  const asset_value& at(size_t) const;

  array_id id() const { return id_; }
  const asset& owner() const { return *owner_; }

  bool empty() const { return values_.empty(); }
  size_t size() const { return values_.size(); }

 private:
  friend class asset_repository;

  void set_owner(asset* owner) { owner_ = owner; }
  asset& owner() { return *owner_; }

  asset_value& back() { return values_.back(); }

  void push_back(asset_value val);
  iterator insert(const_iterator it, asset_value val);
  void pop_back() { values_.pop_back(); }
  iterator erase(const_iterator it) { return values_.erase(it); }

//  auto begin() { return values_.begin(); }
//  auto end() { return values_.end(); }

  auto items() { return iterator_range(values_.begin(), values_.end()); }
  auto items() const { return iterator_range(begin(), end()); }

 private:
  container_t values_;
  asset* owner_;
  array_id id_;
};

struct asset_id {
  uint32_t idx = std::numeric_limits<uint32_t>::max();

  explicit constexpr operator bool() const noexcept { return idx != std::numeric_limits<uint32_t>::max(); }
};

class asset {
 public:
  using key_t = std::string;
  using container_t = std::unordered_map<key_t, asset_value>;
  using iterator = container_t::iterator;
  using const_iterator = container_t::const_iterator;

 public:
  asset() = delete;

  explicit asset(asset_id id)
    : id_(id), owner_(nullptr), version_(0), properties_()
  {}

  asset(const asset&) = delete;
  asset& operator=(const asset&) = delete;

  const asset_value& at(const key_t& key) const {
    return properties_.at(key);
  }

  bool is_orphan() const { return !owner_; }
  const asset& owner() const { return *owner_; }
  uint32_t version() const { return version_; }

  uint32_t increment_version() {
    asset* curr = this;
    while (curr) {
      ++curr->version_;
      curr = curr->owner_;
    }
    return version_;
  }

  bool contains(const key_t& key) const { return properties_.count(key); }

  asset_id id() const { return id_; }

  const_iterator find(const key_t& key) const { return properties_.find(key); }
  const_iterator begin() const { return properties_.begin(); }
  const_iterator end() const { return properties_.end(); }

 private:
  friend class asset_repository;

  asset_value& operator[](const key_t& key) {
    return properties_[key];
  }

  void erase(const key_t& key) {
    properties_.erase(key);
  }

  void erase(const_iterator it) {
    properties_.erase(it);
  }

  auto items() { return iterator_range(properties_.begin(), properties_.end()); }
  auto items() const { return iterator_range(begin(), end()); }

  iterator find_impl(const key_t& key) { return properties_.find(key); }

//  iterator begin() { return properties_.begin(); }
//  iterator end() { return properties_.end(); }

  void set_owner(asset* owner) { owner_ = owner; }

 private:
  container_t properties_ {};
  asset* owner_ {};
  asset_id id_ {};
  uint32_t version_ {};
};

template<typename T, typename... Args>
static T* create(Args&& ... args) {
  auto deleter = [&](T* obj) {
    operator delete(obj);
  };

  std::unique_ptr<T, decltype(deleter)> obj((T*) operator new(sizeof(T)), deleter);
  new (obj.get()) T(std::forward<Args>(args)...);
  assert(obj != nullptr);

  return obj.release();
}

template<typename T>
static void free(T* ptr) {
  ptr->~T();
  operator delete(ptr);
}

template<typename T>
using uncvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

class asset_value {
 public:
  enum class type {
    NONE,
    BOOLEAN,
    INTEGER,
    UNSIGNED,
    FLOAT,
    STRING,
    ARRAY,
    OBJECT,
    BUFFER
  };

 private:
  using string_t = std::string;

  union value {
    std::nullptr_t none;
    bool boolean;
    int64_t number_integer;
    uint64_t number_unsigned;
    float number_float;
    string_t* string;
    asset_array* array;
    asset* object;
    buffer_id buffer;

    value() noexcept : none() {}
    value(bool v) noexcept : boolean(v) {}
    value(int64_t v) noexcept : number_integer(v) {}
    value(uint64_t v) noexcept : number_unsigned(v) {}
    value(float v) noexcept : number_float(v) {}
    value(const string_t& value) : string(create<string_t>(value)) {}
    value(string_t&& value) : string(create<string_t>(std::move(value))) {}
    value(string_t* ptr) : string(ptr) {}
    value(asset* val) : object(val) {}
    value(asset_array* val) : array(val) {}
    value(buffer_id id) : buffer(id) {}

    value(type t) {
      switch (t) {
        case type::BOOLEAN: {
          boolean = false;
          break;
        }
        case type::INTEGER: {
          number_integer = 0;
          break;
        }
        case type::UNSIGNED: {
          number_unsigned = 0;
          break;
        }
        case type::FLOAT: {
          number_float = 0.0;
          break;
        }
        case type::STRING: {
          string = create<string_t>();
          break;
        }
        case type::ARRAY: {
          array = { };
          break;
        }
        case type::OBJECT: {
          object = { };
          break;
        }
        case type::BUFFER: {
          buffer = { };
          break;
        }
        case type::NONE: {
          none = { };
          break;
        }
      }
    }

    void destroy(type t) {
      if (t == type::STRING) {
        free(string);
      }
    }
  };

 private:
  template<class T, class = void>
  struct is_base_numeric_type : std::false_type {};

  template<class T> struct is_base_numeric_type<T,
      std::enable_if_t<
        std::is_same_v<T, float> ||
        std::is_same_v<T, int64_t> ||
        std::is_same_v<T, uint64_t>,
      void>>
    : std::true_type {};

 public:
  static void to_asset_value(asset_value& asset_val, bool val) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::BOOLEAN;
    asset_val.value_ = val;
  }

  static void to_asset_value(asset_value& asset_val, buffer_id val) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::BUFFER;
    asset_val.value_ = val;
  }

  template<class IntegerType,
           class TypeLimits = std::numeric_limits<IntegerType>,
           std::enable_if_t<
             std::is_constructible_v<int64_t, IntegerType> &&
             TypeLimits::is_integer &&
             TypeLimits::is_signed,
           int> = 0>
  static void to_asset_value(asset_value& asset_val, IntegerType val) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::INTEGER;
    asset_val.value_ = static_cast<int64_t>(val);
  }

  template<class UnsignedType,
           class TypeLimits = std::numeric_limits<UnsignedType>,
           std::enable_if_t<
             std::is_constructible_v<uint64_t, UnsignedType> &&
             TypeLimits::is_integer &&
             !TypeLimits::is_signed,
           int> = 0>
  static void to_asset_value(asset_value& asset_val, UnsignedType val) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::UNSIGNED;
    asset_val.value_ = static_cast<uint64_t>(val);
  }

  template<class FloatType,
           std::enable_if_t<std::is_floating_point_v<FloatType>, int> = 0>
  static void to_asset_value(asset_value& asset_val, FloatType val) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::FLOAT;
    asset_val.value_ = static_cast<float>(val);
  }

  static void to_asset_value(asset_value& asset_val, string_t&& val) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::STRING;
    asset_val.value_ = std::move(val);
  }

  static void to_asset_value(asset_value& asset_val, const string_t& val) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::STRING;
    asset_val.value_ = val;
  }

  template<class StringCompatibleType,
           std::enable_if_t<
             !std::is_same_v<string_t, StringCompatibleType> &&
             std::is_constructible_v<string_t, StringCompatibleType>,
           int> = 0>
  static void to_asset_value(asset_value& asset_val, const StringCompatibleType& val) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::STRING;
    asset_val.value_ = create<string_t>(val);
  }

  static void to_asset_value(asset_value& asset_val, asset& ref) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::OBJECT;
    asset_val.value_.object = &ref;
  }

  static void to_asset_value(asset_value& asset_val, asset_array& ref) {
    asset_val.value_.destroy(asset_val.type_);
    asset_val.type_ = type::ARRAY;
    asset_val.value_.array = &ref;
  }

  static void from_asset_value(const asset_value& val, bool& ref) {
    assert(val.is_boolean());
    ref = val.value_.boolean;
  }

  static void from_asset_value(const asset_value& val, buffer_id& ref) {
    assert(val.is_buffer());
    ref = val.value_.buffer;
  }

  static void from_asset_value(const asset_value& val, string_t& ref) {
    assert(val.is_string());
    ref = *val.value_.string;
  }

  template<class ConstructibleStringType,
           std::enable_if_t<
             std::is_constructible_v<ConstructibleStringType, string_t> &&
             !std::is_same_v<string_t, ConstructibleStringType>,
           int> = 0>
  static void from_asset_value(const asset_value& val, ConstructibleStringType& ref) {
    assert(val.is_string());
    ref = *val.value_.string;
  }

  template<class ArithmeticType,
           std::enable_if_t<
             std::is_arithmetic_v<ArithmeticType> &&
             !std::is_same_v<ArithmeticType, bool>,
           int> = 0>
  static void from_asset_value(const asset_value& val, ArithmeticType& ref) {
    assert(val.is_number());
    switch (val.type_) {
      case type::INTEGER: {
        ref = static_cast<ArithmeticType>(val.value_.number_integer);
        break;
      }
      case type::UNSIGNED: {
        ref = static_cast<ArithmeticType>(val.value_.number_unsigned);
        break;
      }
      case type::FLOAT: {
        ref = static_cast<ArithmeticType>(val.value_.number_float);
        break;
      }
      case type::NONE:
      case type::BOOLEAN:
      case type::STRING:
      case type::ARRAY:
      case type::OBJECT:
      default: {
        break;
      }
    }
  }

 private:
  template<class... Args>
  using to_asset_value_function = decltype(to_asset_value(std::declval<Args>()...));

  template<class... Args>
  using from_asset_value_function = decltype(from_asset_value(std::declval<Args>()...));

  template<class T, class = void>
  struct has_to_asset_value : std::false_type {};

  template<class T> struct has_to_asset_value<T,
      std::enable_if_t<is_detected_exact<void, to_asset_value_function, asset_value&, T>::value, void>>
    : std::true_type {};

  template<class T, class = void>
  struct has_from_asset_value : std::false_type {};

  template<class T> struct has_from_asset_value<T,
      std::enable_if_t<is_detected_exact<void, from_asset_value_function, const asset_value&, T&>::value, void>>
    : std::true_type {};

  template<class T, class = void>
  struct is_ptr_enabled : std::false_type {};

  template<class T> struct is_ptr_enabled<T,
      std::enable_if_t<
        std::is_same_v<T, bool> ||
        std::is_same_v<T, float> ||
        std::is_same_v<T, uint64_t> ||
        std::is_same_v<T, int64_t> ||
        std::is_same_v<T, string_t> ||
        std::is_same_v<T, asset> ||
        std::is_same_v<T, asset_array>,
      void>>
    : std:: true_type {};

 public:
  asset_value(type t)
    : type_(t), value_(t)
  {}

  asset_value(std::nullptr_t = nullptr)
    : asset_value(type::NONE)
  {}

  asset_value(const asset_value& other) = delete;
  asset_value& operator=(const asset_value&) = delete;

  asset_value(asset_value&& other) noexcept
    : type_(std::move(other.type_)),
      value_(std::move(other.value_)) {
    other.type_ = type::NONE;
    other.value_ = {};
  }

  asset_value& operator=(asset_value&& other) noexcept {
    asset_value tmp(std::move(other));
    using std::swap;
    swap(type_, tmp.type_);
    swap(value_, tmp.value_);
    return *this;
  }

  template<class T,
           class U = uncvref_t<T>,
           std::enable_if_t<
             !std::is_same_v<U, asset_value> &&
             !std::is_same_v<U, asset> &&
             has_to_asset_value<U>::value,
           int> = 0>
  asset_value(T&& val) {
    to_asset_value(*this, std::forward<T>(val));
  }

  asset_value(asset& val) : type_(type::NONE), value_() {
    to_asset_value(*this, val);
  }

  asset_value(asset_array& val) {
    to_asset_value(*this, val);
  }

  ~asset_value() noexcept {
    value_.destroy(type_);
  }

 private:
  template<
      class ReferenceType,
      std::enable_if_t<
        std::is_reference_v<ReferenceType>,
      int> = 0>
  ReferenceType get_ref() {
    auto* ptr = get_ptr<std::add_pointer_t<ReferenceType>>();
    assert(ptr);
    return *ptr;
  }

  template<
      class ReferenceType,
      std::enable_if_t<
        std::is_reference_v<ReferenceType> &&
        std::is_const_v<std::remove_reference_t<ReferenceType>>,
      int> = 0>
  ReferenceType get_ref() const {
    auto* ptr = get_ptr<std::add_pointer_t<ReferenceType>>();
    assert(ptr);
    return *ptr;
  }

 public:

  template<
     class ValueTypeCV,
     class ValueType = uncvref_t<ValueTypeCV>,
     std::enable_if_t<
       !std::is_reference_v<ValueTypeCV> &&
       has_from_asset_value<ValueType>::value,
     int> = 0>
  constexpr ValueType get() const {
    ValueType ret;
    from_asset_value(*this, ret);
    return ret;
  }

  template<
      class ValueTypeCV,
      class ValueType = uncvref_t<ValueTypeCV>,
      std::enable_if_t<
              std::is_reference_v<ValueTypeCV> &&
              is_ptr_enabled<ValueType>::value,
          int> = 0>
  constexpr decltype(auto) get() const {
    return get_ref<const ValueType&>();
  }

  template<
      class ValueTypeCV,
      class ValueType = uncvref_t<ValueTypeCV>,
      std::enable_if_t<
          std::is_reference_v<ValueTypeCV> &&
              is_ptr_enabled<ValueType>::value,
          int> = 0>
  constexpr decltype(auto) get() {
    return get_ref<ValueType&>();
  }

  template<
      class ValueType,
      std::enable_if_t<!is_ptr_enabled<ValueType>::value, int> = 0>
  operator ValueType() const {
    return get<ValueType>();
  }

  template<
      class ValueType,
      std::enable_if_t<
        is_ptr_enabled<ValueType>::value,
      int> = 0>
  operator const ValueType&() const {
    return get_ref<const ValueType&>();
  }

//  operator asset&() const {
//    auto* ptr = get_ptr_impl(static_cast<asset*>(nullptr));
//    assert(ptr);
//    return *ptr;
//  }

  template<
      class ValueType,
      std::enable_if_t<
        is_ptr_enabled<ValueType>::value,
      int> = 0>
  operator ValueType&() {
    return get_ref<ValueType&>();
  }

  constexpr bool is_primitive() const noexcept {
    return is_null() || is_string() || is_boolean() || is_number();
  }

  constexpr bool is_null() const noexcept {
    return type_ == type::NONE;
  }

  constexpr bool is_number() const noexcept {
    return is_number_integer() || is_number_float();
  }

  constexpr bool is_number_float() const noexcept {
    return type_ == type::FLOAT;
  }

  constexpr bool is_number_integer() const noexcept {
    return type_ == type::UNSIGNED || type_ == type::INTEGER;
  }

  constexpr bool is_number_unsigned() const noexcept {
    return type_ == type::UNSIGNED;
  }

  constexpr bool is_boolean() const noexcept {
    return type_ == type::BOOLEAN;
  }

  constexpr bool is_buffer() const noexcept {
    return type_ == type::BUFFER;
  }

  constexpr bool is_object() const noexcept {
    return type_ == type::OBJECT;
  }

  constexpr bool is_array() const noexcept {
    return type_ == type::ARRAY;
  }

  constexpr bool is_string() const noexcept {
    return type_ == type::STRING;
  }

  static asset_value copy(const asset_value& other) {
    assert(other.is_primitive());

    asset_value copy;
    copy.type_ = other.type_;
    copy.value_ = other.type_ == type::STRING ? create<string_t>(*other.value_.string) : other.value_;
    return copy;
  }

  constexpr operator type() const noexcept {
    return type_;
  }

 private:
  template<
      class PointerType,
      std::enable_if_t<
        std::is_pointer_v<PointerType> &&
        std::is_const_v<std::remove_pointer_t<PointerType>>,
      int> = 0>
  auto get_ptr() const {
    return get_ptr_impl(static_cast<PointerType>(nullptr));
  }

  template<class PointerType,
           std::enable_if_t<std::is_pointer_v<PointerType>, int> = 0>
  auto get_ptr() {
    return get_ptr_impl(static_cast<PointerType>(nullptr));
  }

  bool* get_ptr_impl(bool*) {
    return is_boolean() ? &value_.boolean : nullptr;
  }

  constexpr const bool* get_ptr_impl(const bool*) const {
    return is_boolean() ? &value_.boolean : nullptr;
  }

  int64_t* get_ptr_impl(int64_t*) {
    return is_number_integer() ? &value_.number_integer : nullptr;
  }

  constexpr const int64_t* get_ptr_impl(const int64_t*) const {
    return is_number_integer() ? &value_.number_integer : nullptr;
  }

  uint64_t* get_ptr_impl(uint64_t*) {
    return is_number_unsigned() ? &value_.number_unsigned : nullptr;
  }

  constexpr const uint64_t* get_ptr_impl(const uint64_t*) const {
    return is_number_unsigned() ? &value_.number_unsigned : nullptr;
  }

  float* get_ptr_impl(float*) {
    return is_number_float() ? &value_.number_float : nullptr;
  }

  constexpr const float* get_ptr_impl(const float*) const {
    return is_number_float() ? &value_.number_float : nullptr;
  }

  string_t* get_ptr_impl(string_t*) {
    return is_string() ? value_.string : nullptr;
  }

  constexpr const string_t* get_ptr_impl(const string_t*) const {
    return is_string() ? value_.string : nullptr;
  }

  asset_array* get_ptr_impl(asset_array*) {
    return is_array() ? value_.array : nullptr;
  }

  constexpr const asset_array* get_ptr_impl(const asset_array*) const {
    return is_array() ? value_.array : nullptr;
  }

  asset* get_ptr_impl(asset*) {
    return is_object() ? value_.object : nullptr;
  }

  constexpr const asset* get_ptr_impl(const asset*) const {
    return is_object() ? value_.object : nullptr;
  }

 private:
  value value_ = { };
  type type_ = type::NONE;
};

class asset_buffer {
 public:
  size_t size() const { return size_; }
  const uint8_t* data() const { return ptr_.get(); }

  asset_buffer(size_t size, std::shared_ptr<uint8_t> ptr)
    : size_(size), ptr_(std::move(ptr))
  {}

 private:
  size_t size_;
  std::shared_ptr<uint8_t> ptr_;
};

struct buffer_info {
  size_t size;
  fs::path path;
  size_t offset;
  uint32_t hash;
  mutable std::weak_ptr<uint8_t> weak_ptr;
  mutable std::shared_ptr<uint8_t> loaded_ptr;

  void destroy() {
    loaded_ptr.reset();
    weak_ptr.reset();
    path.clear();
    size = 0;
    offset = 0;
    hash = 0;
  }
};

class asset_repository {
 public:
  buffer_id create_buffer_from_file(const fs::path& path, uint32_t offset, uint32_t size) {
    assert(!path.empty());
    assert(fs::exists(path));

    if (size == 0) {
      size = fs::file_size(path);
    }

    uint32_t index;
    if (!buffers_free_list_.empty()) {
      index = buffers_free_list_.back();
      buffers_free_list_.pop_back();
    } else {
      index = buffers_.size();
      buffers_.resize(index+1);
    }

    buffers_[index].path = std::move(path);
    buffers_[index].size = size;
    buffers_[index].offset = offset;
    return { index };
  }

  void map_buffer_to_file(buffer_id id, const fs::path& path, uint32_t offset = 0, uint32_t size = 0) {
    assert(!path.empty());
    assert(fs::exists(path));

    auto& buf = buffers_[id.idx];
    buf.path = path;
    buf.loaded_ptr.reset();
    buf.offset = offset;
    buf.size = size == 0 ? fs::file_size(path) : size;
  }

  void write_buffer_to_file(buffer_id id, const fs::path& path, uint32_t offset, bool remap = false) {
    auto& buf = buffers_[id.idx];
    std::ofstream dst(path, std::ios::out | std::ios::binary);
    if (offset) dst.rdbuf()->pubseekoff(offset, std::ios::beg, std::ios::in | std::ios::out);

    if (!buf.weak_ptr.expired()) {
      std::copy_n(buf.weak_ptr.lock().get(), buf.size, std::ostreambuf_iterator<char>(dst));
    } else {
      std::ifstream src(buf.path, std::ios::in | std::ios::binary);
      if (buf.offset) src.rdbuf()->pubseekoff(buf.offset, std::ios::beg, std::ios::in | std::ios::out);
      std::copy_n(std::istreambuf_iterator<char>(src), buf.size, std::ostreambuf_iterator<char>(dst));
    }
  }

  void update_buffer(buffer_id id, uint32_t offset, const void* data, uint32_t size) {
    buffer_info& buf = buffers_[id.idx];
    auto ptr = buf.weak_ptr.lock();
    if (!ptr) {
      ptr = std::shared_ptr<uint8_t>(new uint8_t[buf.size], std::default_delete<uint8_t[]>());
      buf.weak_ptr = ptr;
    }

    buf.loaded_ptr = ptr; // don't delete loaded memory until buffer is not saved to file
    buf.hash = 0;
    std::memcpy(ptr.get() + offset, data, size);
  }

  void destroy_buffer(buffer_id id) {
    buffers_[id.idx].loaded_ptr.reset();
    assert(buffers_[id.idx].weak_ptr.expired());

    buffers_free_list_.push_back(id.idx);
    buffers_[id.idx].destroy();
  }

  asset_buffer load_buffer(buffer_id id) {
    buffer_info& buf = buffers_[id.idx];
    auto ptr = buf.weak_ptr.lock();
    if (!ptr) {
      ptr = std::shared_ptr<uint8_t>(new uint8_t[buf.size], std::default_delete<uint8_t[]>());
      buf.weak_ptr = ptr;

      std::ifstream file(buf.path, std::ios::in | std::ios::out | std::ios::binary);
      if (buf.offset) file.rdbuf()->pubseekoff(buf.offset, std::ios::beg, std::ios::in | std::ios::out);
      std::copy_n(std::istreambuf_iterator<char>(file), buf.size, ptr.get());
    }

    return { buf.size, ptr };
  }

  uint32_t buffer_hash(buffer_id id) {
    buffer_info& buf = buffers_[id.idx];
    if (!buf.hash) {
      auto data = load_buffer(id);
      buf.hash = utils::crc32(data.data(), data.size());
    }

    return buf.hash;
  }

  asset& create_asset() {
    return create_asset({});
  }

  asset& create_asset(guid guid) {
    if (!guid.is_valid())
      guid = guid::generate();

    uint32_t index;
    if (!objects_free_list_.empty()) {
      index = objects_free_list_.back();
      objects_free_list_.pop_back();
    } else {
      index = objects_.size();
      objects_.resize(index+1);
    }

    asset* ptr = (objects_[index] = std::make_unique<asset>(asset_id { index })).get();

    if (auto it = guid_to_asset_.find(guid); it != guid_to_asset_.end()) {
      logger::core::Error("Couldn't create asset with guid {} because it has been reserved.", guid.str());
      guid = guid::generate();
    }

    guid_to_asset_[guid] = ptr;
    asset_to_info_[ptr].id = guid;
    return *ptr;
  }

  void destroy_asset(asset_id id) {
    asset& a = *objects_[id.idx];
    assert(a.is_orphan());

    destroy_asset_value_recursive(a);
  }

  const fs::path& get_asset_path(asset_id asset_id) {
    assert(objects_[asset_id.idx]);
    auto a = objects_[asset_id.idx].get();
    return asset_to_info_[a].path;
  }

  bool set_asset_path(asset_id asset_id, const fs::path& p) {
    assert(objects_[asset_id.idx]);
    auto a = objects_[asset_id.idx].get();
    auto& existed = path_to_asset_[p];
    if (existed != nullptr && existed != a)
      return false;

    existed = a;
    asset_to_info_[a].path = p;
    return true;
  }

  const guid& get_guid(const asset& asset) {
    return get_guid(asset.id());
  }

  const guid& get_guid(asset_id asset_id) {
    assert(objects_[asset_id.idx]);
    auto a = objects_[asset_id.idx].get();
    return asset_to_info_[a].id;
  }

  asset* get_asset(asset_id id) {
    if (id.idx >= objects_.size() || !objects_[id.idx])
      return nullptr;

    return objects_[id.idx].get();
  }

  asset* get_asset_by_path(const fs::path& p) const {
    auto it = path_to_asset_.find(p);
    return it != path_to_asset_.end() ? it->second : nullptr;
  }

  asset* get_asset(const guid& id) const {
    auto it = guid_to_asset_.find(id);
    return it != guid_to_asset_.end() ? it->second : nullptr;
  }

  void copy_value(asset& a, const asset::key_t& key, const asset_value& value) {
    set_value(a, key, copy_asset_value(value));
  }

  void set_value(asset& a, const asset::key_t& key, asset_value value) {
    auto it = a.find_impl(key);
    if (it != a.end()) {
      destroy_asset_value_recursive(std::move(it->second));
    }

    set_owner(value, &a);

    a[key] = std::move(value);
    a.increment_version();
  }

  void set_value(asset_id asset_id, const asset::key_t& key, asset_value value) {
    assert(objects_[asset_id.idx]);
    set_value(*objects_[asset_id.idx], key, std::move(value));
  }

  void set_ref(asset_id id, const asset::key_t& key, asset_id value) {
    assert(objects_[id.idx]);
    set_ref(*objects_[id.idx], key, value);
  }

  void set_ref(asset& a, const asset::key_t& key, asset_id value) {
    auto& guid = get_guid(value);
    set_value(a, key, guid.str());
  }

  void erase(asset& a, const asset::key_t& key) {
    auto it = a.find_impl(key);
    if (it == a.end())
      return;

    destroy_asset_value_recursive(std::move(it->second));
    a.erase(key);
  }

  void erase(asset_id asset_id, const asset::key_t& key) {
    assert(objects_[asset_id.idx]);
    erase(*objects_[asset_id.idx], key);
  }

  asset_array& create_array() {
    uint32_t index;
    if (!arrays_free_list_.empty()) {
      index = arrays_free_list_.back();
      arrays_free_list_.pop_back();
    } else {
      index = arrays_.size();
      arrays_.resize(index+1);
    }

    return *(arrays_[index] = std::make_unique<asset_array>(array_id { index }));
  }

//  void destroy_array(asset_array& arr) {
//    destroy_asset_value_recursive(arr);
//  }

  void push_back(array_id id, asset_value val) {
    assert(arrays_[id.idx]);
    push_back(*arrays_[id.idx], std::move(val));
  }

  void push_back(asset_array& array, asset_value val) {
    set_owner(val, &array.owner());

    array.push_back(std::move(val));
  }

  void pop_back(asset_array& array) {
    destroy_asset_value_recursive(std::move(array.back()));
    array.pop_back();
  }

  void pop_back(array_id id) {
    assert(arrays_[id.idx]);
    pop_back(*arrays_[id.idx]);
  }

  buffer_id create_buffer(uint32_t size) {
    buffer_id id = add_buffer();
    buffer_info& buffer = get_buffer_info(id);

    auto ptr = std::shared_ptr<uint8_t>(new uint8_t[size ? size : 1], std::default_delete<uint8_t[]>());
    if (size) {
      std::memset(ptr.get(), 0, size);
    }

    buffer.size = size;
    buffer.path = {};
    buffer.offset = 0;
    buffer.weak_ptr = ptr;
    buffer.loaded_ptr = ptr; // don't delete memory until buffer is not mapped to file
    return id;
  }

  asset_array::const_iterator erase(asset_array& array, asset_array::const_iterator it) {
    auto ps = it - array.begin();
    auto ptr = array.items().begin() + ps;
    destroy_asset_value_recursive(std::move(*ptr));

    return array.erase(it);
  }

  asset_array::const_iterator erase(array_id id, asset_array::const_iterator it) {
    assert(arrays_[id.idx]);
    return erase(*arrays_[id.idx], it);
  }

  asset_array::const_iterator insert(array_id id, asset_array::const_iterator it, asset_value val) {
    assert(arrays_[id.idx]);
    return insert(*arrays_[id.idx], it, std::move(val));
  }

  asset_array::const_iterator insert(asset_array& array, asset_array::const_iterator it, asset_value val) {
    set_owner(val, &array.owner());
    return array.insert(it, std::move(val));
  }

  auto begin() const { return asset_to_info_.begin(); }
  auto end() const { return asset_to_info_.end(); }

 private:

  void set_owner(asset_value& value, asset* owner) {
    if (value.is_object()) {
      asset& obj = value;
      obj.set_owner(owner);
    } else if (value.is_array()) {
      std::vector<asset_array*> queue;
      queue.push_back(&value.get<asset_array&>());
      while(!queue.empty()) {
        asset_array* next = queue.back();
        queue.pop_back();

        next->set_owner(owner);
        for (auto& item : next->items()) {
          if (item.is_object()) {
            asset& obj = item;
            obj.set_owner(owner);
          } else if (item.is_array()) {
            queue.push_back(&item.get<asset_array&>());
          }
        }
      }
    }
  }

  void free_asset(asset_id id) {
    auto& ptr = objects_[id.idx];

    if (auto it = asset_to_info_.find(ptr.get()); it != asset_to_info_.end()) {
      if (it->second.id.is_valid()) {
        guid_to_asset_.erase(it->second.id);
      }
      if (!it->second.path.empty()) {
        path_to_asset_.erase(it->second.path);
      }
      asset_to_info_.erase(it);
    }

    objects_free_list_.push_back(id.idx);
    ptr.reset();
  }

  void free_array(array_id id) {
    arrays_free_list_.push_back(id.idx);
    arrays_[id.idx].reset();
  }

  asset_value copy_asset_value(const asset_value& src) {
    if (src.is_object()) {
      asset& obj_dst = create_asset();
      for (auto& [name, val] : static_cast<const asset&>(src)) {
        set_value(obj_dst, name, copy_asset_value(val));
      }
      return obj_dst;
    }

    if (src.is_array()) {
      asset_array& arr_dst = create_array();
      for (auto& val : static_cast<const asset_array&>(src)) {
        push_back(arr_dst, copy_asset_value(val));
      }
      return arr_dst;
    }

    if (src.is_buffer()) {
      buffer_id dst_id = add_buffer();
      buffer_info& buf_dst = get_buffer_info(dst_id);
      buf_dst = get_buffer_info(src);
      return dst_id;
    }

    return asset_value::copy(src);
  }

  void destroy_asset_value_recursive(asset_value value) {
    std::vector<asset_value> queue;
    queue.emplace_back(std::move(value));
    while (!queue.empty()) {
      asset_value back = std::move(queue.back());
      queue.pop_back();
      if (back.is_object()) {
        auto& obj = static_cast<asset&>(back);
        for (auto& [_, sub] : obj.items()) {
          if (!sub.is_primitive()) {
            queue.emplace_back(std::move(sub));
          }
        }
        free_asset(obj.id());
      } else if (back.is_array()) {
        auto& arr = static_cast<asset_array&>(back);
        for (auto& sub : arr.items()) {
          if (!sub.is_primitive()) {
            queue.emplace_back(std::move(sub));
          }
        }
        free_array(arr.id());
      } else if (back.is_buffer()) {
        destroy_buffer(back);
      }
    }
  }

  buffer_info& get_buffer_info(buffer_id id) {
    return buffers_[id.idx];
  }

  buffer_id add_buffer() {
    uint32_t index;
    if (!buffers_free_list_.empty()) {
      index = buffers_free_list_.back();
      buffers_free_list_.pop_back();
    } else {
      index = buffers_.size();
      buffers_.resize(index+1);
    }

    return { index };
  }

 private:
  std::vector<std::unique_ptr<asset>> objects_;
  std::vector<std::unique_ptr<asset_array>> arrays_;
  std::vector<uint32_t> objects_free_list_;
  std::vector<uint32_t> arrays_free_list_;

  std::vector<buffer_info> buffers_;
  std::vector<uint32_t> buffers_free_list_;

  struct asset_info {
    guid id;
    fs::path path;
  };

  std::unordered_map<guid, asset*> guid_to_asset_;
  std::unordered_map<std::string, asset*> path_to_asset_;
  std::unordered_map<asset*, asset_info> asset_to_info_;
};

nlohmann::json asset_to_json(const asset_value& val, asset_repository& rep, std::vector<buffer_id>& buffers);
asset_value parse_json(nlohmann::json& j, asset_repository& rep, const fs::path& buffers_path);
