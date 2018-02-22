/**
 * @file CConfig.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Config, help Cubic App read/save config
 * @detail Cubic Config, help Cubic App read/save config
 */

#ifndef _CCONFIG_CC_
#define _CCONFIG_CC_ 1

#include "CFileLock.cc"
#include "CLock.cc"
#include "CUtil.cc"
#include "CStringTool.cc"
#include <iostream>
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cubic_inc.h"
#define LOG_TAG "CConfig"

using namespace std;


class IListenConfig
{
public:
    virtual void onConfigChange( const string &str_key ) = 0;
};

#define CUBIC_CFG_COMMIT_KEY ".commit"

class CConfig
{
private:
    typedef struct ListenItem {
        string str_key;
        int n_watch_descriptor;
        IListenConfig* p_listener;
    } TListenItem;

    const string m_root_path;
    CLock m_watch_listen_tab_lock;
    map<int, TListenItem> m_watch_listen_tab;
    map<string, int> m_key_watch_tab;
    bool mb_listen_stop_flag;
    pthread_t m_listen_thread;
    int m_inotify_fd;
    bool m_protect_ro_item;

    string key2Path( const string &str_key ) {
        string path( m_root_path );
        path += "/";
        path += CStringTool::replaceDist( str_key, ".", "/" );
        return path;
    }

    string Path2key( const string &str_path ) {
        string str_key = str_path.substr( m_root_path.length() + 1 );
        str_key = CStringTool::replaceDist( str_key, "/", "." );
        return str_key;
    }

public:
    typedef enum ErrCode {
        ERR_NO_ERROR = 0,
        ERR_LISTENER_ALREADY_START = -1,
        ERR_CREATE_LISTEN_THREAD_FAIL = -2,
        ERR_MAKE_DIRCTORY_FAIL = -3,
        ERR_OPEN_OR_CREATE_FILE_FAIL = -4,
        ERR_LOCK_FILE_FAIL = -5,
        ERR_INOTIFY_NOT_READY = -6,
        ERR_FILE_DISCRIPTOR_NOT_READY = -7,
        ERR_WAIT_TIMEOUT = -8,
        ERR_WAIT_FAIL = -9,
        ERR_ALREADY_LISTENED = -10,
        ERR_READ_ONLY = -11,
        ERR_PATH_STATE = -12
    } EErrCode;

private:
    EErrCode waitData( int &n_fd, int n_timeout ) {
        int     ret = 0;
        int     maxfd = 0;
        fd_set  fds;
        struct timeval timeout = { n_timeout / 1000, n_timeout % 1000 };

        if( n_fd <= 0 ) {
            return ERR_FILE_DISCRIPTOR_NOT_READY;
        }

        /* init fds */
        FD_ZERO( &fds );
        FD_SET( n_fd, &fds );
        maxfd = n_fd;
        maxfd ++;
        /* select read*/
        ret = select( maxfd, &fds, 0, 0, &timeout );

        switch( ret ) {
        case -1:
            perror( "Select Notify File" );
            return ERR_WAIT_FAIL;

        case 0:
            return ERR_WAIT_TIMEOUT;

        default:
            if( FD_ISSET( n_fd, &fds ) ) {
                return ERR_NO_ERROR;
            }

            break;
        }

        return ERR_WAIT_FAIL;
    }

