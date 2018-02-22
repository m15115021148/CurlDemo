/**
 * @file CCoreWatch.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Logger, to send log info to log service
 * @detail Cubic Logger, to send log info to log service
 */

#ifndef _CCORE_WATCH_CC_
#define _CCORE_WATCH_CC_ 1

#include "CCoreWatch.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>


using namespace std;

class CCoreWatch
{
public:
    typedef enum ErrCode {
        ERR_NO_ERROR = 0,
        ERR_CREATE_FAIL = -1,
        ERR_SET_OPTION_FAIL = -2,
        ERR_SOCKET_NOT_READY = -3,
        ERR_DATA_SEND_FAIL = -4,
    } EErrCode;

private:
    static const int INVALID_SOCKET = -1;
    int m_socket;
    string m_app_name;

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

    EErrCode sendData( const void* p_data, int n_size ) {
        int ret = 0;
        struct sockaddr_un t_addr;
        socklen_t t_addr_size = sizeof( struct sockaddr_un );
        // ----------- prepare address -------------
        memset( &t_addr, 0, sizeof( t_addr ) );
        t_addr.sun_family = AF_UNIX;
        strcpy( t_addr.sun_path, CORE_WATCH_SERV_PATH );
        // ------------ send data ---------------
        ret = sendto( m_socket, p_data, n_size, 0, ( struct sockaddr* )( &t_addr ), t_addr_size );

        if( ret != n_size ) {
            perror( "Send Data W" );
            return ERR_DATA_SEND_FAIL;
        }

        return ERR_NO_ERROR;
    };

public:
    CCoreWatch( const string &app_name )
        : m_socket( INVALID_SOCKET )
        , m_app_name( app_name ) {
        initSocket();
    };

    ~CCoreWatch() {
        deinitSocket();
    };

    void feed() {
        sendData( m_app_name.c_str(), m_app_name.length() );
    };
};


#endif // _CCORE_WATCH_CC_
