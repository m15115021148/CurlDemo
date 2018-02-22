/**
 * @file CMessager.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Messager, help Cubic App communicate with each other
 * @detail Cubic Messager, help Cubic App communicate with each other
 */

#ifndef _CMESSAGER_CC_
#define _CMESSAGER_CC_ 1

#include "CLock.cc"
#include "CEvent.cc"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/input.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <iostream>
#include <map>

using namespace std;

class IMsgListener
{
public:
    virtual void onMessage( int n_session_id, const string &str_src_app_name, int n_msg_id, const void* p_data ) = 0;
};


class CMessager
{
public:
    static const unsigned int MAX_MESSAGE_SIZE = 1536;
    static const unsigned int MAX_RESULT_SIZE = 128;
    static const int INVALID_SOCKET = -1;
    static const unsigned int RECV_TIMEOUT = 1000; // ms
    static const unsigned int PROCESS_TIMEOUT = 10000; // ms
private:
    typedef enum MsgType {
        MSG_REQ, // this message is send as request
        MSG_ACK, // this message is send as acknowledge for a recived request or response
        MSG_RSP, // this message is send as response, for a finished request
    } EMsgType;

    typedef struct Message {
        int             n_session_id; // random number to identify each session
        EMsgType        e_msg_type;   // type of message
        int             n_id;         // message id to identify message command
        int             n_data_size;  // size of message data
        unsigned char   auc_data[MAX_MESSAGE_SIZE]; // payload for massage data
    } TMessage;

    class RspWaiter
    {
    private:
        CEvent m_wake_event;
        string m_src_app;
        TMessage m_message;

    public:
        inline CEvent::EErrCode wait( unsigned int n_ms ) {
            return m_wake_event.wait( n_ms );
        };

        inline void wake( const string &str_app, const TMessage &t_message ) {
            m_src_app = str_app;
            m_message = t_message;
            m_wake_event.set();
        };

        inline const string &getSrcApp() {
            return m_src_app;
        };

        inline const TMessage &getMessage() {
            return m_message;
        };
    };

    string m_app_name;
    CLock m_clnt_lock;
    int m_clnt_sock;
    int m_serv_sock;

    bool mb_listen_stop_flag;
    pthread_t m_listen_thread;
    IMsgListener* mp_listener;

    CLock m_resp_wait_tab_lock;
    map<int, RspWaiter*> m_resp_wait_tab;

public:
    typedef enum ErrCode {
        ERR_NO_ERROR = 0,
        ERR_CREATE_FAIL = -1,
        ERR_SET_OPTION_FAIL = -2,
        ERR_BIND_ADDR_FAIL = -3,
        ERR_MESSAGE_DATA_TOOL_LARGE = -4,
        ERR_DATA_SEND_FAIL = -5,
        ERR_DATA_RECV_FAIL = -6,
        ERR_TIMEOUT = -7,
        ERR_WAIT_SOCKET_FAIL = -8,
        ERR_ACK_NOT_MATCH = -9,
        ERR_RECIVER_POINTER_IS_EMPTY = -10,
        ERR_LISTENER_ALREADY_START = -11,
        ERR_CREATE_LISTEN_THREAD_FAIL = -12,
        ERR_NOT_FOUND = -13,
        ERR_DATA_SIZE_NOT_MATCH = -14,
        ERR_SOCKET_NOT_READY = -15,
        ERR_ITEM_ALREADY_EXIST = -16,
        ERR_RSP_APP_NOT_MATCH = -17,
        ERR_WAIT_RESP_FAIL = -18,
    } EErrCode;

protected:
    inline string getClntAddr( const string &str_app_name ) {
        string str_addr = CUBIC_MSG_SOCK_DIR;
        str_addr += str_app_name;
        str_addr += ".clnt.sock";
        return str_addr;
    };

    inline string getServAddr( const string &str_app_name ) {
        string str_addr = CUBIC_MSG_SOCK_DIR;
        str_addr += str_app_name;
        str_addr += ".serv.sock";
        return str_addr;
    };

    inline string getAppNameOfAddr( const string &str_addr ) {
        string str_app_name = str_addr.substr( 5, str_addr.length() - 15 );
        return str_app_name;
    };

