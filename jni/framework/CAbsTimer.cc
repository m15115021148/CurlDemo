/**
 * @file CAbsTimer.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Timer, help Cubic App setup a timer and callback
 * @detail Cubic Timer, help Cubic App setup a timer and callback
 */

#ifndef _CABSTIMER_CC_
#define _CABSTIMER_CC_ 1

#include "CLock.cc"
#include "CEvent.cc"
#include "CThread.cc"
#include "CAsyncRun.cc"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/time.h>


using namespace std;

class IAbsTimer
{
public:
    virtual void onAbsTimer(int n_timer_id) = 0;
};


class CAbsTimer : public CThread
{
private:
    typedef enum TaskType
    {
        TYPE_TIMEOUT,
        TYPE_INTERVAL,
    } TaskType;

    typedef struct CTask
    {
        int         id;
        TaskType    type;
        uint32_t    time;
        IAbsTimer*  timer;
        uint32_t    left;
    } CTask;

    static const int TIME_PRECISION = 100; //ms
    vector<CTask>  m_timer_tab;
    CLock          m_timer_tab_lock;
	CEvent         m_wait_evt;
	uint32_t       m_wait_time;
    int            m_next_timer_id;
	struct timeval m_last_run_time;


    static void notify_timer( CTask task )
    {
        if( task.timer != NULL )
        {
            task.timer->onAbsTimer(task.id);
        }
    };

    int setupTimer( uint32_t time,  IAbsTimer *timer, TaskType type )
    {
        CLock::Auto lock( m_timer_tab_lock );
        CTask task;

        task.id    = m_next_timer_id;
        task.type  = type;
        task.time  = time;
        task.timer = timer;
        task.left  = time;
        m_timer_tab.push_back(task);

        // start timer if first task added
        if( m_timer_tab.size() == 1 )
        {
			m_wait_time = time;
            this->start();
        };
		m_wait_evt.set();
        return m_next_timer_id++;
    };

	void removeTimer(int n_timer_id)
    {
        CLock::Auto lock( m_timer_tab_lock );

        for( vector<CTask>::iterator i = m_timer_tab.begin(); i != m_timer_tab.end(); i++ )
        {
            if( (*i).id == n_timer_id )
            {
                m_timer_tab.erase(i);
                break;
            }
        }

		if( m_timer_tab.size() <= 0 ) {
			this->stop();
		}
    };

	CAbsTimer()
		: m_timer_tab()
		, m_timer_tab_lock()
		, m_wait_evt()
		, m_wait_time(1)
		, m_next_timer_id(0)
		, m_last_run_time()
	{};

public:
    ~CAbsTimer()
    {
        this->stop();
        m_timer_tab.clear();
    };

	static CAbsTimer& getInstance() {
		static CAbsTimer s_timer;
		return s_timer;
	}

    static int setTimer(uint32_t n_milliseconds, IAbsTimer *p_timer)
    {
        return getInstance().setupTimer(n_milliseconds, p_timer, TYPE_TIMEOUT);
    };

    static int setTimerInterval(uint32_t n_milliseconds, IAbsTimer *p_timer)
    {
        return getInstance().setupTimer(n_milliseconds, p_timer, TYPE_INTERVAL);
    };

    static void killTimer(int n_timer_id)
    {
        getInstance().removeTimer(n_timer_id);
    };

    virtual RunRet run(void* user)
    {
		m_wait_evt.wait(m_wait_time);

		// calculate passed time
		struct timeval now;
		uint32_t time_passed = 0;
		gettimeofday( &now, NULL );
		if( m_last_run_time.tv_sec == 0 && m_last_run_time.tv_usec == 0 ) {
			m_last_run_time = now;
			return RUN_CONTINUE;
		}
		time_passed = ((now.tv_sec - m_last_run_time.tv_sec) * 1000) +
					  ((now.tv_usec - m_last_run_time.tv_usec) / 1000);
		m_last_run_time = now;


        // process current timer
		CLock::Auto lock( m_timer_tab_lock );
		m_wait_time = TIME_PRECISION;
        for( vector<CTask>::iterator i = m_timer_tab.begin(); i != m_timer_tab.end();  )
        {
            if( (*i).left <= time_passed )
            {
                if( (*i).timer != NULL )
                {
                    CAsyncRun<CTask>::async_run( notify_timer, (*i) );
                }

                if( (*i).type == TYPE_INTERVAL )
                {
                    (*i).left = (*i).time;
					if( m_wait_time > (*i).left ) {
						m_wait_time = (*i).left;
					}
                    i++;
                }
                else
                {
                    i = m_timer_tab.erase(i);
                }
            }
            else
            {
                (*i).left -= time_passed;
				if( m_wait_time > (*i).left ) {
					m_wait_time = (*i).left;
				}
                i++;
            }
        }

        return RUN_CONTINUE;
    };
};


#endif //_CABSTIMER_CC_
