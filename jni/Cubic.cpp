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
#define CUBIC_APP_RELEASE_SIGN				"308201dd30820146020101300d06092a864886f70d010105050030373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b3009060355040613025553301e170d3137303930313033313835385a170d3437303832353033313835385a30373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b300906035504061302555330819f300d06092a864886f70d010101050003818d0030818902818100b0a42ffd4eb99022a2098fafd4fdf697f415ad50b15de89e39a54ea5379b8cd0f76ae1aaadc91476c891e7f79f0746ad7c17555d8b6afb72fe29cd7bb4adddecf289f09385b81807a8e2464fad16fb31868216447ac3fff39f737715c764ccb2e4a6f6c4e37caf846b6a57d0591f73eca1550e608540c364eeac622e30b980d50203010001300d06092a864886f70d01010505000381810044613ad3552c2c628b9721c1bfea27806b9922429b8970b778811f54cea86def3c2e1da690803a67b3725359cb0487a5dc66a2516fd3214aa66ccf037501e2acb1ce6d3096a1a71e6dbbcd855913a86004b0061eb091bbb4e03033dc0564763ff401c7e748f4f026615b7f2c15d4d4a4cef192186294fc110239b403d2c49231"
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
JNIEXPORT jint JNICALL meig_initAppInfo(JNIEnv *env, jclass type , jobject obj ) {
	jclass native_class = env->GetObjectClass(obj);
    jmethodID pm_id = env->GetMethodID(native_class, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jobject pm_obj = env->CallObjectMethod(obj, pm_id);
    jclass pm_clazz = env->GetObjectClass(pm_obj);
// 得到 getPackageInfo 方法的 ID
    jmethodID package_info_id = env->GetMethodID(pm_clazz, "getPackageInfo","(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jclass native_classs = env->GetObjectClass(obj);
    jmethodID mId = env->GetMethodID(native_classs, "getPackageName", "()Ljava/lang/String;");
    jstring pkg_str = static_cast<jstring>(env->CallObjectMethod(obj, mId));
// 获得应用包的信息
    jobject pi_obj = env->CallObjectMethod(pm_obj, package_info_id, pkg_str, 64);
// 获得 PackageInfo 类
    jclass pi_clazz = env->GetObjectClass(pi_obj);
// 获得签名数组属性的 ID
    jfieldID signatures_fieldId = env->GetFieldID(pi_clazz, "signatures", "[Landroid/content/pm/Signature;");
    jobject signatures_obj = env->GetObjectField(pi_obj, signatures_fieldId);
    jobjectArray signaturesArray = (jobjectArray)signatures_obj;
    jsize size = env->GetArrayLength(signaturesArray);
    jobject signature_obj = env->GetObjectArrayElement(signaturesArray, 0);
    jclass signature_clazz = env->GetObjectClass(signature_obj);
    jmethodID string_id = env->GetMethodID(signature_clazz, "toCharsString", "()Ljava/lang/String;");
    jstring str = static_cast<jstring>(env->CallObjectMethod(signature_obj, string_id));
    char *c_msg = (char*)env->GetStringUTFChars(str,0);
    
    if(strcmp(c_msg,CUBIC_APP_RELEASE_SIGN)==0)//签名一致  返回合法的 api key，否则返回错误
    {
		string packName  = CUtil::jstringTostring(env,pkg_str);
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
        return -1;
    }

        return 0;
    }else
    {
        return 404;
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

	jclass envcls = env->FindClass("android/os/Environment");
//	if (envcls == nullptr) return "";
	jmethodID id = env->GetStaticMethodID(envcls, "getExternalStorageDirectory", "()Ljava/io/File;"); 
	jobject fileObj = env->CallStaticObjectMethod(envcls,id,""); 
	jclass flieClass = env->GetObjectClass(fileObj);
	jmethodID getpathId = env->GetMethodID(flieClass, "getPath", "()Ljava/lang/String;"); 
	jstring pathStr = (jstring)env->CallObjectMethod(fileObj,getpathId,"");  

	string path = CUtil::jstringTostring(env, pathStr);
	LOGD("updateApp download path=%s",path.c_str() );
	path += "/MeiG/MeiGApp.apk";
	LOGD("updateApp download path=%s",path.c_str() );

	string req = CRemoteReport::updateApp(code, path);
	return env->NewStringUTF(req.c_str());
};

/*
 * Class:     getVoiceMessageList
 * Method:    meig_getVoiceMessageList
 * Signature: (II)I 
 */
JNIEXPORT jstring JNICALL meig_getVoiceMessageList(JNIEnv *env, jclass type, jstring group_uuid) {
	string j_group_uuid  = CUtil::jstringTostring(env,group_uuid);
	CubicCfgSet(CUBIC_CFG_push_group,j_group_uuid);
	CRemoteReport::getVMList(10);
	return group_uuid;
};


//------------------------------------jni loaded----------------------------------------------------------

//JNIEXPORT const char *classPathNameRx = "com/meigsmart/test/CubicUtil";
JNIEXPORT const char *classPathNameRx = "com/meigsmart/meigsdklibs/jni/CubicUtil";


static JNINativeMethod methodsRx[] = { 
	{"meig_getDeviceList", "()Ljava/lang/String;", (void*)meig_getDeviceList },
	{"meig_registerUser", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void*)meig_registerUser },
	{"meig_initAppInfo","(Ljava/lang/Object;)I",(void*)meig_initAppInfo },
	{"meig_updateApp","(Ljava/lang/String;)Ljava/lang/String;",(void*)meig_updateApp },
	{"meig_getVoiceMessageList","(Ljava/lang/String;)Ljava/lang/String;",(void*)meig_getVoiceMessageList },
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
