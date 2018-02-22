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

#pjsip openssl
include $(CLEAR_VARS)  
LOCAL_MODULE    := libpjsua2  
LOCAL_SRC_FILES := libs/pjproject-2.7/lib/libpjsua2.so  
LOCAL_SHARED_LIBRARIES := libz
LOCAL_STATIC_LIBRARIES := libcrypto libssl
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libs/openssl/include
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libcubic
LOCAL_SRC_FILES := Cubic.cpp 	
		
LOCAL_LDLIBS := -llog 

#设置可以使用C++代码  
LOCAL_CPPFLAGS += -std=c++11

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS :=optional


LOCAL_SHARED_LIBRARIES := libpjsua	\
						  libpjlib-util \
						  libpjlib \
						  libpjsip \
						  libpjsip-simple \
						  libpjsip-ua \
						  libpjmedia \
						  libpjmedia-audiodev \
						  libpjmedia-codec  \
						  libui \
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
						  libz \
						  libpjsua2
						  
LOCAL_STATIC_LIBRARIES := libcrypto libssl libcurl				  
						  

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
