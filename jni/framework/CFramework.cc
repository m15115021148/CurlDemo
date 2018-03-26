/**
 * @file CFramework.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Framework, main frame work of cubic item
 * @detail Cubic Framework, main frame work of cubic item
 */

#ifndef _CFRAMEWORK_CC_
#define _CFRAMEWORK_CC_ 1

#include "CConfig.cc"
#include "cubic_inc.h"

#include <signal.h>
#include <iostream>
using namespace std;

#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "framework"


class CFramework
{
private:
    CConfig      m_config;

    bool mb_initAppOk;
    bool mb_stopPump;

    CFramework()
        : m_config( CUBIC_CONFIG_ROOT_PATH )
        , mb_initAppOk( false )
        , mb_stopPump( true )
    {};

    ~CFramework()
    {};

public:
    inline static CFramework &GetInstance() {
        static CFramework instance;
        return instance;
    };


    bool init() {
        if( mb_initAppOk ) {
            return true;
        }
		LOGD("init .." );
        
        mb_initAppOk = true;
        return true;
    };

    void deinit() {
        
    };

    void onMessage( int n_session_id, const string &str_src_app_name, int n_msg_id, const void* p_data ) {
        
    };


    void stop() {
        CubicLogD( "framework stop" );
        mb_stopPump = true;
    };

    CConfig &GetConfig() {
        return m_config;
    };

};

static void main_quit( int sig )
{
    switch( sig ) {
    case SIGINT:
    case SIGHUP:
    case SIGABRT:
    case SIGTERM:
    case SIGSTOP:
        CFramework::GetInstance().stop();
        break;

    case SIGCHLD: {
            pid_t pid;
            pid = waitpid( -1, NULL, WNOHANG );

            if( pid != -1 && errno != 10 ) {
                CubicLogD( "onChildDead pid[%d]", pid );
            }
        }
        break;
    }
};


int main( int argc, const char* argv[] )
{
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

    // deinit framwork
    CFramework::GetInstance().deinit();
    return 0;
};

#endif //_CFRAMEWORK_CC_

