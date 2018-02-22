/**
 * @file CLogService.h
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Log Service Header, define type and constants
 * @detail Cubic Log Service Header, define type and constants
 */

#ifndef _CLOG_SERVICE_H_
#define _CLOG_SERVICE_H_ 1


#include <sys/time.h>

#define LOG_MAX_SIZE 1024
#define APP_NAME_MAX 16
#define TAG_LEN_MAX 32
#define LOG_SERVICE_ADDR "/var/log-service.sock"

typedef struct LogMessage {
    struct timeval time_stamp;
    int    n_level;                   // leval number
    char   str_appname[APP_NAME_MAX + 4]; // appname
    char   str_tag[TAG_LEN_MAX + 4]; // tag
    char   str_data[LOG_MAX_SIZE + 4];  // payload for massage data
} TLogMessage;

#endif // _CLOG_SERVICE_H_