    EErrCode initSocket( int &n_socket, const string &str_sock_addr ) {
        const unsigned int ASYN_TRUE = 1;
        int ret = 0;

        // ------------ create socket --------------
        if( n_socket == INVALID_SOCKET ) {
            n_socket = socket( AF_UNIX, SOCK_DGRAM, 0 );
        }

        if( n_socket == INVALID_SOCKET ) {
            perror( "Create Socket" );
            return ERR_CREATE_FAIL;
        }

        // ------------ set to async ----------------
        ret = ioctl( n_socket, FIONBIO, &ASYN_TRUE );

        if( ret < 0 ) {
            perror( "Set ASYNC Socket" );
            return ERR_SET_OPTION_FAIL;
        }

        // ------------ bind address ----------------
        struct sockaddr_un t_addr;
        t_addr.sun_family = AF_UNIX;
        strncpy( t_addr.sun_path, str_sock_addr.c_str(), sizeof( t_addr.sun_path ) );
        unlink( str_sock_addr.c_str() );
        ret = bind( n_socket, ( struct sockaddr* )( &t_addr ), ( socklen_t )sizeof( struct sockaddr_un ) );

        if( ret < 0 ) {
            perror( "Bind Address" );
            return ERR_BIND_ADDR_FAIL;
        }

        return ERR_NO_ERROR;
    };

    void deinitSocket( int &n_socket, const string &str_sock_addr ) {
        if( n_socket == INVALID_SOCKET ) {
            return;
        }

        close( n_socket );
        n_socket = INVALID_SOCKET;
        unlink( str_sock_addr.c_str() );
    }

    EErrCode sendData( int &n_socket, const string &str_dst_sock_addr, const TMessage &data ) {
        int ret = 0;
        struct sockaddr_un t_addr;
        socklen_t t_addr_size = sizeof( struct sockaddr_un );
        // ----------- prepare address -------------
        memset( &t_addr, 0, sizeof( t_addr ) );
        t_addr.sun_family = AF_UNIX;
        strcpy( t_addr.sun_path, str_dst_sock_addr.c_str() );
        // ------------ send data ---------------
        ret = sendto( n_socket, ( void* )( &data ), sizeof( TMessage ), 0, ( struct sockaddr* )( &t_addr ), t_addr_size );

        if( ret != sizeof( TMessage ) ) {
            perror( "Send Data M" );
            return ERR_DATA_SEND_FAIL;
        }

        return ERR_NO_ERROR;
    };

    EErrCode recvData( int &n_socket, string &str_src_sock_addr, const TMessage &data ) {
        int ret = 0;
        struct sockaddr_un t_addr;
        socklen_t t_addr_size = sizeof( struct sockaddr_un );
        memset( &t_addr, 0, sizeof( t_addr ) );
        ret = recvfrom( n_socket, ( void* )( &data ), sizeof( TMessage ), 0, ( struct sockaddr* )( &t_addr ), &t_addr_size );

        if( ret != sizeof( TMessage ) ) {
            perror( "Recv Data" );
            return ERR_DATA_RECV_FAIL;
        }

        str_src_sock_addr = t_addr.sun_path;
        return ERR_NO_ERROR;
    };

    EErrCode waitData( int &n_socket, int n_timeout ) {
        int     ret = 0;
        int     maxfd = 0;
        fd_set  fds;
        struct timeval timeout = { n_timeout / 1000, n_timeout % 1000 };
        memset( &fds, 0, sizeof( fd_set ) );

        if( n_socket <= 0 ) {
            return ERR_SOCKET_NOT_READY;
        }

        /* init fds */
        FD_ZERO( &fds );
        FD_SET( n_socket, &fds );
        maxfd = n_socket;
        maxfd ++;
        /* select read*/
        ret = select( maxfd, &fds, 0, 0, &timeout );

        switch( ret ) {
        case -1:
            perror( "Select Socket" );
            return ERR_WAIT_SOCKET_FAIL;

        case 0:
            return ERR_TIMEOUT;

        default:
            if( FD_ISSET( n_socket, &fds ) ) {
                return ERR_NO_ERROR;
            }

            break;
        }

        return ERR_WAIT_SOCKET_FAIL;
    };


