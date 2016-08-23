/* uScript FreeRTOSConfig overrides.

*/

/* cooperative multitasking is fine. */
#define configUSE_PREEMPTION 0

/* Use the defaults for everything else */
#include_next<FreeRTOSConfig.h>