    void* procListen() {
#define BUFF_LENTH 1024
#define LISTEN_TIMEOUT 1000
        unsigned char buff[BUFF_LENTH] = {0};
        int n_read = 0, n_offset = 0;
        EErrCode ret = ERR_NO_ERROR;
        struct inotify_event* event = NULL;

        while( !mb_listen_stop_flag ) {
            ret = waitData( m_inotify_fd, LISTEN_TIMEOUT );

            if( ret == ERR_WAIT_TIMEOUT ) {
                continue;
            }
            else if( ret != ERR_NO_ERROR ) {
                sleep( 5 );
                continue;
            }

            memset( buff, 0, BUFF_LENTH );
            n_read = read( m_inotify_fd, buff, BUFF_LENTH );
            n_offset = 0;

            while( n_offset < n_read ) {
                event = ( struct inotify_event* )( buff + n_offset );
                m_watch_listen_tab_lock.lock();

                if( m_watch_listen_tab.find( event->wd ) != m_watch_listen_tab.end()
                    && m_watch_listen_tab[event->wd].p_listener ) {
                    m_watch_listen_tab[event->wd].p_listener->onConfigChange(
                        m_watch_listen_tab[event->wd].str_key );
                }

                m_watch_listen_tab_lock.unlock();
                n_offset += offsetof( struct inotify_event, name ) + event->len;
            }
        }

        return NULL;
    }

    static void* static_procListen( void* p ) {
        CConfig* p_this = ( CConfig* )p;
        return p_this->procListen();
    }

    EErrCode startConfigListen() {
        if( m_listen_thread ) {
            return ERR_LISTENER_ALREADY_START;
        }

        mb_listen_stop_flag = false;

        if( 0 != pthread_create( &m_listen_thread, NULL, static_procListen, this ) ) {
            perror( "Create Listen Thread" );
            return ERR_CREATE_LISTEN_THREAD_FAIL;
        }

        return ERR_NO_ERROR;
    }

    void stopListen() {
        if( !m_listen_thread ) {
            return;
        }

        mb_listen_stop_flag = true;
        pthread_join( m_listen_thread, NULL );
        m_listen_thread = 0;
    }

    const string getCfg( const string &str_key ) {
#define CFG_LENTH 1024
        string ret;
        char buf[CFG_LENTH] = {0};
        int n_fd = open( key2Path( str_key ).c_str(), O_RDONLY );
		
        if( n_fd <= 0 ) {
            ret = "";
            return ret;
        }

        memset( buf, 0, CFG_LENTH );
		
        if( read( n_fd, buf, CFG_LENTH ) < 0 )
        { ret += ""; }
        else
        { ret += buf; }

        close( n_fd );

        return ret;
    }

    EErrCode setCfg( const string &str_key, const string &str_val ) {
        // is read only
        if( !str_key.find( "ro." ) ) {
            return ERR_READ_ONLY;
        }

        string path = key2Path( str_key );

        // create parrent dirs
        if( !CUtil::mkdirAndParrent( CUtil::getParrentDirOfPath( path ), S_IRWXU | S_IRWXG | S_IROTH ) ) { // 775
            return ERR_MAKE_DIRCTORY_FAIL;
        }

        // open file
        int n_fd = open( path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH ); // 664

        if( n_fd <= 0 ) {
            return ERR_OPEN_OR_CREATE_FILE_FAIL;
        }

        CFileLock lock( n_fd );

        if( !lock.lock() ) {
            close( n_fd );
            return ERR_LOCK_FILE_FAIL;
        }

        if( write( n_fd, str_val.c_str(), str_val.size() ) != ( int )str_val.size() ) {
            lock.unlock();
            close( n_fd );
            return ERR_WAIT_FAIL;
        }

        lock.unlock();
        close( n_fd );
        return ERR_NO_ERROR;
    }

public:
    CConfig( const string &root_path )
        : m_root_path( root_path )
        , m_watch_listen_tab_lock()
        , m_watch_listen_tab()
        , m_key_watch_tab()
        , mb_listen_stop_flag( false )
        , m_listen_thread( 0 )
        , m_inotify_fd( -1 ) {
        m_inotify_fd = inotify_init();
    }

    ~CConfig() {
        stopListen();
        m_watch_listen_tab_lock.lock();

        for( map<string, int>::iterator it = m_key_watch_tab.begin();
             it != m_key_watch_tab.end(); it++ ) {
            inotify_rm_watch( m_inotify_fd, it->second );
        }

        m_key_watch_tab.clear();
        m_watch_listen_tab.clear();
        m_watch_listen_tab_lock.unlock();
        close( m_inotify_fd );
    }

