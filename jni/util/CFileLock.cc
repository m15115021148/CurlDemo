/**
 * @file CFileLock.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Mutex, utility mutex tool
 * @detail Cubic Mutex, utility mutex tool
 */

#ifndef _CFILE_LOCK_CC_
#define _CFILE_LOCK_CC_ 1

#include <unistd.h>
#include <fcntl.h>

class CFileLock
{
private:
    int m_fd;

    // lock whole file
    int lock( int cmd, int type ) {
        struct flock lock;
        lock.l_type = type;
        lock.l_start = 0;
        lock.l_whence = SEEK_SET;
        lock.l_len = 0;
        return ( fcntl( m_fd, cmd, &lock ) );
    }

    // test lock
    int testLock( int type ) {
        struct flock lock;
        lock.l_type = type;
        lock.l_start = 0;
        lock.l_whence = SEEK_SET;
        lock.l_len = 0;

        if( fcntl( m_fd, F_GETLK, &lock ) < 0 ) {
            return -1;
        }

        if( lock.l_type == F_UNLCK ) {
            return 0;
        }

        return 1;
    }

public:
    CFileLock( int fd )
        : m_fd( fd ) {
    };

    ~CFileLock() {
        unlock();
    };

    inline bool lock() {
        return ( 0 == lock( F_SETLKW, F_WRLCK ) );
    };

    inline bool trylock() {
        return ( 0 == lock( F_SETLK, F_WRLCK ) );
    };

    inline bool unlock() {
        return ( 0 == lock( F_SETLK, F_UNLCK ) );
    };

    inline bool test() {
        return ( 0 == testLock( F_WRLCK ) );
    }
};

#endif //_CFILE_LOCK_CC_
