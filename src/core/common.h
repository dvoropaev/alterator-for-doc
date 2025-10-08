#ifndef AB_COMMON_H
#define AB_COMMON_H

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__)
#define AB_SYMBOL_EXPORT __attribute__((__dllexport__))
#define AB_SYMBOL_IMPORT __attribute__((__dllimport__))
#else

#if defined(__LCC__)
#define AB_SYMBOL_EXPORT
#else
#define AB_SYMBOL_EXPORT __attribute__((__visibility__("default")))
#endif

#define AB_SYMBOL_IMPORT
#endif

#endif // AB_COMMON_H
