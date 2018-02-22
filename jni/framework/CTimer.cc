/**
 * @file CTimer.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Timer, help Cubic App setup a timer and callback
 * @detail Cubic Timer, help Cubic App setup a timer and callback
 */

#ifndef _CTIMER_CC_
#define _CTIMER_CC_ 1

#include "CLock.cc"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <sys/time.h>



using namespace std;

class ITimer
{
public:
    virtual void onTimer( int n_timer_id ) = 0;
};

class CTimer
{
private:
    typedef struct TimerTask {
        int     id;
        timer_t timer;
        ITimer* callback;
    } TimerTask;
    static int m_next_timer_id;
    static map<int, TimerTask> m_timer_tab;
	static CLock m_lock;

    static void on_timer( sigval_t arg ) {
		CLock::Auto lock(m_lock);
        int id = arg.sival_int;
        RETIF( m_timer_tab.find( id ) == m_timer_tab.end() );
        TimerTask task = m_timer_tab[id];

        if( task.callback != NULL ) {
            task.callback->onTimer( id );
        }

        // if not interval task, just retire it
        struct itimerspec spec;
        timer_gettime( task.timer, &spec );

        if( spec.it_interval.tv_sec == 0 &&
            spec.it_interval.tv_nsec == 0 ) {
            timer_delete( task.timer );
            m_timer_tab.erase( id );
        }
    };

    static int setupTimer( ITimer* callback, uint32_t ms_next, uint32_t ms_interval ) {
		CLock::Auto lock(m_lock);
        struct sigevent sev;
        timer_t timerid;
        sev.sigev_notify = SIGEV_THREAD;
        sev.sigev_signo = SIGRTMIN;
        sev.sigev_value.sival_int = m_next_timer_id;
        sev.sigev_notify_function = on_timer;
        sev.sigev_notify_attributes = NULL;

        if( timer_create( CLOCK_MONOTONIC, &sev, &timerid ) < 0 ) {
            perror( "fail to timer_create" );
            return ( -1 );
        }

        struct itimerspec spec;

        spec.it_value.tv_sec = ms_next / 1000;

        spec.it_value.tv_nsec = ( ms_next % 1000 ) * 1000000;

        spec.it_interval.tv_sec = ms_interval / 1000;

        spec.it_interval.tv_nsec = ( ms_interval % 1000 ) * 1000000;

        if( timer_settime( timerid, 0, &spec, NULL ) < 0 ) {
            perror( "fail to timer_settime" );
            return ( -2 );
        }

        TimerTask task;
        task.id = m_next_timer_id;
        task.timer = timerid;
        task.callback = callback;
        m_timer_tab[m_next_timer_id] = task;
        return m_next_timer_id++;
    };


public:
    static int setTimer( uint32_t n_milliseconds, ITimer* p_timer ) {
        return setupTimer( p_timer, n_milliseconds, 0 );
    };

    static int setTimerInterval( uint32_t n_milliseconds, ITimer* p_timer ) {
        return setupTimer( p_timer, n_milliseconds, n_milliseconds );
    };

    static void killTimer( int n_timer_id ) {
		CLock::Auto lock(m_lock);
        RETIF( m_timer_tab.find( n_timer_id ) == m_timer_tab.end() );
        timer_delete( m_timer_tab[n_timer_id].timer );
        m_timer_tab.erase( n_timer_id );
    };
};

int CTimer::m_next_timer_id = 0;
map<int, CTimer::TimerTask> CTimer::m_timer_tab;
CLock CTimer::m_lock;

#endif //_CTIMER_CC_
