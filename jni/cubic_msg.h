/**
 * @file cubic_msg.h
 * @brief status name for all cubic
 * @detail status name for all cubic
 */

#ifndef _CUBIC_MSG_H_
#define _CUBIC_MSG_H_ 1


#define CUBIC_UUID_LEN_MAX 64
#define CUBIC_URL_LEN_MAX 256
#define CUBIC_TIME_LEN_MAX 32
#define CUBIC_PIN_PUK_LEN_MAX 16
#define CUBIC_BLE_PARAM_LEN_MAX 1024
#define CUBIC_VERNO_LEN_MAX 128
#define CUBIC_INFO_LEN_MAX 256
#define CUBIC_PATH_MAX 256

typedef struct cubic_msg_gps_fence_evt {
    int index; // index of fence config
    int is_in; // 0:out, 1:in
} cubic_msg_gps_fence_evt;

typedef struct cubic_msg_vm_received {
    char group_uuid[CUBIC_UUID_LEN_MAX];
    char sender_uuid[CUBIC_UUID_LEN_MAX];
    char created[CUBIC_TIME_LEN_MAX];
    char voice_message_url[CUBIC_URL_LEN_MAX];
} cubic_msg_vm_received;

typedef struct cubic_msg_test_upload_vm {
    char file_path[CUBIC_URL_LEN_MAX];
} cubic_msg_test_upload_vm;

typedef struct cubic_msg_test_ble_test {
    char param[CUBIC_BLE_PARAM_LEN_MAX];
} cubic_msg_test_ble_test;

typedef struct cubic_msg_light_test_light {
    int light;
} cubic_msg_light_test_light;

typedef struct cubic_msg_update_firmware {
    char version[CUBIC_VERNO_LEN_MAX];
    char date[CUBIC_TIME_LEN_MAX];
    char info[CUBIC_INFO_LEN_MAX];
} cubic_msg_update_firmware;

typedef struct cubic_msg_join_group {
    char group_uuid[CUBIC_UUID_LEN_MAX];
    char token[CUBIC_UUID_LEN_MAX];
} cubic_msg_join_group;

typedef cubic_msg_test_upload_vm cubic_msg_test_download_vm;

typedef cubic_msg_test_upload_vm cubic_msg_test_play_sound;

typedef cubic_msg_test_upload_vm cubic_msg_test_record_sound;

typedef cubic_msg_test_ble_test cubic_msg_test_ble;

typedef cubic_msg_update_firmware cubic_msg_update;

typedef struct cubic_msg_sip_call_dial {
    char peer_uuid[CUBIC_UUID_LEN_MAX];
} cubic_msg_sip_call_dial;

typedef struct cubic_msg_net_pin_verify {
    char pin_puk_code[CUBIC_PIN_PUK_LEN_MAX] ;
} cubic_msg_net_pin_verify;

typedef cubic_msg_net_pin_verify cubic_msg_net_enable_pin;

typedef struct cubic_msg_test_dial {
    char number[CUBIC_UUID_LEN_MAX] ;
} cubic_msg_test_dial;

typedef struct cubic_msg_sound_play_start {
    char path[CUBIC_PATH_MAX];
    int  loop;
} cubic_msg_sound_play_start;

typedef struct cubic_msg_sound_record_start {
    char path[CUBIC_PATH_MAX];
} cubic_msg_sound_record_start;

typedef struct cubic_msg_play_done {
    char path[CUBIC_PATH_MAX];
    int  complete;
    int  error;
} cubic_msg_play_done;

typedef cubic_msg_play_done cubic_msg_record_done;

typedef struct cubic_msg_last_unread_vm {
    char path[CUBIC_PATH_MAX];
} cubic_msg_last_unread_vm;

typedef cubic_msg_last_unread_vm cubic_msg_leave_message_vm;
typedef cubic_msg_last_unread_vm cubic_msg_lm;
typedef cubic_msg_last_unread_vm cubic_msg_vm_send;

typedef struct cubic_msg_vm_read {
    int  error;
    char path[CUBIC_PATH_MAX];
} cubic_msg_vm_read;

typedef struct cubic_msg_vm_get_lm {
    int index;
} cubic_msg_vm_get_lm;


