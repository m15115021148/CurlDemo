#include <jni.h>
#include <string.h>
#include "cubic_inc.h"
#include "CRemoteReport.cc"
#include "CMessager.cc"
#include "CCoreWatch.cc"
#include "CShareStatus.cc"
#include "CSafeQueue.cc"
#include "CConfig.cc"
#include "CUtil.cc"
#include "CLogger.cc"
#include <stdio.h>

#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "Cubic" 
#define LOG_TAG "Cubic"

#define UNUSED_ARG(arg) (void)arg
#define CUBIC_APP_SERVER_URL 				"https://www.meigelink.com/meiglink/api/v1"
#define CUBIC_APP_SIP_DEFAILT_STUN_ADDR 	"120.24.77.212:3478"
#define CUBIC_APP_SIP_STUN_ADDR 			"120.24.77.212:3478"
#define CUBIC_APP_SIP_PROTOCOL 				"TLS"
using namespace std;



class Cubic : public ICubicApp
{
public :
	virtual ~Cubic()
    {};

    static Cubic &getInstance() {
        static Cubic instance;
        return instance;
    };

	// interface of ICubicApp
    bool onInit() {
		LOGD("onInit %s",CUBIC_THIS_APP );
        return true;
    };

    // interface of ICubicApp
    void onDeInit() {
        LOGD( "onDeInit" );
        return;
    };
	
	// interface of ICubicApp
    virtual int onMessage( const string &str_src_app_name, int n_msg_id, const void* p_data ) {
        LOGE( "n_msg_id:<%d>", n_msg_id );
        return 0;
    };
	
};

// IMPLEMENT_CUBIC_APP(CoreApp)
static ICubicApp* cubic_get_app_instance()
{
    return &Cubic::getInstance();
};
static const char* cubic_get_app_name()
{
    return "CoreApp";
};

//------------------------------------jni methods-------------------------------------------------------


/*
 * Class:     init
 * Method:    initAppInfo
 * Signature: (II)I
 */
JNIEXPORT void JNICALL meig_initAppInfo(JNIEnv *env, jclass type , jobject obj ) {
	jclass native_class = env->GetObjectClass(obj);
	jmethodID mId = env->GetMethodID(native_class, "getPackageName", "()Ljava/lang/String;");
	jstring p_name = static_cast<jstring>(env->CallObjectMethod(obj, mId));
	string packName  = CUtil::jstringTostring(env,p_name);
	CubicCfgSetRootPath(packName);
	CubicCfgSet(CUBIC_CFG_push_server,CUBIC_APP_SERVER_URL);
	CubicCfgSet(CUBIC_CFG_sip_defailt_stun_addr, CUBIC_APP_SIP_DEFAILT_STUN_ADDR);
	CubicCfgSet(CUBIC_CFG_sip_stun_addr, CUBIC_APP_SIP_STUN_ADDR);
	CubicCfgSet(CUBIC_CFG_sip_protocol, CUBIC_APP_SIP_PROTOCOL);

	// setup signal handle
    signal( SIGINT,  main_quit );
    signal( SIGHUP,  main_quit );
    signal( SIGABRT, main_quit );
    signal( SIGTERM, main_quit );
    signal( SIGSTOP, main_quit );
    signal( SIGCHLD, main_quit );

    // setup frameork instance
    if( !CFramework::GetInstance().init() ) {
        CubicLogE( "Fail to init app" );
        return ;
    }
};

/*
 * Class:     getDeviceList
 * Method:    test
 * Signature: (II)I 
 */
JNIEXPORT jstring JNICALL meig_getDeviceList(JNIEnv *env, jclass type) {
//	string weburl = CUtil::jstringTostring(env,jstr);
	
	string str = CRemoteReport::getDeviceList();
	
	//string ---> jstring   env->NewStringUTF(str.c_str())
	return env->NewStringUTF(str.c_str()); 
	
};

/*
 * Class:     registerUser
 * Method:    test
 * Signature: (II)I 
 */
JNIEXPORT jint JNICALL meig_registerUser(JNIEnv *env, jclass type, jstring userName, jstring mac, jstring versionCode, jstring versionName ) {
	string u_name = CUtil::jstringTostring(env, userName);
	string j_mac = CUtil::jstringTostring(env, mac);
	string code = CUtil::jstringTostring(env, versionCode);
	string name = CUtil::jstringTostring(env, versionName);
	CubicCfgSet(CUBIC_CFG_serial_num ,j_mac );
	int ret = CRemoteReport::activate(u_name, code, name );
	if(ret == 0){
		LOGD("sip register start ...........");
		CubicPost( CUBIC_APP_NAME_SIP_SERVICE, CUBIC_MSG_SIP_REGISTER );
	}
	return ret;
};

/*
 * Class:     updateApp
 * Method:    updateApp
 * Signature: (II)I 
 */
JNIEXPORT jstring JNICALL meig_updateApp(JNIEnv *env, jclass type, jstring versionCode) {
	string code = CUtil::jstringTostring(env, versionCode);
	string req = CRemoteReport::updateApp(code);
	return env->NewStringUTF(req.c_str());
};

//------------------------------------jni loaded----------------------------------------------------------

//JNIEXPORT const char *classPathNameRx = "com/meigsmart/test/CubicUtil";
JNIEXPORT const char *classPathNameRx = "com/meigsmart/meigsdklibs/jni/CubicUtil";


static JNINativeMethod methodsRx[] = { 
	{"meig_getDeviceList", "()Ljava/lang/String;", (void*)meig_getDeviceList },
	{"meig_registerUser", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void*)meig_registerUser },
	{"meig_initAppInfo","(Ljava/lang/Object;)V",(void*)meig_initAppInfo },
	{"meig_updateApp","(Ljava/lang/String;)Ljava/lang/String;",(void*)meig_updateApp },
};

/*
 * Register several native methods for one class.
 */
static jint registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    LOGD("%s, success\n", __func__);
    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static jint registerNatives(JNIEnv* env)
{
    jint ret = JNI_FALSE;

    if (registerNativeMethods(env, classPathNameRx,methodsRx,
        sizeof(methodsRx) / sizeof(methodsRx[0]))) {
        ret = JNI_TRUE;
    }

    LOGD("%s, done\n", __func__);
    return ret;
}


// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */

typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	UNUSED_ARG(reserved);
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;

    LOGI("JNI_OnLoad");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        goto fail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        LOGE("ERROR: registerNatives failed");
        goto fail;
    }

    result = JNI_VERSION_1_4;

fail:
    return result;
}
