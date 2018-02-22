/**
* @file cubic_status.h
* @brief status name for all cubic
* @detail status name for all cubic
*/

#ifndef _CUBIC_STATUS_H_
#define _CUBIC_STATUS_H_ 1

#define CUBIC_STAT_location_signal              "location.signal"           // level "0":no service, "1":low, "2":normal "3": good
#define CUBIC_STAT_location_signal_measure      "location.signal_measure"   // signal measure sum, use to judge signal level
#define CUBIC_STAT_location_valid               "location.valid"            // valid of location, 0:not valid, 1: valid
#define CUBIC_STAT_location_stars               "location.stars"            // number of searched stars
#define CUBIC_STAT_location_lat                 "location.latitude"
#define CUBIC_STAT_location_long                "location.longitude"
#define CUBIC_STAT_location_alt                 "location.altitude"
#define CUBIC_STAT_location_vel                 "location.velocity"
#define CUBIC_STAT_location_dir                 "location.direction"        // angle in degree between north

#define CUBIC_STAT_env_temperature              "env.temperature"           // Celsius degree 32.7 ==> "32.7"
#define CUBIC_STAT_env_humidity                 "env.humidity"              // percent 58.5 % ==> "58.5"
#define CUBIC_STAT_env_pressure                 "env.pressure"              // kilo Pascal 100.5 kpa ==> "100.5"
#define CUBIC_STAT_accel_x                      "accel.x"                   // acceleration in direction X, unit gravity
#define CUBIC_STAT_accel_y                      "accel.y"                   // acceleration in direction Y, unit gravity
#define CUBIC_STAT_accel_z                      "accel.z"                   // acceleration in direction Z, unit gravity

#define CUBIC_STAT_bat_level                    "bat.level"                 // level "0":out, "1":low, "2":normal "3": good
#define CUBIC_STAT_bat_vol                      "bat.voltage"               // voltage 3.75 ==> "3.75"
#define CUBIC_STAT_bat_vol_measure              "bat.voltage_measure"       // measure voltage 3.75 ==> "3.75"
#define CUBIC_STAT_bat_percent                  "bat.percent"               // percent of battery
#define CUBIC_STAT_charger_type                 "charger.type"              // "none", "usb", "air"
#define CUBIC_STAT_charger_status               "charger.status"            // "none", "on", "full"

#define CUBIC_STAT_net_connected                "net.connected"             // 0: disconnected, 1 connected
#define CUBIC_STAT_net_wanstat_v4               "net.wanstat.v4"            // qcmap_msgr_wwan_status_enum_v01
#define CUBIC_STAT_net_wanstat_v6               "net.wanstat.v6"            // qcmap_msgr_wwan_status_enum_v01
#define CUBIC_STAT_net_signal                   "net.signal"                // level "0":no service, "1":low, "2":normal "3": good
#define CUBIC_STAT_net_mcc                      "net.mcc"                   //china  --460
#define CUBIC_STAT_net_mnc                      "net.mnc"                   //uincom --01
#define CUBIC_STAT_net_uim_state                "net.uim_state"             // 0 -UIM init completed 1 -UIM fail 2 -UIM no present
#define CUBIC_STAT_net_pin_status               "net.pin_status"            // 0 -PIN is not initialized 1 -PIN is enabled not verified 2 -PIN is enabled, verified
#define CUBIC_STAT_net_verify_retries_left      "net.pin_retires"           // pin retires times 1 2 3
#define CUBIC_STAT_net_unblock_retries_left     "net.puk_retires"           // puk retires times 1 2 3

#define CUBIC_STAT_sip_stat                     "sip.stat"                  // current index of sip status
#define CUBIC_STAT_sip_registered               "sip.registered"            // 0: not registered, 1: registered
#define CUBIC_STAT_sip_reg_code                 "sip.reg_code"              // last code of register
#define CUBIC_STAT_sip_result                   "sip.result"                // last code of dial

#define CUBIC_STAT_bt_status                    "bt.stat"                   // "off", "on", "err"

#define CUBIC_STAT_snd_play                     "snd.play"                  // 0: off, 1:playing
#define CUBIC_STAT_snd_record                   "snd.rec"                   // 0: off, 1:recording
#define CUBIC_STAT_snd_play_path                "snd.path.play"             // "earset", "body", "both"
#define CUBIC_STAT_snd_record_path              "snd.path.rec"              // "earset", "body"
#define CUBIC_STAT_snd_play_vol                 "snd.vol.rec"               // 0~100

#define CUBIC_STAT_vm_uploading                 "vm.upload"                 // number of uploading VM
#define CUBIC_STAT_vm_downloading               "vm.download"               // number of downloading VM
#define CUBIC_STAT_vm_unread                    "vm.unread"                 // number of unread VM
#define CUBIC_STAT_vm_read                      "vm.read"                   // number of read VM

#define CUBIC_STAT_core_stat                    "core.stat"                 // current index of core status
#define CUBIC_STAT_core_group_join_stat         "core.group.join_stat"      // stat of add group  0 wait 1 joining

#define CUBIC_STAT_package_upgrade_stat		"otaservice.package.upgrade_stat"	// stat of download  0 not upgrade 1 upgrading

#endif //_CUBIC_STATUS_H_
