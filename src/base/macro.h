#ifndef UBIK_CORE_MACRO_H_
#define UBIK_CORE_MACRO_H_

#define UBIK_COMPILER_GCC 0
#define UBIK_COMPILER_CLANG 0
#define UBIK_COMPILER_MSVC 0

#if defined(__GNUC__)
#   undef UBIK_COMPILER_GCC
#   define UBIK_COMPILER_GCC ((__GNUC__ * 10000) + (__GNUC_MINOR__ * 100) + (__GNUC_PATCHLEVEL__))
#elif defined(__clang__)
#   undef  UBIK_COMPILER_CLANG
#   define UBIK_COMPILER_CLANG ((__clang_major__ * 10000) + (__clang_minor__ * 100) + (__clang_patchlevel__))
#elif defined(_MSC_VER)
#   undef  UBIK_COMPILER_MSVC
#   define UBIK_COMPILER_MSVC _MSC_VER
#else
#	error "UBIK_COMPILER_* is undefined!"
#endif

#if UBIK_COMPILER_GCC || UBIK_COMPILER_CLANG
#	define UBIK_FUNCTION __PRETTY_FUNCTION__
#   define UBIK_FUNCTION_PREFIX '='
#   define UBIK_FUNCTION_SUFFIX ']'
#elif UBIK_COMPILER_MSVC
#	define UBIK_FUNCTION __FUNCTION__
#   define UBIK_FUNCTION_PREFIX '<'
#   define UBIK_FUNCTION_SUFFIX '>'
#endif

#define UBIK_CONCAT(__a, __b) UBIK_CONCAT_(__a, __b)
#define UBIK_CONCAT_(__a, __b) __a ## __b

#endif //UBIK_CORE_MACRO_H_
