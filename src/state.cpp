#include "state.h"

// Instantiate the actual struct in memory (zero-initialized)
VCS_State_t globalState = {0};

// Instantiate and unlock the FreeRTOS spinlock
portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;