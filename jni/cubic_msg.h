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
#define CUBIC_FUNC_DESC_MAX 64

typedef enum CubicMessage {
    //received by OTA
    CUBIC_MSG_OTA_UPGRADEFIRMWARE,
    CUBIC_MSG_TEST_UPDATEFIRMWARE,
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
#define CUBIC_APP_NAME_PWR_SERVICE          "PowerService"

#endif //_CUBIC_MSG_H_
