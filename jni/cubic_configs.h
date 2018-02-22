/**
* @file cubic_configs.h
* @brief configus name for all cubic
* @detail configus name for all cubic
*/

#ifndef _CUBIC_CONFIGS_H_
#define _CUBIC_CONFIGS_H_ 1

#define CUBIC_CFG_serial_num                        "ro.core.serial_num"       // 17 characters, Ex: 11111222333333w, setup by NetworkService
#define CUBIC_CFG_version_num                       "ro.core.version_num"      // version number string setup by build script
#define CUBIC_CFG_bt_addr                           "ro.core.bt_addr"          // 12 characters of MAC address hex string, setup by NetworkService

#define CUBIC_CFG_sleep_mode                        "rw.core.sleep_mode"       // sleep mode, "none" never sleep, "standby" sleep when idle status, "sleepy" sleep if no communication
#define CUBIC_CFG_sleep_timeout                     "rw.core.sleep_timeout"   // 0~65535 unit seconds
#define CUBIC_CFG_active_timeout                    "rw.core.active_timeout"   // 0~65535 unit seconds
#define CUBIC_CFG_preset_number                     "rw.core.quick_dial.%d.no" // max 64 characters, uuid of target client
#define CUBIC_CFG_report_interval                   "rw.core.report.interval"  // report interval of active mode
#define CUBIC_CFG_report_p_interval                 "rw.core.report.p_interval" // report interval of passive mode
#define CUBIC_CFG_light_polling_period              "rw.core.polling_period" // particle len of light effect, ms
#define CUBIC_CFG_light_particle_len                "ro.core.light_particle_len" // particle len of light effect, us

#define CUBIC_CFG_key_press_time_hold               "ro.core.key_press_time.hold"  // in ms
#define CUBIC_CFG_key_press_time_long               "ro.core.key_press_time.long"  // in ms

#define CUBIC_CFG_watch_timeout                     "ro.watch.timeout"
#define CUBIC_CFG_watch_init                        "ro.watch.init"
#define CUBIC_CFG_watch_init_name                   CUBIC_CFG_watch_init".%s.name"    // max 32 charectors to name task
#define CUBIC_CFG_watch_init_command                CUBIC_CFG_watch_init".%s.command" // command to start process
#define CUBIC_CFG_watch_init_type                   CUBIC_CFG_watch_init".%s.type"    // only "once", "init", "eternal" is allowed

#define CUBIC_CFG_log_level_limit                   "rw.log.level_limit"       // limit for log level
#define CUBIC_CFG_log_file_path                     "ro.log.file_path"         // limit for log file path
#define CUBIC_CFG_log_net_addr                      "ro.log.net_addr"          // limit for log net, empty to disable, other will send log out
#define CUBIC_CFG_log_nmea                          "rw.log.nmea"              // path to save raw nmea log, empty to disable

#define CUBIC_CFG_push_server                       "rw.core.push.server_url"  // url of push server, max 260 characters
#define CUBIC_CFG_push_group                        "rw.core.push.group"       // uuid of group
#define CUBIC_CFG_push_uname                        "rw.core.push.uname"       // uuid for ourself, use as client_id or user name when communicate with push server
#define CUBIC_CFG_push_upswd                        "rw.core.push.upswd"       // passworkd for some push message
#define CUBIC_CFG_push_group_invite_token           "rw.core.push.invite.token"			// token info of group invitation
#define CUBIC_CFG_push_group_invite_gid             "rw.core.push.invite.gid"  			// gourp id of group invitation
#define CUBIC_CFG_push_group_invite_retry_cnt       "rw.core.push.invite.retry_count"   // retried count of join

#define CUBIC_CFG_sip_domain                        "rw.sip.domain"                 // sip server domain name
#define CUBIC_CFG_sip_proxy                         "rw.sip.proxy"                  // sip proxy server address
#define CUBIC_CFG_sip_protocol                      "rw.sip.protocol"               // sip server communicate protocol, can be "TCP" "UDP" "TLS" "SCTP"
#define CUBIC_CFG_sip_registrar                     "rw.sip.registrar"              // sip registrar server address
#define CUBIC_CFG_sip_uname                         "rw.sip.uname"                  // uuid as user name for sip server
#define CUBIC_CFG_sip_upswd                         "rw.sip.upswd"                  // password for sip server
#define CUBIC_CFG_sip_mport_start                   "rw.sip.mport.start"            // 0-65535, min port number of SIP multimedia transfer, Ex.: voice sound, video
#define CUBIC_CFG_sip_mport_end                     "rw.sip.mport.end"              // 0-65535, max port number of SIP multimedia transfer, Ex.: voice sound, video
#define CUBIC_CFG_sip_sport                         "rw.sip.sport"                  // 0-65535, port number of SIP control session, 0 means random choose by pjsip
#define CUBIC_CFG_sip_stun                          "rw.sip.stun"                   // STUN configs list
#define CUBIC_CFG_sip_defailt_stun_addr             CUBIC_CFG_sip_stun".default.addr"// default STUN server address
#define CUBIC_CFG_sip_stun_addr                     CUBIC_CFG_sip_stun".%s.addr"	// STUN server address


