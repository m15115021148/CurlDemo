#include <jni.h>
#include "cubic_inc.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <rapidjson/document.h>
#include "CRemoteReport.cc"
#include "CFramework.cc"
#include "DownloadThread.cc"
#include "JNIUtil.h"

#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "CoreApp" 

#define CUBIC_APP_SERVER_URL				"http://116.62.205.204:7000/meiglink/api/v1"

JNIUtil util;

class CoreApp : public IDownloadThread , public IDownloadCallBack, public CThread
{
private :
	double 				now_size;
	double				now_total;
	
	CoreApp()
        : now_size( 0 )
		, now_total( 0 )
    {};
	
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
		CRemoteReport::getInstance().registerDownloadCallBack( this );
		DownloadThread::getInstance().registerUser( this );
		DownloadThread::getInstance().start();		
		DownloadThread::getInstance().addNewDownload(rootPath);
	};
		
	// interface for IDownloadThread
    virtual void downloadComplete( const string &local_path, int error ) {
        LOGD("downloadComplete local_path=%s",local_path.c_str() );	
		JavaVM* vm = util.GetVM();
		JNIEnv* env = util.GetJNIEnv(); 
		
		/*
		vm->AttachCurrentThread(&(env), NULL);
		
		
		jclass cls = env->FindClass("com/meigsmart/meigota/MeigOtaService");
		jobject obj = env->AllocObject(cls);
		
		jmethodID down_success = env->GetMethodID(cls, "downSuccess","(Ljava/lang/String;)V");
		env->CallVoidMethod(obj,down_success, env->NewStringUTF( local_path.c_str() ) );
		
		vm->DetachCurrentThread();*/
		
		stop();
    };

	// interface for IDownloadCallBack
	virtual void downloadProgress( double dltotal, double dlnow ) {
		LOGD("downloadProgress ->(%g %%)\n", dlnow*100.0/dltotal);  
		now_size = dlnow;
		now_total = dltotal;
			
	};
	
	// interface for IDownloadCallBack
	virtual void downloadFailuer( string result) {
		
	};
	
	// interface for IDownloadCallBack
	virtual void downloadCancel() {
		
	};
	
	virtual RunRet run( void* user ) {
		UNUSED_ARG( user );
		/*LOGD("download apk ....start run");
				
		JNIEnv* env = util.GetJNIEnv(); 
		
		jclass cls = env->FindClass("com/meigsmart/meigota/MeigOtaService");
		jobject obj = env->AllocObject(cls);
			
		jmethodID down_success = env->GetMethodID(cls, "downProgress","(DD)V");
		
		env->CallVoidMethod(obj,down_success, now_size, now_total);*/
		
		if(user == NULL) return RUN_END;
  
		//JavaVM* vm = util.GetVM();
		//JNIEnv* env = util.GetJNIEnv(); 
		
		cubic_down_load* p_down = (cubic_down_load*)user;
		
		
		JavaVM* vm = p_down->vm;
		JNIEnv* env = p_down->env; 
		
		int getEnvStat = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
		if (getEnvStat == JNI_EDETACHED) {
			if (vm->AttachCurrentThread( &env, NULL) != 0) {
				return RUN_END;
			}
		}

		
		//强转回来
		jobject jcallback = p_down->jobj;
		
		//通过强转后的jcallback 获取到要回调的类
		jclass javaClass = env->GetObjectClass(jcallback);

		if (javaClass == 0) {
			LOGE("Unable to find class");
			vm->DetachCurrentThread();
			return RUN_END;
		}

	   //获取要回调的方法ID
		jmethodID javaCallbackId = env->GetMethodID(javaClass, "onProgress","(DD)V");
		
		if (javaCallbackId == NULL) {
			LOGE("Unable to find method:onProgress");
			return RUN_END;
		}
		//执行回调
		env->CallVoidMethod(jcallback, javaCallbackId, getNowSize(), getNowTotal());	
		
		return RUN_CONTINUE;
	};
	
	virtual void onStop( void* user ) {
        UNUSED_ARG( user );
		LOGD("onStop.");
		
		/*
		if(user == NULL) return;
		
		JavaVM* vm = util.GetVM();
		JNIEnv* env = util.GetJNIEnv(); 
		
		//强转回来
		jobject jcallback = (jobject)user;
		
		//释放当前线程
		vm->DetachCurrentThread();
		env = NULL;
		
		//释放你的全局引用的接口，生命周期自己把控
		env->DeleteGlobalRef(jcallback);
		jcallback = NULL;*/
		
    };
	
	double &getNowSize(){
		return now_size;
	}; 
	
	double &getNowTotal(){
		return now_total;
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
    
    string packName  = util.Jstring2String(pkg_str);
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
	string u_name = util.Jstring2String( userName );
	string j_mac = util.Jstring2String( mac );
	string code = util.Jstring2String(versionCode);
	string name = util.Jstring2String(versionName);
	CubicCfgSet(CUBIC_CFG_serial_num ,j_mac );
	int ret = CRemoteReport::getInstance().activate(u_name, code, name );
	return ret;
};

