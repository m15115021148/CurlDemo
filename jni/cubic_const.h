/**
 * @file cubic_const.h
 * @brief macro function for all cubic
 * @detail macro function for all cubic
 */

#ifndef _CUBIC_CONST_H
#define _CUBIC_CONST_H 1


#ifndef TRUE
#define TRUE 1
#endif //TRUE

#ifndef FALSE
#define FALSE 0
#endif //FALSE

#ifndef NULL
#define NULL 0
#endif //NULL

#ifndef PATH_MAX
#define PATH_MAX 260
#endif //PATH_MAX

#ifndef CUBIC_APP_NAME_MAX
#define CUBIC_APP_NAME_MAX 64
#endif //CUBIC_APP_NAME_MAX



#define CUBIC_CONFIG_PATH                   "/etc/cfg"
#define CUBIC_MSG_SOCK_DIR                  "/var/"
#define CUBIC_VOICE_MSG_CACHE               "/tmp"
#define CUBIC_OTA_CACHE          			"/cache"
#define CUBIC_BLE_UPLOAD_LOG_PATH           "/tmp/log.log"
#define CUBIC_SIP_VOICE_PIPE_SRC            CUBIC_MSG_SOCK_DIR"sip_voice.src.pipe"
#define CUBIC_SIP_VOICE_PIPE_SINK           CUBIC_MSG_SOCK_DIR"sip_voice.sink.pipe"

#define CUBIC_STUN_SERVERS_MAX              16
#define CUBIC_APN_CUSTMOIZE                 "cust"


#endif //_CUBIC_CONST_H
