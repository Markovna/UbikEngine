#pragma once

#include <string>

class memory_reader {
 public:
  explicit memory_reader(const uint8_t* data, size_t size) : data_(data), size_(size), pos_(0) {}

  size_t read(void* data, size_t size) {
    size_t rem = size_ - pos_;
    size = std::min(size, rem);
    std::memcpy(data, &data_[pos_], size);
    pos_ += size;
    return size;
  }

 private:
  const uint8_t* data_;
  size_t size_;
  size_t pos_;
};

template<class T, class = typename std::enable_if<std::is_trivial_v<T>, T>::type>
void read(memory_reader &reader, T& value) {
  uint8_t data[sizeof(T)];
  reader.read((void *) data, sizeof(T));
  value = *reinterpret_cast<T*>(data);
}

inline bool read(memory_reader &reader, char *data, uint32_t size) {
  return reader.read(data, size) == size;
}