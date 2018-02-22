/**
 * @file CShareStatusTable.h
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Share Status Table
 * @detail Cubic Share Status Table
 */

#ifndef _CSHARE_STATUS_TABLE_H_
#define _CSHARE_STATUS_TABLE_H_ 1

#include "cubic_status.h"

typedef struct StatusWatchElement {
    const char*   s_name;
    const int    n_value_size;
    const char**   notify_list;
} TStatusWatchElement;


TStatusWatchElement s_watch_table[] = {
    {CUBIC_STAT_location_signal,        4,  NULL},
    {CUBIC_STAT_location_stars,         4,  NULL},
    {CUBIC_STAT_location_valid,         4,  NULL},
    {CUBIC_STAT_location_signal_measure, 8,  NULL},
    {CUBIC_STAT_location_lat,          16,  NULL},
    {CUBIC_STAT_location_long,         16,  NULL},
    {CUBIC_STAT_location_alt,          16,  NULL},
    {CUBIC_STAT_location_vel,          16,  NULL},
    {CUBIC_STAT_location_dir,          16,  NULL},
    {CUBIC_STAT_env_temperature,       16,  NULL},
    {CUBIC_STAT_env_humidity,          16,  NULL},
    {CUBIC_STAT_env_pressure,          16,  NULL},
    {CUBIC_STAT_accel_x,               16,  NULL},
    {CUBIC_STAT_accel_y,               16,  NULL},
    {CUBIC_STAT_accel_z,               16,  NULL},
    {CUBIC_STAT_bat_level,              4,  NULL},
    {CUBIC_STAT_bat_vol,               16,  NULL},
    {CUBIC_STAT_bat_vol_measure,       16,  NULL},
    {CUBIC_STAT_bat_percent,           16,  NULL},
    {CUBIC_STAT_bt_status,              8,  NULL},
    {CUBIC_STAT_snd_play,               4,  NULL},
    {CUBIC_STAT_snd_record,             4,  NULL},
    {CUBIC_STAT_snd_play_path,          8,  NULL},
    {CUBIC_STAT_snd_record_path,        8,  NULL},
    {CUBIC_STAT_snd_play_vol,           8,  NULL},
    {CUBIC_STAT_charger_type,          16,  NULL},
    {CUBIC_STAT_charger_status,        16,  NULL},
    {CUBIC_STAT_net_signal,             4,  NULL},
    {CUBIC_STAT_net_mcc,                4,  NULL},
    {CUBIC_STAT_net_mnc,                4,  NULL},
    {CUBIC_STAT_sip_stat,              16,  NULL},
    {CUBIC_STAT_sip_registered,         4,  NULL},
    {CUBIC_STAT_sip_reg_code,          16,  NULL},
    {CUBIC_STAT_sip_result,            16,  NULL},
    {CUBIC_STAT_net_connected,          4,  NULL},
    {CUBIC_STAT_net_wanstat_v4,         4,  NULL},
    {CUBIC_STAT_net_wanstat_v6,         4,  NULL},
    {CUBIC_STAT_core_stat,              4,  NULL},
    {CUBIC_STAT_core_group_join_stat,   4,  NULL},
    {CUBIC_STAT_net_uim_state,          4,  NULL},
    {CUBIC_STAT_net_pin_status,         4,  NULL},
    {CUBIC_STAT_net_verify_retries_left,  4, NULL},
    {CUBIC_STAT_net_unblock_retries_left, 4, NULL},
    {CUBIC_STAT_vm_uploading,           8,  NULL},
    {CUBIC_STAT_vm_downloading,         8,  NULL},
    {CUBIC_STAT_vm_unread,              8,  NULL},
    {CUBIC_STAT_vm_read,                8,  NULL},
    {CUBIC_STAT_package_upgrade_stat,   4,  NULL},
    {NULL, 0, NULL}
};


#endif //_CSHARE_STATUS_TABLE_H_
