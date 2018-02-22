/**
 * file: CThread.cc
 * auto: thirchina
 * brif: POSIX thread class
 */

#ifndef _C_THREAD_CC_
#define _C_THREAD_CC_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "cubic_inc.h"

class CThread
{
private:
    bool               m_stop_flag;
    pthread_t          m_thread;
    pthread_attr_t     m_thread_attr;
    void*              m_user;
    bool               m_joinable;
    bool               m_running;

protected:
    typedef enum RunRet {
        RUN_CONTINUE,
        RUN_END,
    } RunRet;

    static void* static_run_proc( void* p ) {
        CThread* pthis = ( CThread* )p;
        RETNIF( pthis == NULL, NULL );
        pthis->proc();
        return NULL;
    };

    void proc() {
        RunRet ret = RUN_CONTINUE;
        m_running = true;
        onStart( m_user );

        while( !m_stop_flag && ret == RUN_CONTINUE ) {
            ret = run( m_user );
        }

        onStop( m_user );
        m_running = false;
    }

    inline bool needAbort() {
        return m_stop_flag;
    };

public:
    CThread( bool joinable = true )
        : m_stop_flag( false )
        , m_thread( 0 )
        , m_joinable( joinable )
        , m_running( false )
    {};

    virtual ~CThread() {
        stop();
    };

    inline bool isRunning() {
        return m_running;
    };

    bool start( void* data = NULL ) {
        RETNIF( m_thread != 0, true );
        m_stop_flag = false;
        m_user = data;
        pthread_attr_init( &m_thread_attr );
        pthread_attr_setdetachstate( &m_thread_attr, ( m_joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED ) );
        RETNIF( pthread_create( &m_thread, &m_thread_attr, CThread::static_run_proc, this ), false );
        return true;
    };

    void stop() {
        RETIF( m_thread == 0 );
        m_stop_flag = true;

        if( m_joinable ) { pthread_join( m_thread, NULL ); }

        pthread_attr_destroy( &m_thread_attr );
        m_thread = 0;
    };

    virtual void onStart( void* user ) {
        UNUSED_ARG( user );
    };

    virtual void onStop( void* user ) {
        UNUSED_ARG( user );
    };

    virtual RunRet run( void* user ) = 0;
};


#endif //_C_THREAD_CC_

