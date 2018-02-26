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
#define CUBIC_APP_SERVER_URL "https://www.meigelink.com/meiglink/api/v1"
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
	
	static int read_eeprom(int fd, char buff[], int addr, int count)  
	{  
		int res;  
		int i;  
		  
		for(i=0; i<PAGE_SIZE; i++)  
		{  
			buff[i]=0;  
		}  
		  

		if(write(fd, &addr, 1) != 1)  
			return -1;  
		usleep(10000);  
		res=read(fd, buff, count);  
		LOGD("read %d byte at 0x%.2x\n", res, addr);  
		for(i=0; i<PAGE_SIZE; i++)  
		{  
			LOGD("0x%.2x, ", buff[i]);  
		}  
		  
		return res;  
	}
	
};

// IMPLEMENT_CUBIC_APP(CoreApp)
static ICubicApp* cubic_get_app_instance()
{
    return &Cubic::getInstance();
};
static const char* cubic_get_app_name()
{
    return "Cubic";
};

//------------------------------------jni methods-------------------------------------------------------


/*
 * Class:     init
 * Method:    initAppInfo
 * Signature: (II)I
 */
JNIEXPORT void JNICALL initAppInfo(JNIEnv *env, jclass type , jobject obj ,jstring mac,jstring name) {
	jclass native_class = env->GetObjectClass(obj);
        jmethodID mId = env->GetMethodID(native_class, "getPackageName", "()Ljava/lang/String;");
        jstring p_name = static_cast<jstring>(env->CallObjectMethod(obj, mId));
	string packName  = CUtil::jstringTostring(env,p_name);
	CubicCfgSetRootPath(packName);
	CubicCfgSet(CUBIC_CFG_push_server,CUBIC_APP_SERVER_URL);
	CubicCfgSet(CUBIC_CFG_serial_num ,CUtil::jstringTostring( env, mac) );
	CubicCfgSet( CUBIC_CFG_push_uname, CUtil::jstringTostring( env, name) );
};

/*
 * Class:     getDeviceList
 * Method:    test
 * Signature: (II)I 
 */
JNIEXPORT jstring JNICALL getDeviceList(JNIEnv *env, jclass type) {
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
JNIEXPORT jint JNICALL registerUser(JNIEnv *env, jclass type, jstring versionCode, jstring versionName ) {
	string code = CUtil::jstringTostring(env, versionCode);
	string name = CUtil::jstringTostring(env, versionName);
	return CRemoteReport::activate(code, name );
};

/*
 * Class:     updateApp
 * Method:    updateApp
 * Signature: (II)I 
 */
JNIEXPORT jstring JNICALL updateApp(JNIEnv *env, jclass type, jstring versionCode) {
	string code = CUtil::jstringTostring(env, versionCode);
	string req = CRemoteReport::updateApp(code);
	return env->NewStringUTF(req.c_str());
};


//------------------------------------jni loaded----------------------------------------------------------

JNIEXPORT const char *classPathNameRx = "com/meigsmart/test/CubicUtil";


static JNINativeMethod methodsRx[] = { 
	{"getDeviceList", "()Ljava/lang/String;", (void*)getDeviceList },
	{"registerUser", "(Ljava/lang/String;Ljava/lang/String;)I", (void*)registerUser },
	{"initAppInfo","(Ljava/lang/Object;Ljava/lang/String;Ljava/lang/String;)V",(void*)initAppInfo },
	{"updateApp","(Ljava/lang/String;)Ljava/lang/String;",(void*)updateApp }
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
