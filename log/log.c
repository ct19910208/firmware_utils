#include "log.h"

static log_level_t log_level = LOG_LEVEL_WARNING;

void set_log_level(log_level_t level) 
{
    log_level = level;
    LOG_WARNING("[LOG]", "Set log level: %d\n", log_level);
}
log_level_t get_log_level(void)
{
    return log_level;
}