    template<class T>
    EErrCode sendMessage( const string &str_dst_app_name, int n_msg_id, const T &data, EMsgType e_msg_type, int n_session ) {
        EErrCode ret = ERR_NO_ERROR;

        if( sizeof( T ) > MAX_MESSAGE_SIZE ) {
            return ERR_MESSAGE_DATA_TOOL_LARGE;
        }

        m_clnt_lock.lock();
        // -------------- fill data ------------------
        TMessage t_message;
        memset( &t_message, 0, sizeof( TMessage ) );
        t_message.n_session_id = n_session;
        t_message.e_msg_type = e_msg_type;
        t_message.n_id = n_msg_id;
        t_message.n_data_size = sizeof( T );
        memcpy( &( t_message.auc_data ), ( unsigned char* )( &data ), sizeof( T ) );
        // -------------- send data ------------------
        ret = sendData( m_clnt_sock, getServAddr( str_dst_app_name ), t_message );

        if( ERR_NO_ERROR != ret ) {
            m_clnt_lock.unlock();
            return ret;
        }

        // ------------- wait ack --------------------
        ret = waitData( m_clnt_sock, RECV_TIMEOUT );

        if( ret != ERR_NO_ERROR ) {
            m_clnt_lock.unlock();
            return ret;
        }

        // ------------- recv ack --------------------
        TMessage t_message_recv;
        string src_sock_addr;
        memset( &t_message_recv, 0, sizeof( TMessage ) );
        ret = recvData( m_clnt_sock, src_sock_addr, t_message_recv );
        m_clnt_lock.unlock();

        if( ERR_NO_ERROR != ret ) {
            return ret;
        }

        if( t_message_recv.n_session_id != t_message.n_session_id // check if session id
            || t_message_recv.e_msg_type != MSG_ACK // check message type
            || t_message_recv.n_id != t_message.n_id // check message id
            || t_message_recv.n_data_size != 0 // check data size
          ) {
            return ERR_ACK_NOT_MATCH;
        }

        return ERR_NO_ERROR;
    };

    EErrCode registerWaiter( int n_session_id, RspWaiter* waiter ) {
        m_resp_wait_tab_lock.lock();

        if( m_resp_wait_tab.end() != m_resp_wait_tab.find( n_session_id ) ) {
            m_resp_wait_tab_lock.unlock();
            return ERR_ITEM_ALREADY_EXIST;
        }

        m_resp_wait_tab[n_session_id] = waiter;
        m_resp_wait_tab_lock.unlock();
        return ERR_NO_ERROR;
    };

    void unregisterWaiter( int n_session_id ) {
        m_resp_wait_tab_lock.lock();

        if( m_resp_wait_tab.end() != m_resp_wait_tab.find( n_session_id ) ) {
            m_resp_wait_tab.erase( n_session_id );
        }

        m_resp_wait_tab_lock.unlock();
    };

    void onResponse( int n_session_id, const string &str_app_name, const TMessage &t_message ) {
        m_resp_wait_tab_lock.lock();

        if( m_resp_wait_tab.end() != m_resp_wait_tab.find( n_session_id ) ) {
            m_resp_wait_tab[n_session_id]->wake( str_app_name, t_message );
            m_resp_wait_tab.erase( n_session_id );
        }

        m_resp_wait_tab_lock.unlock();
    };

    void* procListen() {
        EErrCode ret = ERR_NO_ERROR;
        TMessage t_message_recv;
        TMessage t_message_ack;
        string src_sock_addr;

        while( !mb_listen_stop_flag ) {
            // ------------- wait message --------------------
            ret = waitData( m_serv_sock, RECV_TIMEOUT );

            if( ret == ERR_TIMEOUT ) {
                continue;
            }
            else if( ret != ERR_NO_ERROR ) {
                break;
            }

            // ------------- recieve ---------------------
            memset( &t_message_recv, 0, sizeof( TMessage ) );
            ret = recvData( m_serv_sock, src_sock_addr, t_message_recv );

            if( ERR_NO_ERROR != ret ) {
                continue;
            }

            // -------------- send ack ------------------
            memset( &t_message_ack, 0, sizeof( TMessage ) );
            t_message_ack.n_session_id = t_message_recv.n_session_id;
            t_message_ack.n_id = t_message_recv.n_id;
            t_message_ack.e_msg_type = MSG_ACK;
            ret = sendData( m_serv_sock, src_sock_addr, t_message_ack );

            if( ERR_NO_ERROR != ret ) {
                m_clnt_lock.unlock();
                continue;
            }

            // ------------- handle ---------------------
            switch( t_message_recv.e_msg_type ) {
            case MSG_REQ:

                // should handle by listener
                if( mp_listener ) {
                    mp_listener->onMessage(
                        t_message_recv.n_session_id,
                        getAppNameOfAddr( src_sock_addr ),
                        t_message_recv.n_id,
                        ( void* )t_message_recv.auc_data );
                }

                break;

            case MSG_RSP:
                onResponse( t_message_recv.n_session_id, getAppNameOfAddr( src_sock_addr ), t_message_recv );
                break;

            case MSG_ACK:
            default:
                // do nothing;
                break;
            }
        }

        return NULL;
    };

