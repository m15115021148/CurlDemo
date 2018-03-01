#include "cubic_inc.h"
#include "CFramework.cc"


#include "CAsyncRun.cc"
#include "CRemoteReport.cc"
#include "DownloadThread.cc"
#include "CSafeQueue.cc"
#include "CLock.cc"

#include <iostream>
#include <time.h>

#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "OtaAppService"

class OtaAppService : public ICubicApp, public IDownloadThread, public IAbsTimer
{
private:
    static const int CUBIC_VM_READ_LIST_MAX = 10;
    static const int CUBIC_VM_EXPIRED_LIST_MAX = 128;


    CLock                   m_vm_tab_lock;
    list<string>            m_vm_list_fetching;
    list<string>            m_vm_list_unread;
    list<string>            m_vm_list_read;
    list<string>            m_vm_list_expired;

    int                     m_poll_vm_timer;

    inline string getLocalPath( const string &file_name ) {
        ostringstream oss;
        oss << CUBIC_VOICE_MSG_CACHE << "/" << file_name;
        return oss.str();
    }

    // create new and make to fetching
    void addNewVMtoFetch( const string &remote_url ) {
        LOGD( "addNewVMtoFetch url=%s", remote_url.c_str() );
        CLock::Auto lock( m_vm_tab_lock );
        string file_name = CUtil::getFileNameOfPath( remote_url );

        DownloadThread::getInstance().addNewDownload( remote_url );
    };

	void pollFromServer(const string versionCode){
        // timer callback is running in standalone thread, just do report here
        vector<string> vm_list = CRemoteReport::getVMList( CUBIC_VM_READ_LIST_MAX );
        LOGD( "pollFromServer, got voice message: %d", vm_list.size() );

        for ( size_t i = 0; i < vm_list.size(); i ++ ) {
            addNewVMtoFetch( vm_list[i] );
        }
    }


public:
    bool onInit() {
        LOGD( "%s onInit: %d", CUBIC_THIS_APP, getpid() );
        curl_global_init( CURL_GLOBAL_ALL );
        DownloadThread::getInstance().registerUser( this );
        DownloadThread::getInstance().start();
        UploadThread::getInstance().start();
        return true;
    };

    void onDeInit() {
        LOGD( "%s onDeInit", CUBIC_THIS_APP );
        DownloadThread::getInstance().stop();
        UploadThread::getInstance().stop();
        return;
    };

    virtual int onMessage( const string &str_src_app_name, int n_msg_id, const void* p_data ) {
        LOGD( "onMessage: %s, %d", str_src_app_name.c_str(), n_msg_id );
	return 0;
    };

    // interface for IDownloadThread
    virtual void downloadComplete( const string &local_path, int error ) {
        if( error == 0 ) {
            moveVMtoUnread( local_path );
        }
        else {
            unlink( local_path.c_str() );
        }
    };

    // interface for IAbsTimer
    virtual void onAbsTimer( int n_timer_id ) {
		LOGD( "onAbsTimer: %d", n_timer_id );
		pollFromServer();
    };

	void loadApk(const string &url){
		addNewVMtoFetch(url);
	};
};

IMPLEMENT_CUBIC_APP( VMService )

