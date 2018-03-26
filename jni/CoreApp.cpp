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
#define CUBIC_APP_SERVER_URL				"http://116.62.205.204:7000/meiglink/api/v1"

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
jstring meig_updateApp(JNIEnv *env, jclass type, jstring versionCode) {
	string code = CUtil::jstringTostring(env, versionCode);
	string req = CRemoteReport::updateApp(code);		
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


//------------------------------------jni loaded----------------------------------------------------------

static const JNINativeMethod methodsRx[] = { 
	{"register", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void*)meig_registerUser },
	{"init","(Ljava/lang/Object;)I",(void*)meig_initAppInfo },
	{"updateApp","(Ljava/lang/String;)Ljava/lang/String;",(void*)meig_updateApp },
	{"downApp","()Ljava/lang/String;",(void*)meig_downApp },
};

int register_CoreApp(JNIEnv *env){
	return jniRegisterNativeMethods(env, "com/meigsmart/meigota/MeigOtaService", methodsRx, NELEM(methodsRx) );
}

};
