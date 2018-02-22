/**
 * @file CLogger.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Logger, to send log info to log service
 * @detail Cubic Logger, to send log info to log service
 */

#ifndef _CLOGGER_CC_
#define _CLOGGER_CC_ 1

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include "CLogService.h"

using namespace std;

class CLogger
{
public:
    typedef enum ErrCode {
        ERR_NO_ERROR = 0,
        ERR_CREATE_FAIL = -1,
        ERR_SET_OPTION_FAIL = -2,
        ERR_BIND_ADDR_FAIL = -3,
        ERR_LEVEL_INVALID = -4,
        ERR_SOCKET_NOT_READY = -5,
        ERR_DATA_SEND_FAIL = -6,
    } EErrCode;

private:
    static const int INVALID_SOCKET = -1;

    int m_level_limit;
    int m_socket;
    char m_app_name[APP_NAME_MAX + 4];

protected:
    EErrCode initSocket() {
        const unsigned int ASYN_TRUE = 1;
        int ret = 0;

        // ------------ create socket --------------
        if( m_socket == INVALID_SOCKET ) {
            m_socket = socket( AF_UNIX, SOCK_DGRAM, 0 );
        }

        if( m_socket == INVALID_SOCKET ) {
            perror( "Create Socket" );
            return ERR_CREATE_FAIL;
        }

        // ------------ set to async ----------------
        ret = ioctl( m_socket, FIONBIO, &ASYN_TRUE );

        if( ret < 0 ) {
            perror( "Set ASYNC Socket" );
            return ERR_SET_OPTION_FAIL;
        }

        return ERR_NO_ERROR;
    };

    void deinitSocket() {
        if( m_socket == INVALID_SOCKET ) {
            return;
        }

        close( m_socket );
        m_socket = INVALID_SOCKET;
    };

    EErrCode sendData( const TLogMessage &data ) {
        int ret = 0;
        struct sockaddr_un t_addr;
        socklen_t t_addr_size = sizeof( struct sockaddr_un );
        // ----------- prepare address -------------
        memset( &t_addr, 0, sizeof( t_addr ) );
        t_addr.sun_family = AF_UNIX;
        strcpy( t_addr.sun_path, LOG_SERVICE_ADDR );
        // ------------ send data ---------------
        ret = sendto( m_socket, ( void* )( &data ), sizeof( TLogMessage ), 0, ( struct sockaddr* )( &t_addr ), t_addr_size );

        if( ret != sizeof( TLogMessage ) ) {
            perror( "Send Data L" );
            return ERR_DATA_SEND_FAIL;
        }

        return ERR_NO_ERROR;
    };

public:
    CLogger( const string &app_name, int n_level_limit = 4 )
        : m_level_limit( n_level_limit )
        , m_socket( INVALID_SOCKET ) {
        memset( m_app_name, 0, APP_NAME_MAX );
        strncpy( m_app_name, app_name.c_str(), MIN( APP_NAME_MAX, app_name.length() ) );
        initSocket();
    };

    ~CLogger() {
        deinitSocket();
    };

    inline void setLevelLimit( int n_level_limit ) {
        m_level_limit = n_level_limit;
    };

    inline int getLevelLimit() {
        return m_level_limit;
    };

    EErrCode logout( int n_level, const char* tag, const char* fmt, ... ) {
        va_list parm_list;
        va_start( parm_list, fmt );
        EErrCode ret = logoutV( n_level, tag, fmt, parm_list );
        va_end( parm_list );
        return ret;
    };

    EErrCode logoutV( int n_level, const char* tag, const char* fmt, va_list parm_list ) {
        TLogMessage msg;
        memset( &msg, 0, sizeof( msg ) );
        msg.n_level = n_level;
        strncpy( msg.str_appname, m_app_name, APP_NAME_MAX );
        strncpy( msg.str_tag, tag, TAG_LEN_MAX );
        vsnprintf( msg.str_data, LOG_MAX_SIZE, fmt, parm_list );
        RETNIF( m_level_limit < n_level, ERR_LEVEL_INVALID );
        fprintf( stderr, "[%d] [%s] [%s] %s\n",
                 msg.n_level,
                 msg.str_appname,
                 msg.str_tag,
                 msg.str_data );
        RETNIF( m_socket == INVALID_SOCKET, ERR_SOCKET_NOT_READY );
        gettimeofday( &( msg.time_stamp ), NULL );
        return sendData( msg );
        //return ERR_NO_ERROR;
    };
};


#endif // _CLOGGER_CC_
