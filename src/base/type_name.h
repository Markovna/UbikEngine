#pragma once

#include "macro.h"

#include <string_view>

namespace {

inline constexpr std::string_view extract_type_signature(
    std::string_view signature,
    char prefix,
    char suffix) noexcept {
  auto first = signature.find_first_not_of(' ', signature.find_first_of(prefix) + 1);
  return signature.substr(first, signature.find_last_of(suffix) - first);
}

}

template<typename Type>
[[nodiscard]] constexpr std::string_view type_name() noexcept {
#if defined UBIK_FUNCTION
  return extract_type_signature(UBIK_FUNCTION, UBIK_FUNCTION_PREFIX, UBIK_FUNCTION_SUFFIX);
#else
  return { };
#endif
}