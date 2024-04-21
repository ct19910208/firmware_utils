#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_PRINTF

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
} log_level_t;

void set_log_level(log_level_t level);
log_level_t get_log_level(void);

#define LOG_FORMAT(head, fmt)           #head " %s %s(line %d): " fmt
#define LOG_DEBUG(tag, fmt, ...) \
    if (get_log_level() >= LOG_LEVELDEBUG) {\
        char* ptr = strrchr(__FILE__, '/'); \
        if (ptr == NULL) ptr = __FILE__; else ptr++; \
        LOG_PRINTF(LOG_FORMAT([NPU-LOGW], fmt), tag, ptr, __LINE__, ##__VA_ARGS__); \
    }

#define LOG_INFO(tag, fmt, ...) \
    if (get_log_level() >= LOG_LEVELINFO) {\
        char* ptr = strrchr(__FILE__, '/'); \
        if (ptr == NULL) ptr = __FILE__; else ptr++; \
        LOG_PRINTF(LOG_FORMAT([NPU-LOGW], fmt), tag, ptr, __LINE__, ##__VA_ARGS__); \
    }

#define LOG_WARNING(tag, fmt, ...) \
    if (get_log_level() >= LOG_LEVELWARNING) {\
        char* ptr = strrchr(__FILE__, '/'); \
        if (ptr == NULL) ptr = __FILE__; else ptr++; \
        LOG_PRINTF(LOG_FORMAT([NPU-LOGW], fmt), tag, ptr, __LINE__, ##__VA_ARGS__); \
    }

#define LOG_ERROR(tag, fmt, ...) \
    if (get_log_level() >= LOG_LEVELERROR) {\
        char* ptr = strrchr(__FILE__, '/'); \
        if (ptr == NULL) ptr = __FILE__; else ptr++; \
        LOG_PRINTF(LOG_FORMAT([NPU-LOGW], fmt), tag, ptr, __LINE__, ##__VA_ARGS__); \
    }


#ifdef __cplusplus
}
#endif
#endif
