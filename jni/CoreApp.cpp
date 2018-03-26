#include <jni.h>
#include "JNIHelp.h"
#include "cubic_inc.h"
#include <stdio.h>
#include <string.h>
#include "CUtil.cc"
#include "CRemoteReport.cc"
#include "CFramework.cc"

#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "CoreApp" 

#define UNUSED_ARG(arg) (void)arg
//#define CUBIC_APP_SERVER_URL 				"https://www.meigelink.com/meiglink/api/v1"
#define CUBIC_APP_SERVER_URL				"http://116.62.205.204:7000/meiglink/api/v1"
#define CUBIC_APP_SIP_DEFAILT_STUN_ADDR 	"120.24.77.212:3478"
#define CUBIC_APP_SIP_STUN_ADDR 			"120.24.77.212:3478"
#define CUBIC_APP_SIP_PROTOCOL 				"TLS"
#define CUBIC_APP_RELEASE_SIGN				"308203733082025ba00302010202046e946da7300d06092a864886f70d01010b0500306a310d300b06035504061304313131313111300f060355040813087368616e676861693111300f060355040713087368616e67686169310f300d060355040a0c06e7be8ee6a0bc310f300d060355040b0c06e7be8ee6a0bc3111300f060355040313086d6569676c696e6b301e170d3138303330353032343130345a170d3438303232363032343130345a306a310d300b06035504061304313131313111300f060355040813087368616e676861693111300f060355040713087368616e67686169310f300d060355040a0c06e7be8ee6a0bc310f300d060355040b0c06e7be8ee6a0bc3111300f060355040313086d6569676c696e6b30820122300d06092a864886f70d01010105000382010f003082010a0282010100901deea070d5c8d9e9f37cb3d0037d5830cec1fc76d079cdd1b8a39f67eb61acc486ad0402c3368468adf978dbd0be66c21a812543cb716ba401514f52c485ee126f3b4fa982479e9f90707e4a2c2f96e0e1019188cc44aa372407bab6a6f7a0cb98e6982f77543c1f8cdd85a75ccd073a2b5a9bd006d479aa46c1643e4dd2a5a5a1c1a6ac4799086a6ba6979d2e8f6510ca1ee63f3f6cd68b7dbec4ce028a81ed0c95ee6b9a7c0000ad731142b162b4be10f081dff2a7fe510d5688747d9cc71d2c1285a8ecca2acbc7f73a1244d9de3ba3584152d0f0bf98ae92c6e7e26b1bc8fbabb5bbaff0407f42443b1e20e437dd085eaf27807efdf9d49f62ecfecd9f0203010001a321301f301d0603551d0e041604141d1c4ee78436acdcc7de0ec5e28405589a2b6bb2300d06092a864886f70d01010b050003820101002da65470be1e946ce02e8e4c95ab1e6543ac1471c9b4eada9d65825c68da8064a4d1d55fadbd510c81d43d468f74f89780e0f13c3d21e601d25e29b0c906f31e4f7a849757464e25ca34d300c1f30d3c7053e31fc66175e95992466bb58c8a89c088b6716e39c711ae245c679e93a7fd121d56eb287584bc311af30e5b8eebec67326671588f35dd3e4a191be531a9c4a72958338ef4cd94ad7c99060a8ab45be20ab91e6e34a9979e63832d26af312d3e171dc22d79bbc0dc8acd81101c91f1ea1dd47fd069783efe686b79877721b1ac0d52f922b6db5479d2b8a88a4af1da698029e75ccf43bb44489f462a30cd9149d532bcac128e13a19b1180a7b7595d"

using namespace std;

class CoreApp : public ICubicApp
{
public :
	virtual ~CoreApp()
    {};

