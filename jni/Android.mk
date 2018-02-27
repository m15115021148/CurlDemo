LOCAL_PATH := $(call my-dir)

#openssl
include $(CLEAR_VARS)  
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE    := libcrypto  
LOCAL_SRC_FILES := libs/openssl/lib/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY) 

include $(CLEAR_VARS)  
LOCAL_MODULE    := libssl  
LOCAL_SRC_FILES := libs/openssl/lib/libssl.a  
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libcrypto
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY) 

#curl openssl 
include $(CLEAR_VARS)  
LOCAL_MODULE    := libcurl  
LOCAL_SRC_FILES := libs/curl/lib/libcurl.a  
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY) 

#pjsip openssl start --------------------------------------------------##PREBUILT_SHARED_LIBRARY
include $(CLEAR_VARS)  
LOCAL_MODULE    := libpj-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpj-arm-unknown-linux-androideabi.a  
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE    := libpjlib-util-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpjlib-util-arm-unknown-linux-androideabi.a
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE    := libpjmedia-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpjmedia-arm-unknown-linux-androideabi.a
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE    := libpjmedia-audiodev-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpjmedia-audiodev-arm-unknown-linux-androideabi.a
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE    := libpjmedia-codec-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpjmedia-codec-arm-unknown-linux-androideabi.a
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE    := libpjmedia-videodev-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpjmedia-videodev-arm-unknown-linux-androideabi.a
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE    := libpjnath-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpjnath-arm-unknown-linux-androideabi.a
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE    := libpjsip-simple-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpjsip-simple-arm-unknown-linux-androideabi.a
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE    := libpjsua2-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpjsua2-arm-unknown-linux-androideabi.a
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)  
LOCAL_MODULE    := libresample-arm-unknown-linux-androideabi  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libresample-arm-unknown-linux-androideabi.a
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_STATIC_LIBRARY)


#pjsip openssl end  --------------------------------------------------


include $(CLEAR_VARS)
LOCAL_MODULE := libcubic
LOCAL_SRC_FILES := Cubic.cpp 	
		
LOCAL_LDLIBS := -llog 

#设置可以使用C++代码  
LOCAL_CPPFLAGS += -std=c++11

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS :=optional


LOCAL_SHARED_LIBRARIES := libui \
						  libcutils \
						  libutils \
						  libbinder \
						  libsonivox \
						  libicuuc \
						  libexpat \
						  libOpenSLES \
						  libdl \
						  libhardware_legacy \
						  libstlport_static \
						  libz 
						  
LOCAL_STATIC_LIBRARIES := libcrypto \
						  libssl \
						  libcurl \
						  libpj-arm-unknown-linux-androideabi \
						  libpjlib-util-arm-unknown-linux-androideabi \
						  libpjmedia-arm-unknown-linux-androideabi \
						  libpjmedia-audiodev-arm-unknown-linux-androideabi \
						  libpjmedia-codec-arm-unknown-linux-androideabi \
						  libpjmedia-videodev-arm-unknown-linux-androideabi \
						  libpjnath-arm-unknown-linux-androideabi \
						  libpjsip-simple-arm-unknown-linux-androideabi \
						  libpjsua2-arm-unknown-linux-androideabi \
						  libresample-arm-unknown-linux-androideabi 
						  

LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/pjproject-2.7/pjlib/include\
					$(LOCAL_PATH)/libs/pjproject-2.7/pjsip/include \
					$(LOCAL_PATH)/libs/pjproject-2.7/pjlib-util/include \
					$(LOCAL_PATH)/libs/pjproject-2.7/pjmedia/include \
					$(LOCAL_PATH)/libs/pjproject-2.7/pjnath/include \
					$(LOCAL_PATH)/libs/openssl/include \
					$(LOCAL_PATH)/libs/rapidjson/include \
					$(LOCAL_PATH)/libs/curl/include \
					$(LOCAL_PATH)/framework \
					$(LOCAL_PATH)/util \
					$(LOCAL_PATH)/app/sipservice \
					$(LOCAL_PATH)/app/core


include $(BUILD_SHARED_LIBRARY)
