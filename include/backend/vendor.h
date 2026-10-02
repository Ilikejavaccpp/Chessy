/*
 * Basically, the entire `Vendor` namespace from `utils.h` but for C
 * and the entire backend folder.
 *
 * */

#if !defined(CHESSY_VENDOR_H) && !defined(CHESSY_VENDOR_HEADER) &&             \
    !defined(CHESSY_BACKEND_VENDOR_H)
#define CHESSY_VENDOR_H 1
#define CHESSY_VENDOR_HEADER 1
#define CHESSY_BACKEND_VENDOR_H 1

// header includes
#include <string.h>
// end

#define __C_CXX_HEADER__STRING_H 1

/* Ypkg vendor utility functions */
#if __C_CXX_HEADER__STDDEF_H == 0
typedef unsigned long size_t;
#endif

#if __C_CXX_HEADER__STDBOOL_H == 0
#ifndef __cplusplus
#if !defined(CHESSY_BACKEND_CORE_FLAG__USE_BOOL) &&                            \
    !defined(__FLAG__USE_STDBOOL_H)
typedef _Bool bool;
#define true 1
#define false 0
#endif
#endif
#endif

/* for safety */
#ifdef NULL
#undef NULL
#endif

/* :clang-format disable */
#ifndef NULL
#ifdef __cplusplus
#if __cplusplus >= 201103L
#define NULL nullptr
#else
#define NULL 0
#endif
#else
#define NULL ((void *)0)
#endif
#endif

// pass `ret_index` as `nullptr_t` or `NULL` if you want to use the default
// that is false
static inline int findOption(char **list, int listSize,
                             const char *targetOption, bool *ret_index) {
  // If the user didn't pass a valid pointer to track behavior, default to
  // matching indexing behavior
  bool use_index_return = (ret_index != NULL) ? *ret_index : true;
  size_t target_len = strlen(targetOption);

  for (int i = 1; i < listSize; ++i) {
    if (list[i] == NULL)
      continue;

    char *arg = list[i];

    // 1. Matches exact flag (e.g., "--cli")
    if (strcmp(arg, targetOption) == 0) {
      return use_index_return ? i : 0;
    }

    // 2. Matches flag with an embedded '=' (e.g., "--mode=cli")
    if (strncmp(arg, targetOption, target_len) == 0 && arg[target_len] == '=') {
      return use_index_return ? i : 0;
    }
  }

  return -1; // Option not found
}

#endif

/* Re def check */

/* stddef.h */
#if defined(_STDDEF_H) || defined(__STDDEF_H__)
#define __C_CXX_HEADER__STDDEF_H 1
#else
#define __C_CXX_HEADER__STDDEF_H 0
#endif

/* string.h */
#if defined(_STRING_H) || defined(__STRING_H__)
#define __C_CXX_HEADER__STRING_H 1
#else
#define __C_CXX_HEADER__STRING_H 0
#endif
