/**
 * @file CUtil.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Util, utility methods of Cubic
 * @detail Cubic Util, utility methods of Cubic
 */

#ifndef _CUTIL_CC_
#define _CUTIL_CC_ 1

#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <cctype>
#include <ctime>

extern "C" {

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <syscall.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/statfs.h>


#include <linux/reboot.h>
#include <linux/input.h>
#include <linux/rtc.h>
#include <openssl/md5.h>

};

#include "cubic_inc.h"

using   namespace   std;


#define UTIL_SHUTDOWN_TIMEO    15
class CUtil
{
public:
    static int ReadFile( const char* path, char* data, int length ) {
        FILE* p = NULL;
        int num = 0;
        p = fopen( path, "r" );

        if( p == NULL ) {
            return -1;
        }

        num = fread( data, 1, length, p );

        if( num <= 0 ) {
            fclose( p );
            return -2;
        }

        fclose( p );
        return num;
    }

    static int WriteFile( const char* path, const char* data, unsigned int length ) {
        if( path == NULL || data == NULL || length == 0 ) {
            return 0;
        }

        FILE* p = NULL;
        p = fopen( path, "w" );

        if( p == NULL ) {
            fclose( p );
            return -1;
        }

        if( fwrite( data, 1, length, p ) != length ) {
            fclose( p );
            return -2;
        }
        else {
            fclose( p );
            return length;
        }
    }

    static int readLine( int n_fd, char* p_buff, unsigned int n_size ) {
        unsigned char   ch = 0;
        int             n_read = 0;

        while( n_read < ( int )n_size && read( n_fd, &ch, 1 ) > 0 ) {
            if( ch == '\r' || ch == '\n' ) {
                if( n_read == 0 ) {
                    continue;
                }
                else {
                    break;
                }
            }
            else {
                *p_buff++ = ch;
                n_read++;
            }
        };

        *p_buff = 0;

        return n_read;
    };

    static int getline( int n_fd, int n_line, unsigned char* p_buff, unsigned int n_size ) {
        unsigned char   ch = 0;
        int             n = 0;
        int             n_read = 0;

        while( n_read < ( int )n_size && read( n_fd, &ch, 1 ) > 0 ) {
            if( ch == '\r' || ch == '\n' ) {
                n++;

                if( n <= n_line ) {
                    continue;
                }
                else {
                    break;
                }
            }
            else if( n == n_line ) {
                *p_buff++ = ch;
                n_read++;
            }
        };

        *p_buff = 0;

        return n_read;
    };

    static int isDigitStr( const char* str ) {
        for( const char* p = str; *p != 0; p++ ) {
            if( !isdigit( *p ) ) {
                return -1;
            }
        };

        return 1;
    };

public:
    static string getFileNameOfPath( const string &str ) {
        string ret;
        string::size_type pos( 0 );
        pos = str.rfind( '/' );

        if( pos == string::npos ) {
            ret = str;
        }

        ret = str.substr( pos + 1 );
        return ret;
    };

    static string getParrentDirOfPath( const string &str ) {
        string ret;
        string::size_type pos( 0 );
        pos = str.rfind( '/' );

        if( pos == string::npos ) {
            ret = str;
        }

        ret = str.substr( 0, pos );
        return ret;
    };

    static bool mkdirAndParrent( const string &str_path, int n_mod ) {
        string::size_type pos( 0 );
        int ret = 0;
        string str_tmp;
        struct stat file_stat;

        while( pos != string::npos ) {
            pos = str_path.find( '/', pos + 1 );
            str_tmp = ( pos != string::npos ) ? str_path.substr( 0, pos + 1 ) : str_path;
            ret = stat( str_tmp.c_str(), &file_stat );

            if( ret < 0 ) { // not exist, create it
                ret = mkdir( str_tmp.c_str(), n_mod );
            }
            else if( ( file_stat.st_mode & S_IFDIR ) != 0 ) { // already there
                ret = 0;
            }
            else { // failed
                ret = -1;
            }

            if( ret < 0 ) {
                return false;
            }
        };

        return true;
    };