    template <typename T>
    const T get( const string &str_key, const T &def ) {
        string val = getCfg( str_key );

        if( val.length() <= 0 ) {
            return def;
        }

        return CStringTool::fromString<T>( val );
    }

    template <typename T>
    const T getv( const char* key_fmt, const T &def, ... ) {
        char key[PATH_MAX + 4] = {0};
        va_list parm_list;
        va_start( parm_list, def );
        vsnprintf( key, PATH_MAX, key_fmt, parm_list );
        va_end( parm_list );
        return get( key, def );
    }

    template <class T>
    EErrCode set( const string &str_key, const T &val ) {
        string str_val = CStringTool::toString<T>( val );
        return setCfg( str_key, str_val );
    }

    template <class T>
    EErrCode setv( const char* key_fmt, const T &val, ... ) {
        char key[PATH_MAX + 4] = {0};
        va_list parm_list;
        va_start( parm_list, val );
        vsnprintf( key, PATH_MAX, key_fmt, parm_list );
        va_end( parm_list );
        return set( key, val );
    }

    EErrCode commit( const string &str_key ) {
        string path = key2Path( str_key );
        struct stat buf = {0};

        if( stat( path.c_str(), &buf ) < 0 ) {
            return ERR_PATH_STATE;
        }

        if( S_ISREG( buf.st_mode ) ) {
            int i = path.rfind( '/', path.length() );
            string path_tmp = path.replace( i + 1, path.length() - i - 1, CUBIC_CFG_COMMIT_KEY );
            path.clear();
            path = path_tmp;
        }
        else if( S_ISDIR( buf.st_mode ) ) {
            path += "/";
            path += CUBIC_CFG_COMMIT_KEY;
        }

        int n_fd = open( path.c_str(), O_RDWR ); // 664

        if( n_fd <= 0 ) {
            return ERR_OPEN_OR_CREATE_FILE_FAIL;
        }

        CFileLock lock( n_fd );

        if( !lock.lock() ) {
            close( n_fd );
            return ERR_LOCK_FILE_FAIL;
        }

        char tmp[128] = {0};
        memset( tmp, 0, 128 );

        if( read( n_fd, tmp, 128 ) <= 0 ) {
            lock.unlock();
            close( n_fd );
            return ERR_WAIT_FAIL;
        }

        int data = atoi( tmp ) + 1;
        memset( tmp, 0, 128 );
        sprintf( tmp, "%d", data );
        lseek( n_fd, 0, SEEK_SET );

        if( write( n_fd, tmp, strlen( tmp ) ) != ( int )strlen( tmp ) ) {
            lock.unlock();
            close( n_fd );
            return ERR_WAIT_FAIL;
        }

        lock.unlock();
        close( n_fd );
        return ERR_NO_ERROR;
    }

