#include <stdio.h>
#include <time.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level;

void log_msg(log_level level, const char * msg);    //use only macros to acces this function... to be safe ofcourse....

#define log_debug(msg) log_msg(LOG_DEBUG, msg)
#define log_info(msg) log_msg(LOG_INFO, msg)
#define log_warn(msg) log_msg(LOG_WARN, msg)
#define log_error(msg) log_msg(LOG_ERROR, msg)
#define log_fatal(msg) log_msg(LOG_FATAL, msg)