    static bool removeDir( const char* dirname ) {
        DIR* dir;
        struct dirent* entry;
        char path[PATH_MAX];
        dir = opendir( dirname );

        if ( dir == NULL ) {
            perror( "open dir failed" );
            return false;
        }

        while ( ( entry = readdir( dir ) ) != NULL ) {
            if( ( entry->d_name[0] == '.' && entry->d_name[1] == 0 ) ||
                ( entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == 0 ) ) {
                continue;
            }

            snprintf( path, ( size_t ) PATH_MAX, "%s/%s", dirname, entry->d_name );

            if ( entry->d_type == DT_DIR ) {
                removeDir( path );
            }
            else {
                // delete file
                unlink( path );
            }
        }

        closedir( dir );
        // now we can delete the empty dir
        rmdir( dirname );
        return true;
    }


    /**
      * The function can execute the system call
      * @author tiangang
      * @date 2014-11-20
      * @param s_cmd the command to be executed pointer
      * @param s_result the command execute result buff pointer
      * @param n_size the command execute result buff size
      * @return -1:error,>0 correct
      * @brief execute the system call
     */
    static int syscall( const char* s_cmd, char* s_result = NULL, unsigned int n_size = 0 ) {
        int     n_read = 0;
        FILE*    p_stream = 0;
        char buf[1024] = {0};

        if( s_cmd == NULL ) {
            cerr << "system call cmd error" << endl;
            return -1;
        }
        else {
            p_stream = popen( s_cmd, "r" );

            if( p_stream == NULL ) {
                perror( "popen for command" );
                return -1;
            }

            if( s_result != 0 && n_size > 0 ) {
                n_read = fread( s_result, n_size, 1, p_stream );
            }
            else {
                while ( fgets( buf, 1024, p_stream ) != NULL ) {
                    printf( "%s", buf );
                }
            }

            pclose( p_stream );
        }

        return n_read;
    }

    static int syscommand( const char* cmdfmt, ... ) {
        va_list parm_list;
        va_start( parm_list, cmdfmt );
        int ret = syscommandv( cmdfmt, parm_list );
        va_end( parm_list );
        return ret;
    };

    static int syscommandv( const char* fmt, va_list vl ) {
        static const int COMMAND_MAX = 1024;
        char command[COMMAND_MAX + 4];
        vsnprintf( command, COMMAND_MAX, fmt, vl );
        return syscall( command );
    };

    static void kill_with_pid( pid_t app_pid, int timeo ) {
        kill( app_pid, SIGTERM );

        if( waitPid( app_pid, timeo ) > 0 ) {
            kill( app_pid, SIGKILL );
            sleep( 1 );
        }
    };

    static void kill_with_name( const char* name, int timeo ) {
        pid_t app_pid = 0;

        // ==================== kill CoreApp with pid =====================
        while( ( app_pid = findPidByName( name ) ) > 0 ) {
            kill( app_pid, SIGTERM );

            if( waitPid( app_pid, timeo ) > 0 ) {
                kill( app_pid, SIGKILL );
                sleep( 1 );
            }
        }
    };

    static pid_t execel_as_child( const string &name ) {
        pid_t pid = 0;
        pid = fork();

        if( pid < 0 ) {
            perror( "fork first" );
            return pid;
        }

        if( pid == 0 ) {
            execl( name.c_str(), name.c_str(), ( char* )NULL  );
            perror( "execel_as_child" );
            exit( 0 );
        }

        return pid;
    }

    static int execel( const char* name ) {
        pid_t pid = 0;
        // ==================== double fork to execl CoreApp =====================
        pid = fork();

        if( pid < 0 ) {
            perror( "fork first" );
        }
        else if( pid == 0 ) {
            pid_t pid_grand = fork();

            if( pid_grand < 0 ) {
                perror( "fork second" );
            }
            else if( pid_grand == 0 ) {
                execl( name, name, ( char* )NULL  );
            }
            else {
                // first child exit
                exit( 0 );
            }
        }
        else {
            // wait for first child
            waitpid( pid, 0, 0 );
            sleep( 5 );
        }
		return pid;
    };

