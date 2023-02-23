#pragma once

#include <rwcore.h>

#ifdef __cplusplus
extern "C" {
#endif

// Already defined in rwcore.lib but not exposed in rwcore.h
// The game needs to access this from some functions
extern RwBool _rwFrameSyncDirty(void);

#ifdef __cplusplus
}
#endif