#ifdef PLATFORM_INCLUDES
#include <stdio.h>
#endif

#ifdef PLATFORM_OPCODES
OP_PRINT,
#endif

#ifdef PLATFORM_OPNAMES
"PRINT\0"
#endif

#ifdef PLATFORM_CASES
case OP_PRINT:
  printf("%"PRId64"\n", *d--);
  break;
#endif