    static int findPidByName( const char* p_targname ) {
#define PROC_STATS_LINE_MAX     260
        const char* S_KEY = "Name:";
        const char* S_FMT = "/proc/%s/status";
        int        n_keylen = strlen( S_KEY );
        int     n_read = 0;
        int        n_fd = -1;
        int     n_pid = 0;
        DIR*    p_dir = 0;
        struct dirent* next = 0;
        char     s_file[PROC_STATS_LINE_MAX + 4] = {0};
        char     s_buff[PROC_STATS_LINE_MAX + 4] = {0};
        char*     p_name = s_buff + n_keylen + 1; // skip Name:\t
        p_dir = opendir( "/proc" );

        if( p_dir == 0 ) {
            return 0;
        }

        while( n_pid == 0 && ( next = readdir( p_dir ) ) != NULL ) {
            if( ( next->d_name[0] == '.' && next->d_name[1] == 0 ) ||
                ( next->d_name[0] == '.' && next->d_name[1] == '.' && next->d_name[2] == 0 ) ||
                ( next->d_type != DT_DIR ) ||
                ( isDigitStr( next->d_name ) < 0 ) ) {
                continue;
            }

            sprintf( s_file, S_FMT, next->d_name );
            n_fd = open( s_file, O_RDONLY );

            if( n_fd < 0 ) {
                continue;
            }

            while( ( n_read = readLine( n_fd, s_buff, PROC_STATS_LINE_MAX ) ) > 0 ) {
                if( strncmp( s_buff, S_KEY, n_keylen ) == 0 &&
                    strcmp( p_name, p_targname ) == 0 ) {
                    n_pid = atoi( next->d_name );
                    break;
                }
            }

            close( n_fd );
            n_fd = -1;
        }

        closedir( p_dir );
        return n_pid;
    };

    static int waitPid( pid_t pid, int timeo ) {
        int     n_ret = 0;
        DIR*    p_dir = 0;
        struct dirent* next = 0;

        do {
            p_dir = opendir( "/proc" );

            if( p_dir == 0 ) {
                return -1;
            }

            n_ret = 0;

            while( ( next = readdir( p_dir ) ) != NULL ) {
                if( ( next->d_name[0] == '.' && next->d_name[1] == 0 ) ||
                    ( next->d_name[0] == '.' && next->d_name[1] == '.' && next->d_name[2] == 0 ) ||
                    ( next->d_type != DT_DIR ) ||
                    ( isDigitStr( next->d_name ) < 0 ) ) {
                    continue;
                }

                if( atoi( next->d_name ) == pid ) {
                    n_ret = 1;
                    break;
                }
            }

            closedir( p_dir );

            if( n_ret ) {
                timeo --;
                sleep( 1 );
            }
        }
        while( n_ret && timeo > 0 );

        return n_ret;
    };


    static int waitFd( const int &n_fd, int n_timeout ) {
        int     ret = 0;
        int     maxfd = 0;
        fd_set  fds;
        struct timeval timeout = { n_timeout / 1000, n_timeout % 1000 };
        memset( &fds, 0, sizeof( fd_set ) );

        if( n_fd <= 0 ) {
            return -2;
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
            perror( "Select Socket" );
            return -1;

        case 0:
            return 0;

        default:
            if( FD_ISSET( n_fd, &fds ) ) {
                return 1;
            }

            break;
        }

        return -3;
    };

    static int waitAndRecv( const int &n_udp_sock, int n_ms_timeout, string* out_addr, void* out_data_recv, size_t in_data_sz ) {
        int ret = 0;
        struct sockaddr_un t_addr;
        socklen_t t_addr_size = sizeof( struct sockaddr_un );
        RETNIF( waitFd( n_udp_sock, n_ms_timeout ) != 1, 0 );
        memset( &t_addr, 0, sizeof( t_addr ) );
        ret = recvfrom( n_udp_sock, out_data_recv, in_data_sz, 0, ( struct sockaddr* )( &t_addr ), &t_addr_size );

        if( ret < 0 ) {
            perror( "Recv Data" );
            return ret;
        }

        if( out_addr != NULL ) {
            *out_addr = t_addr.sun_path;
        }

        return ret;
    };

    static int waitAndRead( const int &n_fd, int n_ms_timeout, void* out_data_recv, size_t in_data_sz ) {
        int ret = 0;
        RETNIF( waitFd( n_fd, n_ms_timeout ) != 1, 0 );
        ret = read( n_fd, out_data_recv, in_data_sz );

        if( ret < 0 ) {
            perror( "Recv Data" );
            return ret;
        }

        return ret;
    };

