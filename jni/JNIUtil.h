#ifndef JNI_UTIL_H  
#define JNI_UTIL_H  

#include <jni.h>  
#include <string>  
using namespace std;  

class JNIUtil  
{  
  
public:  
    /* 注意Init和CleanUp必须在主线程中执行*/  
    static void Init(JNIEnv* env);  
    static void CleanUp(JNIEnv* env);  
public:  
    JNIUtil();  
    ~JNIUtil();  
    JNIEnv* GetJNIEnv();  
    string Jstring2String(jstring jstr);  
    jstring String2Jstring(const char* str);  
private:  
    static jclass m_strClass;  
    static jmethodID m_ctorID;  
    static jmethodID m_getByteID;  
    void Detach();  
    static JavaVM* m_sJVM;  
public:  
    bool m_bAttach;  
};  
  
  
#endif 