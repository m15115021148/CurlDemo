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
#include <openssl/hmac.h>

#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "Cubic" 
#define LOG_TAG "Cubic"

#define UNUSED_ARG(arg) (void)arg
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
        LOGD( "%s onInit: ");
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
 * Class:     getMsg
 * Method:    test
 * Signature: (II)I
 */
JNIEXPORT jstring JNICALL getMsg(JNIEnv *env, jclass type , jstring msg ) {
	//string str = CUtil::jstringTostring( env, msg) ;
	
//	LOGD("test ....push_server=%s",CubicCfgGetStr( str ).c_str());
		
	jclass envcls = env->FindClass("android/os/Environment"); //获得类引用  
    if (envcls == nullptr) return msg;  
  
    //找到对应的类，该类是静态的返回值是File  
    jmethodID id = env->GetStaticMethodID(envcls, "getExternalStorageDirectory", "()Ljava/io/File;");  
  
    //调用上述id获得的方法，返回对象即File file=Enviroment.getExternalStorageDirectory()  
    //其实就是通过Enviroment调用 getExternalStorageDirectory()  
	jobject fileObj = env->CallStaticObjectMethod(envcls,id,"");  
    
    //通过上述方法返回的对象创建一个引用即File对象  
	jclass flieClass = env->GetObjectClass(fileObj); //或得类引用  
    //在调用File对象的getPath()方法获取该方法的ID，返回值为String 参数为空  
    jmethodID getpathId = env->GetMethodID(flieClass, "getPath", "()Ljava/lang/String;");  
   //调用该方法及最终获得存储卡的根目录  
    jstring pathStr = (jstring)env->CallObjectMethod(fileObj,getpathId,"");  
    
 //   path = env->GetStringUTFChars(pathStr,NULL);
	string str = CUtil::jstringTostring( env, pathStr) ;
	LOGD("str str=%s",str.c_str() );
	
    return msg;
};

/*
 * Class:     getMsg
 * Method:    test
 * Signature: (II)I
 */
JNIEXPORT jstring JNICALL getOpenSSL(JNIEnv *env, jclass type , jstring data ) {
	// jstring 转 char*
	//const char* msg = (char *) env->GetStringUTFChars(data, false);
	// char* 转 string
	std::string str = "123456";
	string str1 = CUtil::getMd5(str.c_str());
	LOGD("result str=%s",str1.c_str());
	return env->NewStringUTF(str1.c_str());
};

/*
 * Class:     getMsg
 * Method:    test
 * Signature: (II)I
 */
JNIEXPORT jbyteArray JNICALL hmacSha256(JNIEnv *env, jclass type , jbyteArray content ) {
	unsigned char key[] = {0x6B, 0x65, 0x79};

	unsigned int result_len;
	unsigned char result[EVP_MAX_MD_SIZE];

	  // get data from java array
	jbyte *data = env->GetByteArrayElements(content, NULL);
	size_t dataLength = env->GetArrayLength(content);

	 HMAC(EVP_sha256(),
		  key, 3,
		  (unsigned char *) data, dataLength,
		  result, &result_len);

	 // release the array
	env->ReleaseByteArrayElements(content, data, JNI_ABORT);

	 // the return value
	 jbyteArray return_val = env->NewByteArray(result_len);
	 env->SetByteArrayRegion(return_val, 0, result_len, (jbyte *) result);
    return return_val;
};

/*
 * Class:     getMsg
 * Method:    test
 * Signature: (II)I 
 */
JNIEXPORT jstring JNICALL testCurl(JNIEnv *env, jclass type ,jstring jstr) {
	string weburl = CUtil::jstringTostring(env,jstr);
	
	std::string str = CRemoteReport::getDeviceList(weburl.c_str());
	
	//string ---> jstring   env->NewStringUTF(str.c_str())
	return env->NewStringUTF(str.c_str()); 
	
};

/*
 * Class:     getMsg
 * Method:    test
 * Signature: (II)I 
 */
JNIEXPORT void JNICALL registerUser(JNIEnv *env, jclass type) {
	
};


//------------------------------------jni loaded----------------------------------------------------------

JNIEXPORT const char *classPathNameRx = "com/meigsmart/test/CubicUtil";


static JNINativeMethod methodsRx[] = { 
	{"getMsg", "(Ljava/lang/String;)Ljava/lang/String;", (void*)getMsg },
	{"getOpenSSL", "(Ljava/lang/String;)Ljava/lang/String;", (void*)getOpenSSL },
	{"hmacSha256", "([B)[B", (void*)hmacSha256 },
	{"testCurl", "(Ljava/lang/String;)Ljava/lang/String;", (void*)testCurl },
	{"registerUser", "()V", (void*)registerUser },
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