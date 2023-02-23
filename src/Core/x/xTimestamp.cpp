#include "xTimestamp.h"

#define BUILD_NUM 69
#define YEAR 2003

#ifdef _WIN32
#define PLATFORM "Windows"
#else
#error "Unknown platform"
#endif

char timestamp[] = GAME_NAME "\n"
                   "Build " STRINGIFY(BUILD_NUM) "\n"
                   "(c) " STRINGIFY(YEAR) " Heavy Iron Studios\n"
                   "Confidential -- internal use only\n"
                   PLATFORM " build: " __DATE__ ", " __TIME__ "\n";