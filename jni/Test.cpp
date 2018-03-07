#include "jni.h"
#include "cubic_inc.h"
#include "JNIHelp.h"

#define LOG_TAG "onload"


namespace android {

void getTest(JNIEnv* env, jobject clazz){
    
}


static const JNINativeMethod g_methods[] = {
    { "getTest", "()V", (void*)getTest },
};


int register_Test(JNIEnv *env){
        return jniRegisterNativeMethods(env, "com/meigsmart/meigsdklibs/jni/Test", g_methods, NELEM(g_methods) );
}

};