    EErrCode setListen( const string &str_key, IListenConfig* p_listen ) {
        m_watch_listen_tab_lock.lock();

        // if listen thread not start, start it
        if( m_key_watch_tab.empty() ) {
            EErrCode ret = startConfigListen();

            if( ret != ERR_NO_ERROR ) {
                m_watch_listen_tab_lock.unlock();
                return ret;
            }
        }

        if( m_inotify_fd <= 0 ) {
            m_watch_listen_tab_lock.unlock();
            return ERR_INOTIFY_NOT_READY;
        }

        // if already listened by some one
        if( m_key_watch_tab.find( str_key ) != m_key_watch_tab.end() ) {
            m_watch_listen_tab_lock.unlock();
            return ERR_ALREADY_LISTENED;
        }

        // register new key to listen
        /*use IN_CLOSE_WRITE instead IN_MODIFY, to avoid twice event on one modify*/
        string path = key2Path( str_key );
        struct stat buf = {0};

        if( stat( path.c_str(), &buf ) < 0 ) {
            m_watch_listen_tab_lock.unlock();
            return ERR_PATH_STATE;
        }

        if( S_ISREG( buf.st_mode ) ) {
            int i = path.rfind( '/', path.length() );
            string path_tmp = path.replace( i + 1, path.length() - i - 1, CUBIC_CFG_COMMIT_KEY );
            path.clear();
            path = path_tmp;
        }
        else if( S_ISDIR( buf.st_mode ) ) {
            path += "/";
            path += CUBIC_CFG_COMMIT_KEY;
        }

        int n_watch_descriptor = inotify_add_watch( m_inotify_fd,
                                 path.c_str(),
                                 IN_CLOSE_WRITE | IN_DELETE | IN_DELETE_SELF | IN_CREATE );
        // register to table
        TListenItem item;
        item.str_key = str_key;
        item.n_watch_descriptor = n_watch_descriptor;
        item.p_listener = p_listen;
        m_key_watch_tab[str_key] = n_watch_descriptor;
        m_watch_listen_tab[n_watch_descriptor] = item;
        m_watch_listen_tab_lock.unlock();
        return ERR_NO_ERROR;
    };

    void unsetListen( const string &str_key ) {
        m_watch_listen_tab_lock.lock();

        // if it's not here
        if( m_key_watch_tab.find( str_key ) == m_key_watch_tab.end() ) {
            m_watch_listen_tab_lock.unlock();
            return;
        }

        // unregeister from watch table
        int n_watch_descriptor = m_key_watch_tab[str_key];
        inotify_rm_watch( m_inotify_fd, n_watch_descriptor );
        m_watch_listen_tab.erase( n_watch_descriptor );
        m_key_watch_tab.erase( str_key );

        // if no more listener, stop listen thread
        if( m_key_watch_tab.empty() ) {
            stopListen();
        }

        m_watch_listen_tab_lock.unlock();
    }

    vector<string> searchConfig( const string &str_key ) {
        DIR* p_dir = NULL;
        struct dirent* next = 0;
        vector<string> dir_lst;
        vector<string> result;
        string path;
        dir_lst.push_back( key2Path( str_key ) );

        while( dir_lst.size() > 0 ) {
            path = dir_lst[0];
            p_dir = opendir( path.c_str() );

            if( p_dir ) {
                while( ( next = readdir( p_dir ) ) != NULL ) {
                    if( ( next->d_name[0] == '.' && next->d_name[1] == 0 ) ||
                        ( next->d_name[0] == '.' && next->d_name[1] == '.' && next->d_name[2] == 0 )  ) {
                        continue;
                    }

                    string sub_name = path;
                    sub_name += "/";
                    sub_name += next->d_name;

                    if( next->d_type & DT_DIR ) {
                        dir_lst.push_back( sub_name );
                        continue;
                    }

                    if( sub_name.find( CUBIC_CFG_COMMIT_KEY, 0 ) == string::npos ) {
                        sub_name = Path2key( sub_name );
                        result.push_back( sub_name );
                    }
                }

                closedir( p_dir );
            }

            dir_lst.erase( dir_lst.begin() );
        }

        return result;
    }

    vector<string> enumSub( const string &str_key ) {
        DIR* p_dir = NULL;
        struct dirent* next = 0;
        vector<string> result;
        p_dir = opendir( key2Path( str_key ).c_str() );
        RETNIF( p_dir == NULL, result );

        while( ( next = readdir( p_dir ) ) != NULL ) {
            CONTINUEIF( ( next->d_name[0] == '.' && next->d_name[1] == 0 ) ||
                        ( next->d_name[0] == '.' && next->d_name[1] == '.' && next->d_name[2] == 0 )  );
            CONTINUEIF( strcmp( CUBIC_CFG_COMMIT_KEY, next->d_name ) == 0 );
            result.push_back( string( next->d_name ) );
        }

        closedir( p_dir );
        return result;
    }
};

#endif //_CCONFIG_CC_

