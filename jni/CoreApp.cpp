#include <jni.h>
#include "cubic_inc.h"
#include <stdio.h>
#include <string.h>
#include <rapidjson/document.h>
#include "CUtil.cc"
#include "CRemoteReport.cc"
#include "CFramework.cc"
#include "DownloadThread.cc"

#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "CoreApp" 

#define CUBIC_APP_SERVER_URL				"http://116.62.205.204:7000/meiglink/api/v1"

class CoreApp : public IDownloadThread
{
public :
	virtual ~CoreApp()
    {};

    static CoreApp &getInstance() {
        static CoreApp instance;
        return instance;
    };

    bool onInit() {
		LOGD("onInit");
        return true;
    };

    void onDeInit() {
        LOGD( "onDeInit" );
        return;
    };
	
	void onStartDownloadApk(const string &rootPath){
		DownloadThread::getInstance().registerUser( this );
		DownloadThread::getInstance().start();		
		DownloadThread::getInstance().addNewDownload(rootPath);
	};
	
	
	
	
	// interface for IDownloadThread
    virtual void downloadComplete( const string &local_path, int error ) {
        LOGD("downloadComplete local_path=%s",local_path.c_str() );
    };

};

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
    
    string packName  = CUtil::jstringTostring(env,pkg_str);
	CubicCfgSetRootPath(packName);
    CubicCfgSet(CUBIC_CFG_push_server,CUBIC_APP_SERVER_URL);
	
	return 0;
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
	return ret;
};

/*
 * Class:     updateApp
 * Method:    updateApp
 * Signature: (II)I 
 */
jint meig_updateApp(JNIEnv *env, jclass type, jstring versionCode) {
	string code = CUtil::jstringTostring(env, versionCode);
	string req = CRemoteReport::updateApp(code);

	Document doc;
	RETNIF_LOGE( doc.Parse<0>( req.c_str() ).HasParseError(), -1, "updateApp error when parse request: %s", req.c_str() );
	RETNIF_LOGE( !doc.HasMember( "result" ) || !doc["result"].IsNumber(), -1, "updateApp fail, not valid result !" );
	
	if(doc["result"].GetInt() == 200){
		LOGD("There is the latest app update");
		return 1;
	}
		
	return 0;
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
	
	CoreApp::getInstance().onStartDownloadApk(path);
			
	return env->NewStringUTF(path.c_str() );
}



//------------------------------------jni loaded----------------------------------------------------------
JNIEXPORT const char *classPathNameRx = "com/meigsmart/meigota/MeigOtaService";

static JNINativeMethod methodsRx[] = { 
	{"register", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void*)meig_registerUser },
	{"init","(Ljava/lang/Object;)I",(void*)meig_initAppInfo },
	{"updateApp","(Ljava/lang/String;)I",(void*)meig_updateApp },
	{"downApp","()Ljava/lang/String;",(void*)meig_downApp },
};

static jint registerNativeMethods(JNIEnv* env, const char* className,JNINativeMethod* gMethods, int numMethods)
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

static jint registerNatives(JNIEnv* env)
{
    jint ret = JNI_FALSE;

    if (registerNativeMethods(env, classPathNameRx, methodsRx, sizeof(methodsRx) / sizeof(methodsRx[0]))) {
        ret = JNI_TRUE;
    }

    LOGD("%s, done\n", __func__);
    return ret;
}


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
	
	CoreApp::getInstance().onInit();

    result = JNI_VERSION_1_4;

fail:
    return result;
}

