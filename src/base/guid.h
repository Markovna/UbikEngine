#pragma once

#include <array>

class guid {
public:
    guid() = default;

    guid(const guid &) = default;
    guid(guid &&) = default;

    guid &operator=(const guid &) = default;
    guid &operator=(guid &&) = default;

    bool operator==(const guid &) const;
    bool operator!=(const guid &) const;

    explicit operator std::string() const;

    [[nodiscard]] std::string str() const;
    [[nodiscard]] const std::array<uint8_t, 16> &bytes() const;
    [[nodiscard]] bool is_valid() const;

    void swap(guid &);

    static guid from_string(const char*);
    static guid from_string(const std::string&);
    static guid generate();
    static guid invalid() noexcept;

private:
    friend std::ostream &operator<<(std::ostream &, const guid &);
    friend std::istream &operator>>(std::istream &, guid &);
    friend bool operator<(const guid &, const guid &);

private:
    std::array<uint8_t, 16> bytes_ = {0};
};

namespace std {

template <>
struct hash<guid> {

  std::size_t operator()(const guid& guid) const {
    const uint64_t* p = reinterpret_cast<const uint64_t*>(guid.bytes().data());
    std::size_t seed = std::hash<uint64_t>{}(p[1]);
    seed ^= std::hash<uint64_t>{}(p[0]) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);
    return seed;
  }

};

}
