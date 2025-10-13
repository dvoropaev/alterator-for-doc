#ifndef AB_CORE_H
#define AB_CORE_H

#include "common.h"

#ifdef AB_CORE_LIBRARY
#define AB_CORE_EXPORT AB_SYMBOL_EXPORT
#else
#define AB_CORE_EXPORT AB_SYMBOL_IMPORT
#endif

#endif // AB_CORE_H
