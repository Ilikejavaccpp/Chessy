#ifndef CHESSY_BACKEND_MACROS
#ifndef CHESSY_BACKEND_MACROS_H
#ifndef __MACROS__H

#define __MACROS__H

/* Necessary-ish */
#include <stdarg.h>
#include <sys/cdefs.h>

/* Toggle behaviour via here, all errors and even
 * some weird macros like namespacing can all be toggled here.
 * `//` are commented out, you can uncomment to toggle them on and vice-versa */

#define MACRO__FLAG_USE_EWALL
#define MACRO__FLAG_USE_LOG
#define MACRO__FLAG_USE_TRACE
// #define MACRO__FLAG_USE_EXPERIMENTAL
// #define MACRO__FLAG_USE_ALL
// #define MACRO__FLAG_DISABLE_ALL

/* Don't touch these, unless you want to modify behaviour */
#define _LOG_OUT_ASMF stderr
#define _LTR_OUT_ASMF stderr

/* Automatically enable the other flags too.
 * Do NOT edit this part */
#ifdef MACRO__FLAG_USE_ALL
#define MACRO__FLAG_USE_EWALL
#define MACRO__FLAG_USE_EXPERIMENTAL
#endif

__BEGIN_DECLS

/* Define a struct (obfuscated) or alias it */
#ifndef def_ob_struct
#define _DEF_FWD_STRUCT(STRUCT) typedef struct STRUCT STRUCT;
#define def_ob_struct(ob_struct) typedef struct ob_struct ob_struct;
#define def_fwd_struct(ob_struct) _DEF_FWD_STRUCT(ob_struct)
#endif

/* Define an interface (structure without having to type `struct` everytime) */
#ifndef def_interface
#define def_interface(interface, params)                                       \
  typedef struct {                                                             \
    params                                                                     \
  } interface;
#endif

/* Convert a macro to a string */
#ifndef MACRO_TO_STRING
#define MACRO_TO_STRING(x) #x
#define to_string(non_string) #non_string
#endif

/* Expand a macro (via double string-ify's) */
#ifndef MACRO_EXPAND
#define MACRO_EXPAND(x) MACRO_TO_STRING(x)
#endif

/* Else-If made easier via borrowing the `elif` */
#ifndef elif
#define elif                                                                   \
  __BEGIN_DECLS                                                                \
  else if __END_DECLS
#else

#if defined(MACRO__FLAG_USE_EWALL) || defined(MACRO__FLAG_USE_EXPERIMENTAL)
_Static_assert(
    __builtin_strcmp(MACRO_EXPAND(elif), "else if") == 0 ||
        __builtin_strcmp(MACRO_EXPAND(elif), "extern \"C\" { else if }") == 0,
    "[ERROR] : Macro `elif` should expand to `else if`, not `" MACRO_EXPAND(
        elif) "`\n");

#endif
#endif

#if defined(MACRO__FLAG_USE_EXPERIMENTAL) && !defined(if)
#define if if
#endif

#if defined(MACRO__FLAG_USE_EXPERIMENTAL) && !defined(else)
#define else else
#endif

/* Broken, don't use it */
#ifndef import_std
#define import_std                                                             \
  "stdio.h"                                                                    \
  "stdlib.h"                                                                   \
  "stdbool.h"                                                                  \
  "stdint.h"
#endif

/* Cleaner namespacing defs.. just ^C^V... easy to control with folding. */
#if !defined(sname) && !defined(namespc)
#define sname(defs) defs
#define namespc(stuff) stuff
#endif

/* Need more info on this */
#ifndef sname_str
#define sname_str(str_defs) MACRO_TO_STRING(str_defs)
#endif

/* Cleaner logs (macros/logs.h) */
#ifdef MACRO__FLAG_USE_LOG
/* clang-format disable */

// go-style
#define info_log(fmt, ...)                                                     \
  fprintf(_LOG_OUT_ASMF, "[INFO] : ", fmt "\n",                                \
          ##__VA_ARGS__) // for non-GCC or non-Clang compilers,
                         // also makes it user friendly
#define LOG_MSG(fmt, ...)                                                      \
  fprintf(_LTR_OUT_ASMF, "[INFO] : " fmt,                                      \
          ##__VA_ARGS__) // for non-GCC or non-Clang compilers,
                         // also makes it user friendly

/* clang-format enable */
#endif
#ifdef MACRO__FLAG_USE_TRACE

#define trace_log(fmt, ...)                                                    \
  fprintf(_LOG_OUT_ASMF, "[DEBUG] : " fmt "\n", ##__VA_ARGS__)
#define TRACE_LOG(fmt, ...)                                                    \
  fprintf(_LTR_OUT_ASMF, "[DEBUG] : " fmt, ##__VA_ARGS__)

#endif

/* Instead... please use the c headers instead (`sys/cdefs.h`) */
#ifndef __BEGIN_DECLS
#define ___BEGIN_DECLS extern "C" {
#endif
#ifndef __END_DECLS
#define __END_DECLS
#endif

#define C_INCPP_START __BEGIN_DECLS
#define C_INCPP_END __END_DECLS

__END_DECLS

/* Disable flags (all) if permitted by the config */
#ifdef MACRO__FLAG_DISABLE_ALL
#undef C_INC_INCPP_END
#undef C_INCPP_START
#undef __BEGIN_DECLS /* optional */
#undef __END_DECLS   /* optional */
#undef if
#undef else
#undef elif
#undef to_string
#undef MACRO_TO_STRING
#undef MACRO_EXPAND

#undef sname
#undef sname_str
#undef namespc

#undef import_std /* optional (since it is broken) */

#undef def_fwd_struct
#undef def_ob_struct
#undef def_interface
#endif

#endif
#endif
#endif
