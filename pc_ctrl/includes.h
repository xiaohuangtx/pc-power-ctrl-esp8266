#ifndef __INCLUDES_H__
#define __INCLUDES_H__

// #define DEBUG_LOG

#ifdef DEBUG_LOG
#define LOG_PRINTF(...) Serial.printf(__VA_ARGS__)
#define LOG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define LOG_PRINT(...) Serial.print(__VA_ARGS__)
#else
#define LOG_PRINTF(...)
#define LOG_PRINTLN(...)
#define LOG_PRINT(...)
#endif

#endif
