/**
 * @file CFramework.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Framework, main frame work of cubic item
 * @detail Cubic Framework, main frame work of cubic item
 */

#ifndef _CSHARE_STATUS_CC_
#define _CSHARE_STATUS_CC_ 1


#include <sys/mman.h> /* for mmap and munmap */
#include <sys/types.h> /* for open */
#include <sys/stat.h> /* for open */
#include <fcntl.h>     /* for open */
#include <unistd.h>    /* for lseek and write */
#include <stdio.h>
//#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>

#include <pthread.h>
#include <signal.h>
#include <iostream>
#include <queue>
#include "CShareStatusTable.h"

#include "cubic_inc.h"
#include "CStringTool.cc"
#include "CEvent.cc"
#include "CLock.cc"
#include "CSafeMap.cc"

using namespace std;

#define USE_SYSTEM_V_SHARE_MEM 1

class CShareStatus
{
public:
#if USE_SYSTEM_V_SHARE_MEM
    static const int SHARE_MEMEORY_ID = 0x12345678;
#else // USE_SYSTEM_V_SHARE_MEM
#define SHARE_MEMEORY_FILE "cubic_share_mem"
#endif // USE_SYSTEM_V_SHARE_MEM

    typedef enum ErrCode {
        ERR_NO_ERROR = 0,
        ERR_OVER_SIZE = -1,
        ERR_NO_ITEM = -2,
        ERR_WAIT_TIME_OUT = -3,
        ERR_NOT_FOUND = -4,
        ERR_NOT_READY = -5,
    } EErrCode;

private:
    typedef struct ShareStatusInfo {
        unsigned int    s_value_offset;
        unsigned int    n_value_size;
        const char**      notify_list;
    } TShareStatusInfo;

    string m_appName;
    unsigned int m_tableSize;
    int m_shareMemId;
    void* m_shareMem;
    CSafeMap<TShareStatusInfo> m_status_map;

private:
    void initStatusTable() {
        m_tableSize = 0;

        for ( TStatusWatchElement* p = s_watch_table; p->s_name != NULL; p++ ) {
            TShareStatusInfo tStatusInfo;
            tStatusInfo.s_value_offset = m_tableSize;
            tStatusInfo.n_value_size = p->n_value_size;
            tStatusInfo.notify_list = p->notify_list;
            m_status_map.setValue( p->s_name, tStatusInfo );
            m_tableSize += p->n_value_size ;
        }
    }

    string getStatus( const string &key ) {
        // get info from table
        TShareStatusInfo tStatusInfo;
        RETNIF( !m_shareMem, "" );

        if( CSafeMap<TShareStatusInfo>::ERR_NO_ERROR !=
            m_status_map.getValue( key, tStatusInfo ) ) {
            return "";
        }

        return string( ( char* )m_shareMem + tStatusInfo.s_value_offset );
    };

    EErrCode setStatus( const string &key, const string &val ) {
        // get info from table
        TShareStatusInfo tStatusInfo;
        RETNIF( !m_shareMem, ERR_NOT_READY );

        if( CSafeMap<TShareStatusInfo>::ERR_NO_ERROR !=
            m_status_map.getValue( key, tStatusInfo ) ) {
            return ERR_NOT_FOUND;
        }

        if( val.length() + 1 > tStatusInfo.n_value_size ) {
            return ERR_OVER_SIZE;
        }

        // save T to memory
        memset( ( char* )m_shareMem + tStatusInfo.s_value_offset,
                0x00,
                tStatusInfo.n_value_size );
        strncpy( ( char* )m_shareMem + tStatusInfo.s_value_offset,
                 val.c_str(),
                 val.length() );
        return ERR_NO_ERROR ;
    };

public:
    CShareStatus( const string &s_appName )
        : m_appName( s_appName )
        , m_tableSize( 0 )
        , m_shareMemId( -1 )
        , m_shareMem( NULL ) {
        // init status table
        initStatusTable();

        if( m_tableSize == 0 ) {
            return;
        }

        // create or get shared memory
#if USE_SYSTEM_V_SHARE_MEM
        //m_shareMemId = shmget( ( key_t )SHARE_MEMEORY_ID, m_tableSize, 0666 | IPC_CREAT );
#else // USE_SYSTEM_V_SHARE_MEM
        m_shareMemId = shm_open( SHARE_MEMEORY_FILE, O_RDWR | O_CREAT, 777 );
#endif // USE_SYSTEM_V_SHARE_MEM

        if ( m_shareMemId < 0 ) {
            perror( "Get Shared Memory" );
            return;
        }

        // map memory to local
#if USE_SYSTEM_V_SHARE_MEM
       // m_shareMem = shmat( m_shareMemId, ( void* )0, 0 );
#else // USE_SYSTEM_V_SHARE_MEM

        if( ftruncate( m_shareMemId, m_tableSize ) < 0 ) {
            perror( "Set Shared Size" );
            return;
        }

        m_shareMem = mmap( NULL, m_tableSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_shareMemId, 0 );
#endif // USE_SYSTEM_V_SHARE_MEM

        if ( m_shareMem == ( void* ) - 1 ) {
            perror( "Map Shared Memory" );
            return;
        }
    };

    ~CShareStatus() {
        // release shared memory
        if( m_shareMem ) {
#if USE_SYSTEM_V_SHARE_MEM
            //shmdt( m_shareMem );
            //shmctl(m_shareMemId, IPC_RMID, 0);
#else // USE_SYSTEM_V_SHARE_MEM
            munmap( m_shareMem, m_tableSize );
            //shm_unlink(m_shareMemId);
#endif // USE_SYSTEM_V_SHARE_MEM
            m_shareMem = NULL;
            m_shareMemId = -1;
        }
    };

    template <typename T>
    const T get( const string &key, const T &def ) {
        string val = getStatus( key );
        RETNIF( val.length() <= 0, def )
        return CStringTool::fromString<T>( val );
    };

    template <typename T>
    EErrCode set( const string &key, const T &val ) {
        return setStatus( key, CStringTool::toString( val ) );
    };
};

#endif //_CSHARE_STATUS_CC_