    static CoreApp &getInstance() {
        static CoreApp instance;
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
    return &CoreApp::getInstance();
};
static const char* cubic_get_app_name()
{
    return "CoreApp";
};

//------------------------------------jni methods-------------------------------------------------------

namespace android {
/*
 * Class:     init
 * Method:    initAppInfo
 * Signature: (II)I
 */
jint meig_initAppInfo(JNIEnv *env, jclass type , jobject obj ) {
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
    
    if(strcmp(c_msg,CUBIC_APP_RELEASE_SIGN)==0){//签名一致  返回合法的 api key，否则返回错误
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
    }else{
        return 404;
	}

};

/*
 * Class:     getDeviceList
 * Method:    test
 * Signature: (II)I 
 */
jstring meig_getDeviceList(JNIEnv *env, jclass type) {
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
jint meig_registerUser(JNIEnv *env, jclass type, jstring userName, jstring mac, jstring versionCode, jstring versionName ) {
	string u_name = CUtil::jstringTostring(env, userName);
	string j_mac = CUtil::jstringTostring(env, mac);
	string code = CUtil::jstringTostring(env, versionCode);
	string name = CUtil::jstringTostring(env, versionName);
	CubicCfgSet(CUBIC_CFG_serial_num ,j_mac );
	int ret = CRemoteReport::activate(u_name, code, name );
	if(ret == 0){
		LOGD("sip register start ...........");
		CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_SIP_REGISTER );
	}
	return ret;
};

/*
 * Class:     updateApp
 * Method:    updateApp
 * Signature: (II)I 
 */
jstring meig_updateApp(JNIEnv *env, jclass type, jstring versionCode) {
	string code = CUtil::jstringTostring(env, versionCode);
	string req = CRemoteReport::updateApp(code);
	
	/*
	jclass cls = env->FindClass("com/meigsmart/meigsdklibs/downService/DownService");
	jobject obj = env->AllocObject(cls);

	//start down service
	jmethodID down_service = env->GetMethodID(cls, "startDownService","(Ljava/lang/String;)V");
	env->CallVoidMethod(obj,down_service, pathStr );

	//start down load apk
	jmethodID down_load = env->GetMethodID(cls, "startDownLoad","(Ljava/lang/String;)V");
	env->CallVoidMethod(obj, down_load, env->NewStringUTF(down_url.c_str()));
	*/
		
	return env->NewStringUTF(req.c_str());
};

/*
 * Class:     downApp
 * Method:    downApp
 * Signature: ()V
 */
jstring meig_downApp(JNIEnv *env, jclass type ){
	jclass envcls = env->FindClass("android/os/Environment");
	if (envcls == NULL) return env->NewStringUTF("");
	jmethodID id = env->GetStaticMethodID(envcls, "getExternalStorageDirectory", "()Ljava/io/File;"); 
	jobject fileObj = env->CallStaticObjectMethod(envcls,id,""); 
	jclass flieClass = env->GetObjectClass(fileObj);
	jmethodID getpathId = env->GetMethodID(flieClass, "getPath", "()Ljava/lang/String;"); 
	jstring pathStr = (jstring)env->CallObjectMethod(fileObj,getpathId,"");  

	string path = CUtil::jstringTostring(env, pathStr);
	
	string req = CRemoteReport::downloadApk(path);
		
	return env->NewStringUTF(req.c_str() );
}

/*
 * Class:     getVoiceMessageList
 * Method:    meig_getVoiceMessageList
 * Signature: (II)I 
 */
jstring meig_getVoiceMessageList(JNIEnv *env, jclass type, jstring group_uuid) {
	string j_group_uuid  = CUtil::jstringTostring(env,group_uuid);
	CubicCfgSet(CUBIC_CFG_push_group,j_group_uuid);
	CRemoteReport::getVMList(10);
	return group_uuid;
};


//------------------------------------jni loaded----------------------------------------------------------

static const JNINativeMethod methodsRx[] = { 
	{"meig_getDeviceList", "()Ljava/lang/String;", (void*)meig_getDeviceList },
	{"meig_registerUser", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void*)meig_registerUser },
	{"meig_initAppInfo","(Ljava/lang/Object;)I",(void*)meig_initAppInfo },
	{"meig_updateApp","(Ljava/lang/String;)Ljava/lang/String;",(void*)meig_updateApp },
	{"meig_getVoiceMessageList","(Ljava/lang/String;)Ljava/lang/String;",(void*)meig_getVoiceMessageList },
	{"meig_downApp","()Ljava/lang/String;",(void*)meig_downApp },
};

int register_CoreApp(JNIEnv *env){
	return jniRegisterNativeMethods(env, "com/meigsmart/meigsdklibs/jni/CubicUtil", methodsRx, NELEM(methodsRx) );
}

};
