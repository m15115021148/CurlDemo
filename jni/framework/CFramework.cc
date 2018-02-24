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
#include "CMessager.cc"
#include "CTimer.cc"
#include "CLogger.cc"
#include "CCoreWatch.cc"
#include "CShareStatus.cc"
#include "CSafeQueue.cc"
#include "cubic_inc.h"

#include <signal.h>
#include <iostream>
using namespace std;

#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "framework"
#define LOG_TAG "framework"

class ICubicApp
{
public:
    virtual ~ICubicApp() {};

    virtual bool onInit() {
        return true;
    };

    virtual void onDeInit() {
        return;
    };

    virtual int onMessage( const string &str_src_app_name, int n_msg_id, const void* p_data ) = 0;
};



static ICubicApp* cubic_get_app_instance();
static const char* cubic_get_app_name();

class CFramework : public IMsgListener
{
public:
    static const int MESSAGE_POP_WAIT = 1000; // ms

private:
    typedef struct Message {
        int n_session_id;
        string str_src_app_name;
        int n_msg_id;
        unsigned char data[CMessager::MAX_MESSAGE_SIZE];
    } TMessage;

    CConfig      m_config;
    CMessager    m_messager;
    CLogger      m_logger;
    CCoreWatch   m_watch;
    CShareStatus m_stat;


    bool mb_initAppOk;
    bool mb_stopPump;
    CSafeQueue<TMessage> m_message_box;

    CFramework()
        : m_config( CUBIC_CONFIG_ROOT_PATH )
        , m_messager( cubic_get_app_name() )
        , m_logger( cubic_get_app_name() )
        , m_watch( cubic_get_app_name() )
        , m_stat( cubic_get_app_name() )
        , mb_initAppOk( false )
        , mb_stopPump( true )
        , m_message_box()
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

        // load config
        int n_level_limit = 0;
        n_level_limit = m_config.get( CUBIC_CFG_log_level_limit, ( int )CUBIC_LOG_LEVEL_DEBUG );
        m_logger.setLevelLimit( n_level_limit );
        // setup message listener
        RETNIF_LOGE( CMessager::ERR_NO_ERROR != m_messager.startListen( this ), false,  "setup fail to start message listen !" );
        m_watch.feed();
        // init app
        RETNIF_LOGE( cubic_get_app_instance() == NULL, false, "App instance is null" );
        RETNIF_LOGE( !cubic_get_app_instance()->onInit(), false, "App init return false" );
        mb_initAppOk = true;
        m_watch.feed();
        return true;
    };

    void deinit() {
        // deinit app
        if( cubic_get_app_instance() ) { cubic_get_app_instance()->onDeInit(); }

        // stop message listener
        m_messager.stopListen();
    };

    void onMessage( int n_session_id, const string &str_src_app_name, int n_msg_id, const void* p_data ) {
        TMessage message;
        message.n_session_id = n_session_id;
        message.n_msg_id = n_msg_id;
        message.str_src_app_name = str_src_app_name;
        memcpy( message.data, p_data, CMessager::MAX_MESSAGE_SIZE );
        // just push it to queue
        m_message_box.push( message );
    };

    void pumpMessage() {
        mb_stopPump = false;
        // pop up message
        TMessage message;
        CSafeQueue<TMessage>::EErrCode err;
        int ret;
        CubicLogD( "framework pumpMessage" );

        while( !mb_stopPump ) {
            ret = -1;
            err = m_message_box.pop( message, MESSAGE_POP_WAIT );
            m_watch.feed();
            CONTINUEIF( err != CSafeQueue<TMessage>::ERR_NO_ERROR );

            if( cubic_get_app_instance() ) {
                ret = cubic_get_app_instance()->onMessage( message.str_src_app_name, message.n_msg_id, message.data );
            }
            else {
                usleep( 10000 );
                ret = 0;
            }

            m_messager.response( message.n_session_id, message.str_src_app_name, message.n_msg_id, ret );
        }
    };

    void stop() {
        CubicLogD( "framework stop" );
        mb_stopPump = true;
    };

    string getAppName() {
        return ( string )cubic_get_app_name();
    };

    CConfig &GetConfig() {
        return m_config;
    };

    CLogger &getLoger() {
        return m_logger;
    };

    CMessager &GetMessger() {
        return m_messager;
    }

    CShareStatus &GetShareStatus() {
        return m_stat;
    }
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

    // start message pump
    CFramework::GetInstance().pumpMessage();
    // deinit framwork
    CFramework::GetInstance().deinit();
    return 0;
};

#endif //_CFRAMEWORK_CC_

