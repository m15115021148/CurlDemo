/**
 * @file CSafeMap.cc
 * @author jiaming.lu
 * @version 1.0
 * @brief Cubic Event, utility tool, to help wait or set event
 * @detail Cubic Event, utility tool, to help wait or set event
 */

#ifndef _CSAFE_MAP_CC_
#define _CSAFE_MAP_CC_ 1

#include "CLock.cc"
#include "CEvent.cc"
#include <iostream>
#include <map>

using namespace std;

template<class T>
class CSafeMap
{
private:
    int m_max_item;
    CEvent m_event;
    CLock m_map_lock;
    map<string, T> m_map;

public:
    typedef enum ErrCode {
        ERR_NO_ERROR = 0,
        ERR_OVER_SIZE = -1,
        ERR_NO_ITEM = -2,
        ERR_WAIT_TIME_OUT = -3,
        ERR_NOT_FOUND = -4,
    } EErrCode;

    CSafeMap() : m_max_item( -1 ) {
    };

    CSafeMap( int n_max ) : m_max_item( n_max ) {
    };

    EErrCode getValue( const string &key, T &value ) {
        m_map_lock.lock();
        typename  std::map<string, T>::iterator iter = m_map.find( key );

        if( m_map.end() != iter ) {
            value = iter->second;
            m_map_lock.unlock();
            return ERR_NO_ERROR;
        }

        m_map_lock.unlock();
        return ERR_NOT_FOUND;
    };

    EErrCode setValue( const string &key, const T &value ) {
        m_map_lock.lock();
        m_map.insert( pair<string, T>( key, value ) );
        m_map_lock.unlock();
        return ERR_NO_ERROR;
    };

    EErrCode eraseValue( const string &key ) {
        m_map_lock.lock();
        typename std::map<string, T>::iterator iter = m_map.find( key );

        if( m_map.end() != iter ) {
            m_map.erase( iter );
            m_map_lock.unlock();
            return ERR_NO_ERROR;
        }

        m_map_lock.unlock();
        return ERR_NOT_FOUND;
    };

    void clear() {
        m_map_lock.lock();
        m_map.clear();
        m_map_lock.unlock();
    };

    int size() {
        return m_map.size();
    };

    bool isEmpty() {
        return m_map.empty();
    };

    class IForeach
    {
    public:
        virtual void process( const string &s_key, T &t_item ) = 0;
    };

    void foreach( IForeach* p ) {
        if( !p ) {
            return;
        }

        m_map_lock.lock();
        typename std::map<string, T>::iterator i;

        for( i = m_map.begin(); i != m_map.end(); i++ ) {
            p->process( i->first, i->second );
        }

        m_map_lock.unlock();
    };
};

#endif //_CSAFE_MAP_CC_
