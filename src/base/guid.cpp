#include "guid.h"
#include <iostream>
#include <iomanip>

#ifdef _WIN32
 #define GUID_WINDOWS
 #include <objbase.h>
#elif defined(__APPLE__) || defined(__MACH__)
 #define GUID_CFUUID
 #include <CoreFoundation/CFUUID.h>
#else
 #define GUID_LIBUUID
 #include <uuid/uuid.h>
#endif

bool guid::operator==(const guid &other) const {
    return bytes_ == other.bytes_;
}

bool guid::operator!=(const guid &other) const {
    return !(*this == other);
}

void guid::swap(guid &other) {
    bytes_.swap(other.bytes_);
}

const std::array<uint8_t, 16> &guid::bytes() const {
    return bytes_;
}

std::string guid::str() const {
    char one[34];
    snprintf(one, 34, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
             bytes_[0], bytes_[1], bytes_[2], bytes_[3],
             bytes_[4], bytes_[5], bytes_[6], bytes_[7],
             bytes_[8], bytes_[9], bytes_[10], bytes_[11],
             bytes_[12], bytes_[13], bytes_[14], bytes_[15]
    );

    std::string out(one);
    return out;
}

guid::operator std::string() const {
    return str();
}

std::ostream &operator<<(std::ostream &s, const guid &guid) {
    std::ios_base::fmtflags f(s.flags());
    s << std::hex << std::setfill('0')
      << std::setw(2) << (int) guid.bytes_[0]
      << std::setw(2) << (int) guid.bytes_[1]
      << std::setw(2) << (int) guid.bytes_[2]
      << std::setw(2) << (int) guid.bytes_[3]
      << std::setw(2) << (int) guid.bytes_[4]
      << std::setw(2) << (int) guid.bytes_[5]
      << std::setw(2) << (int) guid.bytes_[6]
      << std::setw(2) << (int) guid.bytes_[7]
      << std::setw(2) << (int) guid.bytes_[8]
      << std::setw(2) << (int) guid.bytes_[9]
      << std::setw(2) << (int) guid.bytes_[10]
      << std::setw(2) << (int) guid.bytes_[11]
      << std::setw(2) << (int) guid.bytes_[12]
      << std::setw(2) << (int) guid.bytes_[13]
      << std::setw(2) << (int) guid.bytes_[14]
      << std::setw(2) << (int) guid.bytes_[15];
    s.flags(f);
    return s;
}

static bool char2hex(char input, uint8_t &output) {
    if (input >= '0' && input <= '9') {
        output = input - '0';
        return true;
    }

    if (input >= 'A' && input <= 'F') {
        output = input - 'A' + 10;
        return true;
    }

    if (input >= 'a' && input <= 'f') {
        output = input - 'a' + 10;
        return true;
    }

    return false;
}

std::istream &operator>>(std::istream &in, guid &guid) {
    static const auto bytes_count = guid.bytes_.size();
    int8_t c0, c1;
    uint8_t b0, b1;
    size_t idx = 0;
    while (idx < bytes_count) {
        in >> c1 >> c0;
        bool valid = (bool) in;
        valid &= char2hex(c0, b0) && char2hex(c1, b1);
        if (!valid) {
            guid = guid::invalid();
            break;
        }

        guid.bytes_[idx] = (b1 << 4u) + b0;
        idx++;
    }

    return in;
}

guid guid::from_string(const std::string& str) {
  guid result;
  size_t size = str.size();
  size_t index = 0;
  bool firstDigit = true;

  uint8_t b0, b1;
  for (size_t i = 0; i < size; i++) {
    if (str[i] == '-') continue;
    if (index >= 16)
      return guid::invalid();

    bool valid = true;
    if (firstDigit) {
      valid &= char2hex(str[i], b0);
    } else {
      valid &= char2hex(str[i], b1);
      result.bytes_[index++] = (b0 << 4u) + b1;
    }

    if (!valid)
      return guid::invalid();

    firstDigit = !firstDigit;
  }

  return index < 16 ? guid::invalid() : result;
}

bool operator<(const guid &lhs, const guid &rhs) {
    return lhs.bytes_ < rhs.bytes_;
}

guid guid::invalid() noexcept {
    static guid invalid_guid;
    return invalid_guid;
}

bool guid::is_valid() const {
    return *this != invalid();
}

#ifdef GUID_CFUUID
guid guid::generate() {
    auto newId = CFUUIDCreate(nullptr);
    auto bytes = CFUUIDGetUUIDBytes(newId);
    CFRelease(newId);

    guid guid;
    guid.bytes_ = {
        bytes.byte0,
        bytes.byte1,
        bytes.byte2,
        bytes.byte3,
        bytes.byte4,
        bytes.byte5,
        bytes.byte6,
        bytes.byte7,
        bytes.byte8,
        bytes.byte9,
        bytes.byte10,
        bytes.byte11,
        bytes.byte12,
        bytes.byte13,
        bytes.byte14,
        bytes.byte15
    };
    return guid;
}

#endif

#ifdef GUID_WINDOWS
guid guid::generate() {
    GUID newId;
    CoCreateGuid(&newId);

    guid guid;
    guid.bytes_ = {
        (unsigned char)((newId.Data1 >> 24) & 0xFF),
        (unsigned char)((newId.Data1 >> 16) & 0xFF),
        (unsigned char)((newId.Data1 >> 8) & 0xFF),
        (unsigned char)((newId.Data1) & 0xff),

        (unsigned char)((newId.Data2 >> 8) & 0xFF),
        (unsigned char)((newId.Data2) & 0xff),

        (unsigned char)((newId.Data3 >> 8) & 0xFF),
        (unsigned char)((newId.Data3) & 0xFF),

        (unsigned char)newId.Data4[0],
        (unsigned char)newId.Data4[1],
        (unsigned char)newId.Data4[2],
        (unsigned char)newId.Data4[3],
        (unsigned char)newId.Data4[4],
        (unsigned char)newId.Data4[5],
        (unsigned char)newId.Data4[6],
        (unsigned char)newId.Data4[7]
    };
    return guid;
}
#endif

#ifdef GUID_LIBUUID
guid guid::generate() {
    guid guid;
    static_assert(std::is_same<unsigned char[16], uuid_t>::value, "Wrong type!");
    uuid_generate(guid.bytes_.data());
    return guid;
}
#endif