typedef enum CubicMessage {
    // received by core
    CUBIC_MSG_SIM_READY,
    CUBIC_MSG_SIM_ERROR,
    CUBIC_MSG_NETWORK_CONNECTED,
    CUBIC_MSG_NETWORK_LOST,
    CUBIC_MSG_REGISTER_SUCCESS,
    CUBIC_MSG_REGISTER_LOST,
    CUBIC_MSG_GPS_RENEW,
    CUBIC_MSG_GPS_FENCE_EVT, //struct argument
    CUBIC_MSG_GPS_LOST,
    CUBIC_MSG_BATTERY_LOW,
    CUBIC_MSG_BATTERY_OUT,
    CUBIC_MSG_BATTERY_NORMAL,
    CUBIC_MSG_BT_ON,
    CUBIC_MSG_BT_OFF,
    CUBIC_MSG_CHARGER_IN,
    CUBIC_MSG_CHARGER_OUT,
    CUBIC_MSG_CHARGER_FULL,
    CUBIC_MSG_CALL_RECEIVED,
    CUBIC_MSG_CALL_ACCEPTED,
    CUBIC_MSG_CALL_HANGUPED,
    CUBIC_MSG_KEEP_TRACKING,
    CUBIC_MSG_JOIN_GROUP, //struct argument
    CUBIC_MSG_NEW_FIRMWARE_NOTIFY,
    CUBIC_MSG_FLASH_NEW_FIRMWARE_DOWNLOAD_LIGHT,
    CUBIC_MSG_FLASH_OPERATION_FAIL_LIGHT,
    CUBIC_MSG_FLASH_BT_LIGHT,
    CUBIC_MSG_PLAY_DONE,   //struct argument
    CUBIC_MSG_RECORD_DONE, //struct argument

    CUBIC_MSG_LAST_UNREAD_VM,   //struct argument
    CUBIC_MSG_LM,               //struct argument

    CUBIC_MSG_EVT_SPK_IN,
    CUBIC_MSG_EVT_SPK_OUT,
    CUBIC_MSG_EVT_MIC_IN,
    CUBIC_MSG_EVT_MIC_OUT,

    CUBIC_MSG_EVT_KEY_FIRST,
    CUBIC_MSG_EVT_KEY_PRESS_MAIN = CUBIC_MSG_EVT_KEY_FIRST,
    CUBIC_MSG_EVT_KEY_PRESS_MAIN_WITH_SUB1,
    CUBIC_MSG_EVT_KEY_PRESS_MAIN_WITH_SUB2,
    CUBIC_MSG_EVT_KEY_PRESS_MAIN_WITH_SUB3,
    CUBIC_MSG_EVT_KEY_LONGPRESS_MAIN,
    CUBIC_MSG_EVT_KEY_LONGPRESS_MAIN_WITH_POWR,
    CUBIC_MSG_EVT_KEY_LONGPRESS_ALL_SUB_KEY,
    CUBIC_MSG_EVT_KEY_CLICK_MAIN,
    CUBIC_MSG_EVT_KEY_DOUBLE_CLICK_MAIN,
    CUBIC_MSG_EVT_KEY_TRIPLE_CLICK_MAIN,
    CUBIC_MSG_EVT_KEY_RELEASE_MAIN,
    CUBIC_MSG_EVT_KEY_PRESS_PTT,
    CUBIC_MSG_EVT_KEY_LONGPRESS_PTT,
    CUBIC_MSG_EVT_KEY_CLICK_PTT,
    CUBIC_MSG_EVT_KEY_RELEASE_PTT,
    CUBIC_MSG_EVT_KEY_LONGPRESS_SUB1,
    CUBIC_MSG_EVT_KEY_CLICK_SUB1,
    CUBIC_MSG_EVT_KEY_RELEASE_SUB1,
    CUBIC_MSG_EVT_KEY_LONGPRESS_SUB2,
    CUBIC_MSG_EVT_KEY_CLICK_SUB2,
    CUBIC_MSG_EVT_KEY_RELEASE_SUB2,
    CUBIC_MSG_EVT_KEY_LONGPRESS_SUB3,
    CUBIC_MSG_EVT_KEY_CLICK_SUB3,
    CUBIC_MSG_EVT_KEY_RELEASE_SUB3,
    CUBIC_MSG_EVT_KEY_LONGPRESS_POWR,
    CUBIC_MSG_EVT_KEY_END,

    CUBIC_MSG_TEST_UPLOAD_VM, //struct argument
    CUBIC_MSG_TEST_DOWNLOAD_VM, //struct argument
    CUBIC_MSG_TEST_POWER_OFF,
    CUBIC_MSG_TEST_RESET_FACTORY,
    CUBIC_MSG_TEST_PLAY_SOUND,
    CUBIC_MSG_TEST_RECORD_SOUND,
    CUBIC_MSG_TEST_HELP,
    CUBIC_MSG_TEST_ENV,
    CUBIC_MSG_TEST_BAT,
    CUBIC_MSG_TEST_SHOCK,
    CUBIC_MSG_TEST_TRACK,
    CUBIC_MSG_TEST_DIAL, //struct argument
    CUBIC_MSG_TEST_POLL,
    CUBIC_MSG_TEST_MIC,


    // received by sip
    CUBIC_MSG_SIP_REGISTER,
    CUBIC_MSG_SIP_CALL_DIAL, //struct argument
    CUBIC_MSG_SIP_CALL_ACCEPT,
    CUBIC_MSG_SIP_CALL_HANGUP,
    CUBIC_MSG_SIP_SET_CFG,

    // received by network
    CUBIC_MSG_NET_CONNECT,
    CUBIC_MSG_NET_DISCONNECT,
    CUBIC_MSG_NET_SET_APN,
    CUBIC_MSG_NET_PIN_VERIFY, //struct argument
    CUBIC_MSG_NET_ENABLE_PIN, //struct argument

    // received by GPS
    CUBIC_MSG_GPS_SET_GEOFENCE,
    CUBIC_MSG_GPS_ENABLE,
    CUBIC_MSG_GPS_DISABLE,

    // received by BAT

    // received be BLE
    CUBIC_MSG_BLE_ENABLE,
    CUBIC_MSG_BLE_DISABLE,
    CUBIC_MSG_BLE_CHECK,
    CUBIC_MSG_BLE_TEST,
    CUBIC_MSG_BLE_UPLOAD_LOG,

    //received by OTA
    CUBIC_MSG_OTA_UPGRADEFIRMWARE,
    CUBIC_MSG_TEST_UPDATEFIRMWARE,

    //reveived by Event
    CUBIC_MSG_SET_THRESHOLDS,

    //reveived by Light
    CUBIC_MSG_LIGHT_UPDATE,
    CUBIC_MSG_LIGHT_SLEEP_FLASH,
    CUBIC_MSG_LIGHT_DO_VIBRATE,

    CUBIC_MSG_LIGHT_STAT_RESTORING_ON,
    CUBIC_MSG_LIGHT_STAT_RESTORING_OFF,
    CUBIC_MSG_LIGHT_STAT_BOOTING_ON,
    CUBIC_MSG_LIGHT_STAT_BOOTING_OFF,
    CUBIC_MSG_LIGHT_STAT_USR_CHK_BAT_ON,
    CUBIC_MSG_LIGHT_STAT_USR_CHK_BAT_OFF,
    CUBIC_MSG_LIGHT_STAT_USR_CHK_NET_ON,
    CUBIC_MSG_LIGHT_STAT_USR_CHK_NET_OFF,
    CUBIC_MSG_LIGHT_STAT_USR_CHK_GPS_ON,
    CUBIC_MSG_LIGHT_STAT_USR_CHK_GPS_OFF,
    CUBIC_MSG_LIGHT_STAT_SOS_ON,
    CUBIC_MSG_LIGHT_STAT_SOS_OFF,
    CUBIC_MSG_LIGHT_STAT_CALL_MUTE_ON,
    CUBIC_MSG_LIGHT_STAT_CALL_MUTE_OFF,
    CUBIC_MSG_LIGHT_STAT_CALL_CONVERSATION_ON,
    CUBIC_MSG_LIGHT_STAT_CALL_CONVERSATION_OFF,
    CUBIC_MSG_LIGHT_STAT_CALL_UNCONNECT_ON,
    CUBIC_MSG_LIGHT_STAT_CALL_UNCONNECT_OFF,
    CUBIC_MSG_LIGHT_STAT_MESSAGE_ON,
    CUBIC_MSG_LIGHT_STAT_MESSAGE_OFF,
    CUBIC_MSG_LIGHT_STAT_LEAVE_MESSAGE_ON,
    CUBIC_MSG_LIGHT_STAT_LEAVE_MESSAGE_OFF,
    CUBIC_MSG_LIGHT_STAT_BT_ON,
    CUBIC_MSG_LIGHT_STAT_BT_OFF,
    CUBIC_MSG_LIGHT_STAT_ERROR_SIM_ON,
    CUBIC_MSG_LIGHT_STAT_ERROR_SIM_OFF,
    CUBIC_MSG_LIGHT_STAT_ERROR_NET_ON,
    CUBIC_MSG_LIGHT_STAT_ERROR_NET_OFF,
    CUBIC_MSG_LIGHT_STAT_ERROR_ACTIVATE_ON,
    CUBIC_MSG_LIGHT_STAT_ERROR_ACTIVATE_OFF,
    CUBIC_MSG_LIGHT_STAT_ERROR_SIP_ON,
    CUBIC_MSG_LIGHT_STAT_ERROR_SIP_OFF,
    CUBIC_MSG_LIGHT_STAT_UPDATE_READY_ON,
    CUBIC_MSG_LIGHT_STAT_UPDATE_READY_OFF,
    CUBIC_MSG_LIGHT_STAT_UPDATING_ON,
    CUBIC_MSG_LIGHT_STAT_UPDATING_OFF,
    CUBIC_MSG_LIGHT_STAT_ACTIVE_ON,
    CUBIC_MSG_LIGHT_STAT_ACTIVE_OFF,
    CUBIC_MSG_LIGHT_STAT_PASSIVE_ON,
    CUBIC_MSG_LIGHT_STAT_PASSIVE_OFF,
    CUBIC_MSG_LIGHT_STAT_DOWNLOAD_FINISH_ON,
    CUBIC_MSG_LIGHT_STAT_DOWNLOAD_FINISH_OFF,
    CUBIC_MSG_LIGHT_STAT_ERROR_OTA_ON,
    CUBIC_MSG_LIGHT_STAT_ERROR_OTA_OFF,
    CUBIC_MSG_LIGHT_TEST_LIGHT, //struct argument

    //reveived by Sound
    CUBIC_MSG_SOUND_PLAY_START, //struct argument
    CUBIC_MSG_SOUND_PLAY_STOP,
    CUBIC_MSG_SOUND_RECORD_START, //struct argument
    CUBIC_MSG_SOUND_RECORD_STOP,
    CUBIC_MSG_SOUND_SIP_START,
    CUBIC_MSG_SOUND_SIP_STOP,
    CUBIC_MSG_SOUND_ROUTE_PLAY_SPEAKER,
    CUBIC_MSG_SOUND_ROUTE_PLAY_EARPHONE,
    CUBIC_MSG_SOUND_ROUTE_PLAY_BOTH,
    CUBIC_MSG_SOUND_ROUTE_REC_MIC,
    CUBIC_MSG_SOUND_ROUTE_REC_EARPHONE,
    CUBIC_MSG_SOUND_VOL_UP,
    CUBIC_MSG_SOUND_VOL_DOWN,
    CUBIC_MSG_SOUND_VOL_MUTE_OR_RESUME,

    //received by VM
    CUBIC_MSG_VM_RECEIVED, //struct argument
    CUBIC_MSG_VM_POLL,
    CUBIC_MSG_VM_READ,     //struct argument
    CUBIC_MSG_VM_GET_LM,   //struct argument
    CUBIC_MSG_VM_SEND,     //struct argument


    // keep this at last possition
    CUBIC_MSG_TOTAL,
    CUBIC_MSG_APP_PRIVATE = CUBIC_MSG_TOTAL + 1,
} CubicMessage;

#define CUBIC_APP_NAME_CORE                 "CoreApp"
#define CUBIC_APP_NAME_SIP_SERVICE          "SipService"
#define CUBIC_APP_NAME_NET_SERVICE          "NetworkService"
#define CUBIC_APP_NAME_GPS_SERVICE          "ULocationService"
#define CUBIC_APP_NAME_BAT_SERVICE          "BatService"
#define CUBIC_APP_NAME_OTA_SERVICE          "OTAService"
#define CUBIC_APP_NAME_BLE_INTERFACE        "BleInterface"
#define CUBIC_APP_NAME_LOG_SERVICE          "LogService"
#define CUBIC_APP_NAME_EVT_SERVICE          "EventService"
#define CUBIC_APP_NAME_LIT_SERVICE          "LightService"
#define CUBIC_APP_NAME_SND_SERVICE          "SoundService"
#define CUBIC_APP_NAME_VM_SERVICE           "VMService"


#endif //_CUBIC_MSG_H_
