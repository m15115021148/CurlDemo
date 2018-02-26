/**
 * @file cubic_func.h
 * @brief macro function for all cubic
 * @detail macro function for all cubic
 */

#ifndef _CUBIC_FUNC_H
#define _CUBIC_FUNC_H 1



#define UNUSED_ARG(arg) (void)arg


#define MIN(x,y) \
    ((x)<(y)?(x):(y))

#define MAX(x,y) \
    ((x)>(y)?(x):(y))

#define FREE(x) \
    if((x)!=NULL){ \
        free(x); \
        (x)=NULL; \
    }

#define DELETE(x) \
    if((x)!=NULL){ \
        delete x; \
        (x)=NULL; \
    }

#define MALLOC(ptr, n) \
    ptr = (decltype(ptr))malloc(n+4); \
    if( ptr != NULL ) memset(ptr, 0, n+4);

#define CLOSE(x) \
    if( (x) >= 0 ) { \
        close( x ); \
        (x) = -1; \
    }

#define RETNIF(condition, ret) \
    if(condition) return ret;

#define RETIF(condition) \
    if(condition) return;

#define BREAKIF(condition) \
    if(condition) break;
#define BREAKNIF( x, ret, n ) if( (x) ){ (ret) = (n); break; }

#define CONTINUEIF(condition) \
    if(condition) continue;

#define IMPLEMENT_CUBIC_APP( app_name ) \
    static ICubicApp* cubic_get_app_instance() { \
        static app_name instance; \
        return &instance; \
    }; \
    static const char* cubic_get_app_name() { \
        return #app_name; \
    };


#define CUBIC_THIS_APP \
    CFramework::GetInstance().getAppName().c_str()

#define CubicPost( dst, id ) \
    CFramework::GetInstance().GetMessger().postRequest(dst, id, (int)0)

#define CubicPostReq( dst, id, data ) \
    CFramework::GetInstance().GetMessger().postRequest(dst, id, data)

#define CubicSendReq( dst, id, Senddata, data_recv) \
    CFramework::GetInstance().GetMessger().sendRequest(dst, id, Senddata, data_recv)

#define CubicSetTimer( ms, timer ) \
    CTimer::setTimer(ms, timer)

#define CubicSetTimerInterval( ms, timer ) \
    CTimer::setTimerInterval(ms, timer)

#define CubicKillTimer(n_timer_id) \
    CTimer::killTimer(n_timer_id)



/**
 * local configures API
 */

#define CubicCfgGet( key, def ) \
    CFramework::GetInstance().GetConfig().get( key, def )

#define CubicCfgGetI( key ) \
    CFramework::GetInstance().GetConfig().get<int>( key, 0 )

#define CubicCfgGetU( key ) \
    CFramework::GetInstance().GetConfig().get<unsigned int>( key, 0 )

#define CubicCfgGetF( key ) \
    CFramework::GetInstance().GetConfig().get<float>( key, 0.0f )

#define CubicCfgGetStr( key ) \
    CFramework::GetInstance().GetConfig().get<string>( key, "" )

#define CubicCfgGetV( key, def, ... ) \
    CFramework::GetInstance().GetConfig().getv( key, def, __VA_ARGS__ )

#define CubicCfgGetVI( key, ...  ) \
    CFramework::GetInstance().GetConfig().getv<int>( key, 0, __VA_ARGS__ )

#define CubicCfgGetVU( key, ...  ) \
    CFramework::GetInstance().GetConfig().getv<unsigned int>( key, 0, __VA_ARGS__ )

#define CubicCfgGetVF( key, ...  ) \
    CFramework::GetInstance().GetConfig().getv<float>( key, 0.0, __VA_ARGS__ )

#define CubicCfgGetVStr( key, ...  ) \
    CFramework::GetInstance().GetConfig().getv<string>( key, "", __VA_ARGS__ )


#define CubicCfgSetRootPath( key ) \
	CFramework::GetInstance().GetConfig().setRootPath( key )

#define CubicCfgSet( key, val ) \
    CFramework::GetInstance().GetConfig().set( key, val )

#define CubicCfgSetV( key, val, ... ) \
    CFramework::GetInstance().GetConfig().setv( key, val, __VA_ARGS__ )

#define CubicCfgEnum( name ) \
    CFramework::GetInstance().GetConfig().enumSub( name )

#define CubicCfgCommit( name) \
    CFramework::GetInstance().GetConfig().commit(name)

#define CubicCfgSetListen(str_key, p_listen) \
    CFramework::GetInstance().GetConfig().setListen(str_key, p_listen)

#define CubicCfgUnsetListen(str_key) \
    CFramework::GetInstance().GetConfig().unsetListen(str_key)


/**
 * local status API
 */

#define CubicStatGet(strKey, def) \
    CFramework::GetInstance().GetShareStatus().get(strKey, def)

#define CubicStatGetStr(strKey) \
    CFramework::GetInstance().GetShareStatus().get<string>(strKey, "")

#define CubicStatGetI(strKey) \
    CFramework::GetInstance().GetShareStatus().get<int>(strKey, 0)

#define CubicStatGetU(strKey) \
    CFramework::GetInstance().GetShareStatus().get<unsigned int>(strKey, 0)

#define CubicStatGetF(strKey) \
    CFramework::GetInstance().GetShareStatus().get<float>(strKey, 0.0f)

#define CubicStatSet(strKey, strValue) \
    CFramework::GetInstance().GetShareStatus().set(strKey, strValue)

#endif //_CUBIC_FUNC_H