    static void* static_procListen( void* p_msger ) {
        CMessager* p_this = ( CMessager* )p_msger;

        if( p_this ) {
            return p_this->procListen();
        }

        return NULL;
    }

public:
    /**
     * construtor of CMessager
     * @author thirchina
     * @param str_app_name:set application name, will use as socket name
     */
    CMessager( const string &str_app_name )
        : m_app_name( str_app_name )
        , m_clnt_lock()
        , m_clnt_sock( INVALID_SOCKET )
        , m_serv_sock( INVALID_SOCKET )
        , mb_listen_stop_flag( false )
        , m_listen_thread()
        , mp_listener( NULL )
        , m_resp_wait_tab_lock()
        , m_resp_wait_tab() {
        // ------------- init socket ------------
        if( initSocket( m_clnt_sock, getClntAddr( str_app_name ) ) != ERR_NO_ERROR ||
            initSocket( m_serv_sock, getServAddr( str_app_name ) ) != ERR_NO_ERROR ) {
            deinitSocket( m_clnt_sock, getClntAddr( str_app_name ) );
            deinitSocket( m_serv_sock, getServAddr( str_app_name ) );
        }

        // ------------- setup random ------------
        srand( time( 0 ) );
    };

    /**
     * deconstruct of CMessager
     * @author thirchina
     */
    ~CMessager() {
        stopListen();
        deinitSocket( m_clnt_sock, getClntAddr( m_app_name ) );
        deinitSocket( m_serv_sock, getServAddr( m_app_name ) );
    };

    template<class T>
    EErrCode postRequest( const string &str_dst_app_name, int n_msg_id, const T &data ) {
        return sendMessage( str_dst_app_name, n_msg_id, data, MSG_REQ, rand() );
    };

    template<class T>
    inline EErrCode response( int n_session_id, const string &str_dst_app_name, int n_msg_id, const T &data ) {
        return sendMessage( str_dst_app_name, n_msg_id, data, MSG_RSP, n_session_id );
    };

    template<class TSend, class TRecv>
    EErrCode sendRequest( const string &str_dst_app_name, int n_msg_id, const TSend &data, TRecv &data_recv ) {
        EErrCode ret = ERR_NO_ERROR;
        CEvent evt( true, false );
        RspWaiter waiter;
        int n_session_id = rand();
        // register to waiting list
        ret = registerWaiter( n_session_id, &waiter );

        if( ret != ERR_NO_ERROR ) {
            return ret;
        }

        // send request
        ret = sendMessage( str_dst_app_name, n_msg_id, data, MSG_REQ, n_session_id );

        if( ret != ERR_NO_ERROR ) {
            return ret;
        }

        // wait for response
        CEvent::EErrCode err = waiter.wait( PROCESS_TIMEOUT );
        unregisterWaiter( n_session_id );

        if( err != CEvent::ERR_NO_ERROR ) {
            return ERR_WAIT_RESP_FAIL;
        }

        // check response app name
        if( waiter.getSrcApp() != str_dst_app_name ) {
            return ERR_RSP_APP_NOT_MATCH;
        }

        // get response data
        if( sizeof( TRecv ) != waiter.getMessage().n_data_size ) {
            return ERR_DATA_SIZE_NOT_MATCH;
        }

        memcpy( ( void* )( &data_recv ), waiter.getMessage().auc_data, waiter.getMessage().n_data_size );
        return ERR_NO_ERROR;
    };

    EErrCode startListen( IMsgListener* p_listener ) {
        if( !p_listener ) {
            return ERR_RECIVER_POINTER_IS_EMPTY;
        }

        if( m_listen_thread ) {
            return ERR_LISTENER_ALREADY_START;
        }

        mb_listen_stop_flag = false;

        if( 0 != pthread_create( &m_listen_thread, NULL, CMessager::static_procListen, this ) ) {
            perror( "Create Listen Thread" );
            return ERR_CREATE_LISTEN_THREAD_FAIL;
        }

        mp_listener = p_listener;
        return ERR_NO_ERROR;
    };

    void stopListen() {
        if( !m_listen_thread ) {
            return;
        }

        mb_listen_stop_flag = true;
        pthread_join( m_listen_thread, NULL );
        m_listen_thread = 0;
    };

};

#endif // _CMESSAGER_CC_