    static string generateUUID() {
        static const int BUF_SZ = 37;
        char buf[BUF_SZ + 1] = {0};
        ReadFile( "/proc/sys/kernel/random/uuid", buf, BUF_SZ );
        buf[BUF_SZ - 1] = 0;
        return string( buf );
    };

	// make time string as ISO8601
    static string getTimeString( time_t n_time = 0 ) {
        if( n_time <= 0 ) {
            time( &n_time );
        }

		static const int BUF_SZ = 32;
        char buf[BUF_SZ + 4] = {0};
		strftime(buf, BUF_SZ, "%FT%TZ", gmtime(&n_time));
        return string( buf );
    };

	// parse time string from ISO8601
	static time_t parseTimeString( const string& str ) {
		struct tm time_struct;
		strptime(str.c_str(), "%Y-%m-%dT%H:%M:%SZ", &time_struct);
        return timegm(&time_struct);
    };

    static long getLeftSpace( const string &path ) {
        struct statfs diskInfo;
        long ret = statfs( path.c_str(), &diskInfo );
        RETNIF( ret < 0, ret );
        ret = diskInfo.f_bsize * diskInfo.f_bfree;
        return ret;
    };

    static uint32_t getFileSize( const string &file ) {
        struct stat st;
        stat( file.c_str(), &st );
        return st.st_size;
    };

    static string getMd5( const string &file ) {
        const int BUF_SZ = 1024;
        const int MD5_SZ = 16;
        unsigned char md5[MD5_SZ + 4];
        unsigned char buf[BUF_SZ + 4];
        int n = -1;
        FILE* fp = fopen( file.c_str(), "rb" );

        if( fp == NULL ) {
            return string( "" );
        }

        MD5_CTX ctx;
        MD5_Init( &ctx );

        while( ( n = fread( buf, 1, BUF_SZ, fp ) ) > 0 ) {
            MD5_Update( &ctx, buf, n );
        }

        MD5_Final( md5, &ctx );
        fclose( fp );
        char md5_result[( MD5_SZ * 2 ) + 4];

        for( int i = 0; i < MD5_SZ; i++ ) {
            snprintf( md5_result + ( i * 2 ), 4, "%02x", md5[i] );
        }

        md5_result[MD5_SZ * 2] = 0;
        return string( md5_result );
    };

    /*
     * quick method to set system time
     * formart same as struct tm:
     *   int     year;       // < Years since 1900
     *   int     mon;        // < Months since January - [0,11]
     *   int     day;        // < Day of the month - [1,31]
     *   int     hour;       // < Hours since midnight - [0,23]
     *   int     min;        // < Minutes after the hour - [0,59]
     *   int     sec;        //< Seconds after the minute - [0,59]
     */
    static void setSystemTime( int year, int month, int day, int hour, int minute, int second, int off_in_sec = 0 ) {
        int ret = 0;
        time_t target = 0;
        struct tm time = {0};
        struct tm* ptime = NULL;
        time.tm_year    = year;
        time.tm_mon     = month;
        time.tm_mday    = day;
        time.tm_hour    = hour;
        time.tm_min     = minute;
        time.tm_sec     = second;

        // calulate with offset
        if( off_in_sec != 0 ) {
            target = mktime( &time );
            target += off_in_sec;
            ptime = gmtime( &target );
            memcpy( &time, ptime, sizeof( struct tm ) );
        }

        // set time to system
        target = mktime( &time );
 //       ret = stime( &target );

        if( ret < 0 ) {
            perror( "set time failed !" );
        }

        // try to sync to rtc
        int rtcfd = -1;

        if( 0 > ( rtcfd = open( "/dev/rtc", O_WRONLY ) ) &&
            0 > ( rtcfd = open( "/dev/rtc0", O_WRONLY ) ) &&
            0 > ( rtcfd = open( "/dev/misc/rtc", O_WRONLY ) ) ) {
            fprintf( stderr, "no rtc device available !\n" );
            return;
        }

        ioctl( rtcfd, RTC_SET_TIME, &time );
        close( rtcfd );
    };

};

#endif //_CUTIL_CC_

