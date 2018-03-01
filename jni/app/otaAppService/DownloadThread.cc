#ifndef _DOWNLOAD_THREAD_CC_
#define _DOWNLOAD_THREAD_CC_ 1

#include "cubic_inc.h"
#include "CThread.cc"
#include "CSafeQueue.cc"
#include "CRemoteReport.cc"
#include <iostream>


using namespace std;

#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "DownloadThread"

#define CUBIC_VM_MAX_DOWNLOAD_RETRY  30

class IDownloadThread
{
public:
    virtual void downloadComplete( const string &local_path, int error ) = 0;
};

class DownloadThread : public CThread
{
private:
    CSafeQueue<string>  m_download_list;
    int                 m_downloading;
    IDownloadThread*    m_download_user;

    DownloadThread()
        : m_download_list()
        , m_downloading( 0 )
        , m_download_user( NULL )
    {};

public:
    virtual ~DownloadThread()
    {};

    static DownloadThread &getInstance() {
        static DownloadThread instance;
        return instance;
    };

    inline int waitSize() {
        return m_download_list.size();
    }

    void registerUser( IDownloadThread* user ) {
        m_download_user = user;
    };

    void addNewDownload( const string &local_path ) {
        LOGD( "addNewDownload: %s", local_path.c_str() );
        RETIF_LOGD( m_download_list.exist( local_path ), "addNewDownload: already exist!" );
        m_download_list.push( local_path );
        m_downloading++;
        CubicStatSet( CUBIC_STAT_vm_downloading, m_downloading );
    };

    virtual RunRet run( void* user ) {
        UNUSED_ARG( user );

        if( CubicStatGetI( CUBIC_STAT_net_connected ) == 0 ) {
            sleep( 1 );
            return RUN_CONTINUE;
        };

        string remote_url;

        string local_path;

        RETNIF( CSafeQueue<string>::ERR_NO_ERROR != m_download_list.pop( remote_url, 100 ), RUN_CONTINUE );

        LOGD( "download ota apk: %s", remote_url.c_str() );

        for( int i = 0; i < CUBIC_VM_MAX_DOWNLOAD_RETRY; i++ ) {
		local_path = CRemoteReport::downloadApk( remote_url );
            BREAKIF( local_path.length() != 0 && local_path != "null" );
        }

        LOGD( "do_download done" );
        m_downloading--;
        CubicStatSet( CUBIC_STAT_vm_downloading, m_downloading );

		int error = 0;
        if( local_path.length() == 0 || local_path == "null" ) {
            LOGE( "failed to download message, max retry reached: %s",
                  remote_url.c_str() );
			error = 1;
        }

        if( m_download_user != NULL ) {
            m_download_user->downloadComplete( local_path, error );
        }
		if( m_downloading == 0 ) {
			CubicWakeupLockClear(CUBIC_WAKELOCK_ID_VM_DOWNLOAD);
		}
        return RUN_CONTINUE;
    };
};


#endif //_DOWNLOAD_THREAD_CC_
