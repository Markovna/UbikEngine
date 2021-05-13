#ifndef SPRINTENGINE_CORE_MACRO_H_
#define SPRINTENGINE_CORE_MACRO_H_

#define SPRINT_COMPILER_GCC 0
#define SPRINT_COMPILER_CLANG 0
#define SPRINT_COMPILER_MSVC 0

#if defined(__GNUC__)
#   undef SPRINT_COMPILER_GCC
#   define SPRINT_COMPILER_GCC ((__GNUC__ * 10000) + (__GNUC_MINOR__ * 100) + (__GNUC_PATCHLEVEL__))
#elif defined(__clang__)
#   undef  SPRINT_COMPILER_CLANG
#   define SPRINT_COMPILER_CLANG ((__clang_major__ * 10000) + (__clang_minor__ * 100) + (__clang_patchlevel__))
#elif defined(_MSC_VER)
#   undef  SPRINT_COMPILER_MSVC
#   define SPRINT_COMPILER_MSVC _MSC_VER
#else
#	error "SPRINT_COMPILER_* is undefined!"
#endif

#if SPRINT_COMPILER_GCC || SPRINT_COMPILER_CLANG
#	define SPRINT_FUNCTION __PRETTY_FUNCTION__
#elif SPRINT_COMPILER_MSVC
#	define SPRINT_FUNCTION __FUNCTION__
#endif

#define SPRINT_CONCAT(__a, __b) SPRINT_CONCAT_(__a, __b)
#define SPRINT_CONCAT_(__a, __b) __a ## __b

#endif //SPRINTENGINE_CORE_MACRO_H_
