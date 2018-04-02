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

    void addNewDownload( const string &down_path ) {
        LOGD( "addNewDownload: %s, DownSize: %d", down_path.c_str() ,m_download_list.size());
		RETIF_LOGD( m_download_list.size()>1, "addNewDownload: already exist!" );
        RETIF_LOGD( m_download_list.exist( down_path ), "addNewDownload: already exist!" );
        m_download_list.push( down_path );
        m_downloading++;
    };

    virtual RunRet run( void* user ) {
        UNUSED_ARG( user );
		
		string url;

        string local_path;

        RETNIF( CSafeQueue<string>::ERR_NO_ERROR != m_download_list.pop( url, 1 ), RUN_CONTINUE );

        LOGD( "download ota apk: %s", url.c_str() );
		
		local_path = CRemoteReport::getInstance().downloadApk( url );
				
        //m_downloading--;
		
		int error = 0;
        if( local_path.length() == 0 || local_path == "null" ) {
            LOGE( "failed to download message, max retry reached: %s",url.c_str() );
			error = 1;
        }

        if( m_download_user != NULL ) {
            m_download_user->downloadComplete( local_path, error );
        }

        return RUN_CONTINUE;
    };
};


#endif //_DOWNLOAD_THREAD_CC_