/*
 * Class:     updateApp
 * Method:    updateApp
 * Signature: (II)I 
 */
jint meig_updateApp(JNIEnv *env, jclass type, jstring versionCode) {
	string code = util.Jstring2String( versionCode );
	string req = CRemoteReport::getInstance().updateApp(code);

	Document doc;
	RETNIF_LOGE( doc.Parse<0>( req.c_str() ).HasParseError(), -1, "updateApp error when parse request: %s", req.c_str() );
	RETNIF_LOGE( !doc.HasMember( "result" ) || !doc["result"].IsNumber(), -1, "updateApp fail, not valid result !" );
	
	if(doc["result"].GetInt() == 200){
		LOGD("There is the latest app update");
		return 1;
	}
		
	return 0;
};

JavaVM *g_VM;
/*
 * Class:     downApp
 * Method:    downApp
 * Signature: ()V
 */
jstring meig_downApp(JNIEnv *env, jclass type, jobject jcallback ){
	jclass envcls = env->FindClass("android/os/Environment");
	if (envcls == NULL) return env->NewStringUTF("");
	jmethodID id = env->GetStaticMethodID(envcls, "getExternalStorageDirectory", "()Ljava/io/File;"); 
	jobject fileObj = env->CallStaticObjectMethod(envcls,id,""); 
	jclass flieClass = env->GetObjectClass(fileObj);
	jmethodID getpathId = env->GetMethodID(flieClass, "getPath", "()Ljava/lang/String;"); 
	jstring pathStr = (jstring)env->CallObjectMethod(fileObj,getpathId,"");  

	string path = util.Jstring2String( pathStr);
	
	CoreApp::getInstance().onStartDownloadApk(path);	
	
	cubic_down_load* arg;
	memset( &arg, 0, sizeof(arg) );
	
	env->GetJavaVM(&g_VM);
	
	arg->vm = g_VM;
	arg->env = env;
	arg->jcls = type;
	
	//jobject callback = ;

	arg->jobj = env->NewGlobalRef(jcallback);
	
	//CoreApp::getInstance().initDownload(arg);
	
	
	CoreApp::getInstance().start(arg);
	
	return env->NewStringUTF(path.c_str() );
}

void* Download(void* p) {

	if(p == NULL) return NULL;
  
    JavaVM* vm = util.GetVM();
	JNIEnv* env = util.GetJNIEnv(); 
    
    //强转回来
    jobject jcallback = (jobject)p;
    
    //通过强转后的jcallback 获取到要回调的类
    jclass javaClass = env->GetObjectClass(jcallback);

    if (javaClass == 0) {
        LOGE("Unable to find class");
        vm->DetachCurrentThread();
        return NULL;
    }

   //获取要回调的方法ID
    jmethodID javaCallbackId = env->GetMethodID(javaClass, "downProgress","(DD)V");
	
    if (javaCallbackId == NULL) {
        LOGE("Unable to find method:downProgress");
        return NULL;
    }
	//执行回调
    env->CallVoidMethod(jcallback, javaCallbackId,
								CoreApp::getInstance().getNowSize(),
								CoreApp::getInstance().getNowTotal());
    
    //释放当前线程
	vm->DetachCurrentThread();
    env = NULL;
    
    //释放你的全局引用的接口，生命周期自己把控
    env->DeleteGlobalRef(jcallback);
    jcallback = NULL;
	return p;
}

void sendJavaMsg(JNIEnv* env, jobject instance, jmethodID func, const string msg) {
    jstring javaMsg = env->NewStringUTF(msg.c_str());
    env->CallVoidMethod(instance, func, javaMsg);
    env->DeleteLocalRef(javaMsg);
}

//------------------------------------jni loaded----------------------------------------------------------
JNIEXPORT const char *classPathNameRx = "com/meigsmart/meigota/MeigOtaService";

static JNINativeMethod methodsRx[] = { 
	{"register", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void*)meig_registerUser },
	{"init","(Ljava/lang/Object;)I",(void*)meig_initAppInfo },
	{"updateApp","(Ljava/lang/String;)I",(void*)meig_updateApp },
	{"downApp","(Lcom/meigsmart/meigota/DownCallBack;)Ljava/lang/String;",(void*)meig_downApp },
};

static jint registerNativeMethods(JNIEnv* env, const char* className,JNINativeMethod* gMethods, int numMethods){
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

static jint registerNatives(JNIEnv* env){
    jint ret = JNI_FALSE;

    if (registerNativeMethods(env, classPathNameRx, methodsRx, sizeof(methodsRx) / sizeof(methodsRx[0]))) {
        ret = JNI_TRUE;
    }

    LOGD("%s, done\n", __func__);
    return ret;
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
	JNIEnv* env;
	LOGI("JNI_OnLoad");
	
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
		LOGE("ERROR: GetEnv failed");
        return JNI_ERR; 
    }
	
	if (registerNatives(env) != JNI_TRUE) {
        LOGE("ERROR: registerNatives failed");
        return JNI_ERR;
    }
		
	JNIUtil::Init(env); 
		
	CoreApp::getInstance().onInit();
	
    return  JNI_VERSION_1_6;
	
}

