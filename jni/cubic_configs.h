/**
* @file cubic_configs.h
* @brief configus name for all cubic
* @detail configus name for all cubic
*/

#ifndef _CUBIC_CONFIGS_H_
#define _CUBIC_CONFIGS_H_ 1

#define CUBIC_CFG_serial_num                        "ro.core.serial_num"       // 17 characters, Ex: 11111222333333w, setup by NetworkService
#define CUBIC_CFG_version_num                       "ro.core.version_num"      // version number string setup by build script


#define CUBIC_CFG_push_server                       "rw.core.push.server_url"  // url of push server, max 260 characters
#define CUBIC_CFG_push_group                        "rw.core.push.group"       // uuid of group
#define CUBIC_CFG_push_uname                        "rw.core.push.uname"       // uuid for ourself, use as client_id or user name when communicate with push server
#define CUBIC_CFG_push_upswd                        "rw.core.push.upswd"       // passworkd for some push message

#define CUBIC_CFG_log_level_limit                   "rw.log.level_limit"       // limit for log level

#endif //_CUBIC_CONFIGS_H_

