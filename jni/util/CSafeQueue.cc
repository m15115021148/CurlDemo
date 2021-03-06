/**
 * @file CSafeQueue.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Event, utility tool, to help wait or set event
 * @detail Cubic Event, utility tool, to help wait or set event
 */

#ifndef _CSAFE_QUEUE_CC_
#define _CSAFE_QUEUE_CC_ 1

#include "CLock.cc"
#include "CEvent.cc"
#include <list>

using namespace std;

template<class T>
class CSafeQueue
{
private:
    int m_max_item;
    CEvent m_event;
    CLock m_queue_lock;
    list<T> m_list;

public:
    typedef enum ErrCode {
        ERR_NO_ERROR = 0,
        ERR_OVER_SIZE = -1,
        ERR_NO_ITEM = -2,
        ERR_WAIT_TIME_OUT = -3,
        ERR_NOT_FOUND = -4,
    } EErrCode;

    CSafeQueue() : m_max_item( -1 ) {
    };

    CSafeQueue( int n_max ) : m_max_item( n_max ) {
    };

	T front() {
		CLock::Auto autolock( m_queue_lock );
		T ret = m_list.front();
		return ret;
	};

	bool exist( const T &item ) {
		CLock::Auto autolock( m_queue_lock );
		for( typename list<T>::iterator i = m_list.begin(); i != m_list.end(); i++ ) {
            if( item == *i  ) {
                return true;
            }
        }
		return false;
	};

	bool empty()
	{
		CLock::Auto autolock( m_queue_lock );
		return m_list.empty();
	}

	size_t size()
	{
		CLock::Auto autolock( m_queue_lock );
		return m_list.size();
	}


    EErrCode push( const T &new_item ) {
        CLock::Auto autolock( m_queue_lock );
        int n_current_size = m_list.size();

        if( m_max_item > 0 && n_current_size >= m_max_item ) {
            return ERR_OVER_SIZE;
        }

        m_list.push_back( new_item );

        // if queue is empty before push new item
        // we should set event after new item ready
        if( n_current_size == 0 ) {
            m_event.set();
        }

        return ERR_NO_ERROR;
    };

    EErrCode pop( T &pop_item, int n_milliseconds = 0 ) {
        m_queue_lock.lock();

        if( m_list.empty() ) {
            if( n_milliseconds <= 0 ) {
                m_queue_lock.unlock();
                return ERR_NO_ITEM;
            }
            else {
                m_queue_lock.unlock(); // so that productor can push new item into queue
                CEvent::EErrCode ret = m_event.wait( n_milliseconds );

                if( ret == CEvent::ERR_WAIT_TIMEOUT ) {
                    return ERR_WAIT_TIME_OUT;
                }

                m_queue_lock.lock(); // lock queue

                // check if there is new item
                if( m_list.empty() ) {
                    m_queue_lock.unlock();
                    return ERR_NO_ITEM;
                }
            }
        }

        pop_item = m_list.front();
        m_list.pop_front();

        if( m_list.empty() ) {
            m_event.reset();
        }

        m_queue_lock.unlock();
        return ERR_NO_ERROR;
    };

    void erase( const T &item ) {
        CLock::Auto autolock( m_queue_lock );

        for( typename list<T>::iterator i = m_list.begin(); i != m_list.end(); i++ ) {
            if( item == *i  ) {
                m_list.erase( i );
                break;
            }
        }
    };
};

#endif //_CSAFE_QUEUE_CC_