#define CUBIC_CFG_loc_fence                    		"rw.location.fence"
#define CUBIC_CFG_loc_fence_name                    CUBIC_CFG_loc_fence".%d.name"
#define CUBIC_CFG_loc_fence_lat                     CUBIC_CFG_loc_fence".%d.latitude"
#define CUBIC_CFG_loc_fence_long                    CUBIC_CFG_loc_fence".%d.longitude"
#define CUBIC_CFG_loc_fence_rad                     CUBIC_CFG_loc_fence".%d.radius"
#define CUBIC_CFG_loc_fence_num						CUBIC_CFG_loc_fence".number"

#define CUBIC_CFG_env_threshold_shock_x             "rw.env.threshold.shock.x"
#define CUBIC_CFG_env_threshold_shock_y             "rw.env.threshold.shock.y"
#define CUBIC_CFG_env_threshold_shock_z             "rw.env.threshold.shock.z"
#define CUBIC_CFG_env_threshold_temperature_low     "rw.env.threshold.temperature.low"
#define CUBIC_CFG_env_threshold_temperature_high    "rw.env.threshold.temperature.high"
#define CUBIC_CFG_env_threshold_humidity_low        "rw.env.threshold.humidity.low"
#define CUBIC_CFG_env_threshold_humidity_high       "rw.env.threshold.humidity.high"
#define CUBIC_CFG_env_threshold_pressure_low        "rw.env.threshold.pressure.low"
#define CUBIC_CFG_env_threshold_pressure_high       "rw.env.threshold.pressure.high"

#define CUBIC_CFG_net_apn                           "rw.net.apn"
#define CUBIC_CFG_net_apn_profile_name              CUBIC_CFG_net_apn".%s.profile_name"      // char user-defined name for the profile Lenlimited 50
#define CUBIC_CFG_net_apn_mcc                       CUBIC_CFG_net_apn".%s.mcc"               // use mcc match apn
#define CUBIC_CFG_net_apn_mnc                       CUBIC_CFG_net_apn".%s.mnc"               // use mnc match apn
#define CUBIC_CFG_net_apn_apn_name                  CUBIC_CFG_net_apn".%s.apn_name"          // char used to select the GGSN and external packet data network Lenlimited 150
#define CUBIC_CFG_net_apn_auth_type                 CUBIC_CFG_net_apn".%s.auth_type"         // 0 NONE 1 PAP 2 CHAP 3 PAP or CHAP
#define CUBIC_CFG_net_apn_user_name                 CUBIC_CFG_net_apn".%s.user_name"         //char Lenlimited 127 sername used during data network authentication
#define CUBIC_CFG_net_apn_password                  CUBIC_CFG_net_apn".%s.password"          //char Lenlimited 127 Password to be used during data network authentication
#define CUBIC_CFG_net_apn_protocol                  CUBIC_CFG_net_apn".%s.protocol"          //The protocol. Must be either "IPV4"  or "IPV6" or "IPV4_IPV6"
#define CUBIC_CFG_net_apn_ipv4_primary_dns          CUBIC_CFG_net_apn".%s.ipv4_primary_dns"  //uint32 Used as a preference during negotiation with the network
#define CUBIC_CFG_net_apn_ipv4_secondary_dns        CUBIC_CFG_net_apn".%s.ipv4_secondary_dns"//bakeup ipv4 primary
#define CUBIC_CFG_net_apn_ipv6_primary_dns          CUBIC_CFG_net_apn".%s.ipv6_primary_dns"  //uint8 Used as a preference during negotiation with the network
#define CUBIC_CFG_net_apn_ipv6_secondary_dns        CUBIC_CFG_net_apn".%s.ipv6_secondary_dns"//bakeup ipv6 primary

#define CUBIC_CFG_ble_password                      "rw.ble.password"
#define CUBIC_CFG_ble_active_timeout                "ro.ble.active_timeout"

#define CUBIC_CFG_update_jsonpath                   "rw.update.jsonpath"


#endif //_CUBIC_CONFIGS_H_

