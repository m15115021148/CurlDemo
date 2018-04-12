/**
 * @file CRemoteReport.cc
 * @author Shujie.Li
 * @version 1.0
 * @brief package for push message
 * @detail package for push message
 */
#ifndef _REMOTE_REPORT_CC_
#define _REMOTE_REPORT_CC_ 1

#include "cubic_inc.h"
#include "CUtil.cc"
#include "CLock.cc"
#include "CStringTool.cc"
#include "CFramework.cc"
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/allocators.h>
#include <rapidjson/stringbuffer.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <openssl/crypto.h>
#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <vector>
#include <iomanip>
#include <string.h>
#include <fstream>
#include <assert.h>

#define CUBIC_REQUEST_APP_KEY 				"2017fbd152bf43c796219ad494cc010d"
#define CUBIC_REQUEST_APP_SECRET 			"22f7f12b2348444aadb8aeb9ecd7a359"
#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "CRemoteReport"


using namespace rapidjson;
using namespace std;

class IDownloadCallBack
{
public:
	virtual void downloadProgress( double dltotal, double dlnow ) = 0;
	virtual void downloadFailuer( string result) = 0; 
	virtual void downloadCancel() = 0;
};

class CRemoteReport
{
private:
    static const int JSON_SIZE_MAX = 1600;
	IDownloadCallBack *p_down_load;
	
	CRemoteReport()
		:p_down_load( NULL )
	{};

    string getSHA256( const void* data, int size ) {
        string ret = "error";
        EVP_MD_CTX* mdctx;
        uint8_t* digest = NULL;
        uint32_t digist_len = 0;
        mdctx = EVP_MD_CTX_create();
        RETNIF( mdctx == NULL, ret );

        do {
            BREAKIF( 1 != EVP_DigestInit_ex( mdctx, EVP_sha256(), NULL ) );
            BREAKIF( 1 != EVP_DigestUpdate( mdctx, data, size ) );
            digest = ( uint8_t* )OPENSSL_malloc( EVP_MD_size( EVP_sha256() ) );
            BREAKIF( NULL == digest );
            BREAKIF( 1 != EVP_DigestFinal_ex( mdctx, digest, &digist_len ) );
            ostringstream oss;
            oss << hex << setw( 2 ) << setfill( '0' );

            for( uint32_t i = 0; i < digist_len; i++ ) {
                oss << digest[i];
            }

            ret = oss.str();
        }
        while( 0 );

        OPENSSL_free( digest );
        EVP_MD_CTX_cleanup( mdctx );
        EVP_MD_CTX_destroy( mdctx );
        return ret;
    };

    string encodeToBase64( const char* input, int size ) {
        BIO* bio, *b64;
        BUF_MEM* bufferPtr;
        b64 = BIO_new( BIO_f_base64() );
        bio = BIO_new( BIO_s_mem() );
        bio = BIO_push( b64, bio );
        //BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
        BIO_write( bio, input, size );
        BIO_flush( bio );
        BIO_get_mem_ptr( bio, &bufferPtr );
        BIO_set_close( bio, BIO_NOCLOSE );
        BIO_free_all( bio );
        string ret( bufferPtr->data );
        BUF_MEM_free( bufferPtr );
        return CStringTool::trim( ret );
    };

    size_t calcDecodeLength( const char* b64input ) { //Calculates the length of a decoded string
        size_t len = strlen( b64input ),
               padding = 0;

        if ( b64input[len - 1] == '=' && b64input[len - 2] == '=' ) //last two chars are =
        { padding = 2; }
        else if ( b64input[len - 1] == '=' ) //last char is =
        { padding = 1; }

        return ( len * 3 ) / 4 - padding;
    }

    string decodeFromBase64( char* input, int size ) {
        BIO* bio, *b64;
        char* buffer = NULL;
        int decodeLen = calcDecodeLength( input );
        buffer = ( char* )malloc( decodeLen + 1 );
        RETNIF( buffer == NULL, "" );
        buffer[decodeLen] = '\0';
        bio = BIO_new_mem_buf( input, -1 );
        b64 = BIO_new( BIO_f_base64() );
        bio = BIO_push( b64, bio );
        //BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
        int length = BIO_read( bio, buffer, decodeLen );
        assert( length == decodeLen ); //length should equal decodeLen, else something went horribly wrong
        BIO_free_all( bio );
        string ret( buffer );
        free( buffer );
        return ret;
    };

    string getWsseHeader() {
        string uname = CubicCfgGetStr( CUBIC_CFG_push_uname );
        string upass = CubicCfgGetStr( CUBIC_CFG_push_upswd );
        string nonce = CUtil::generateUUID();
        string create = CUtil::getTimeString();
        string digest_srouce;
        digest_srouce += nonce;
        digest_srouce += create;
        digest_srouce += upass;
        string digest = getSHA256( digest_srouce.c_str(), digest_srouce.length() );
        string digest_base64 = encodeToBase64( digest.c_str(), digest.length() );
        string wsse = "X-WSSE: UsernameToken Username=\"";
        wsse += uname;
        wsse += "\",PasswordDigest=\"";
        wsse += digest_base64;
        wsse += "\",Nonce=\"";
        wsse += nonce;
        wsse += "\",Created=\"";
        wsse += create;
        wsse += "\"";
        LOGD( "getWsseHeader=%s", wsse.c_str() );
        return wsse;
    };

	string getCustomHeader(){
		string uname = CubicCfgGetStr( CUBIC_CFG_push_uname );
		string x_uuid = "client_uuid: ";
		x_uuid += uname;
		LOGD("getCustomHeader=%s", x_uuid.c_str() );
		return x_uuid;
	}

	string getCustomHeader1(){
		string psw = CubicCfgGetStr( CUBIC_CFG_push_upswd );
		string x_api_psw = "api_password: ";
		x_api_psw += psw;
		LOGD("getCustomHeader1=%s", x_api_psw.c_str() );
		return x_api_psw;
    }

    static int CurlDebugCallback( CURL* handle, curl_infotype type, char* data, size_t size, void* userptr ) {
        const int BUF_SIZE = 1024;
        char buf[BUF_SIZE + 4];
        strncpy( buf, data, MIN( BUF_SIZE, size ) );

        switch ( type ) {
        case CURLINFO_HEADER_OUT:
            LOGI( "==> Send Head: %s", buf );
            break;

        case CURLINFO_DATA_OUT:
            LOGI( "==> Send data (size:%u)", size );
            break;

        case CURLINFO_SSL_DATA_OUT:
            LOGI( "==> Send SSL data" );
            break;

        case CURLINFO_HEADER_IN:
            LOGI( "<== Recv Head: %s", buf );
            break;

        case CURLINFO_DATA_IN:
            LOGI( "<== Recv data (size:%u)", size );
            break;

        case CURLINFO_SSL_DATA_IN:
            LOGI( "<== Recv SSL data" );
            break;

        case CURLINFO_TEXT:
            LOGI( "transfer info: %s", buf );
            break;

        default: /* in case a new one is introduced to shock us */
            break;
        }

        return 0;
    };

    typedef struct BufferStruct {
        char* buf;
        size_t size;
        size_t max;
    } BufferStruct;

    static size_t WriteFixBufferCallback( void* contents, size_t size, size_t nmemb, void* userp ) {
        struct BufferStruct* mem = ( struct BufferStruct* )userp;
        size_t realsize = size * nmemb;
        LOGD( "WriteFixBufferCallback size=%u", size );
#if 0 // auto resize
        mem->buf = realloc( mem->buf, mem->size + realsize + 1 );
        RETNIF_LOGE( mem->buf == NULL,
                     "WriteBufferCallback not enough memory when realloc, new size:%d",
                     realsize );
#else
        RETNIF( mem == NULL, 0 );
        RETNIF_LOGE( mem->buf == NULL || mem->size + realsize + 1 > mem->max, -1,
                     "WriteBufferCallback not enough memory when realloc, new:%d, curr:%d, max:%d",
                     realsize, mem->size, mem->max );
#endif
        memcpy( mem->buf + mem->size, contents, realsize );
        mem->size += realsize;
        mem->buf[mem->size] = 0;
        return realsize;
    };

    int sendRequest( const string &addr, const char* req = NULL, char* resp = NULL, int resp_sz = 0, bool withWsse = true ) {
        CURL* curl = curl_easy_init();
        RETNIF( curl == NULL, -1 );
        struct curl_slist* head_list = NULL;
        BufferStruct resp_buf = {NULL, 0, 0};
        LOGD( "sendRequest addr=%s, req=%s", addr.c_str(), req == NULL ? "NULL" : req );
        // cutomize header
        head_list = curl_slist_append( head_list, "Content-Type: application/json; charset=UTF-8" );

        if( withWsse ) {
            head_list = curl_slist_append( head_list, getWsseHeader().c_str() );
        }

		head_list = curl_slist_append( head_list , getCustomHeader().c_str() );
		head_list = curl_slist_append( head_list , getCustomHeader1().c_str() );

        curl_easy_setopt( curl, CURLOPT_URL, addr.c_str() );
        curl_easy_setopt( curl, CURLOPT_HTTPHEADER, head_list );

        if( req != NULL ) {
            curl_easy_setopt( curl, CURLOPT_POST, TRUE );
            curl_easy_setopt( curl, CURLOPT_POSTFIELDS, req );
        }
        else {
            curl_easy_setopt( curl, CURLOPT_HTTPGET, TRUE );
        }

        if( resp != NULL && resp_sz > 0 ) {
            resp_buf.buf = resp;
            resp_buf.size = 0;
            resp_buf.max = resp_sz;
            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteFixBufferCallback );
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, ( void* )&resp_buf );
        }

        curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );
        curl_easy_setopt( curl, CURLOPT_DEBUGFUNCTION, CurlDebugCallback );
        curl_easy_setopt( curl, CURLOPT_TIMEOUT, 55 );
        curl_easy_setopt( curl, CURLOPT_AUTOREFERER, 1 );
        curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 );
        curl_easy_setopt( curl, CURLOPT_MAXREDIRS, 1 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 30 );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, FALSE );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, FALSE );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1L);
        //curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT_MS, 2000L );
        //curl_easy_setopt( curl, CURLOPT_EXPECT_100_TIMEOUT_MS, 2000L );
        //curl_easy_setopt( curl, CURLOPT_ACCEPTTIMEOUT_MS, 2000L );
        LOGD( "sendRequest before curl_easy_perform" );
        int ret = curl_easy_perform( curl );
        LOGD( "sendRequest curl_easy_perform ret=%d", ret );

        if( ret != CURLE_OK ) {
            LOGE( "sendRequest error when curl_easy_perform, ret=%d", ret );
            curl_easy_cleanup( curl );
            return -1;
        }

        curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &ret );
        curl_easy_cleanup( curl );
        LOGD( "sendRequest ret=%d", ret );
        return ret;
    };

    typedef struct FileStruct {
        FILE* file;
        size_t size;
        size_t max;
    } FileStruct;

    static size_t WriteToFileCallback( void* contents, size_t size, size_t nmemb, void* userp ) {
        struct FileStruct* file = ( struct FileStruct* )userp;
        size_t realsize = size * nmemb;
        //LOGD( "WriteToFileCallback size=%u", size );
        RETNIF_LOGE( realsize + file->size > file->max, -1,
                     "WriteToFileCallback failed for over size, last size:%d", file->size );
        RETNIF_LOGE( 0 >= fwrite( contents, size, nmemb, file->file ), -1,
                     "WriteToFileCallback failed when write file" );
        file->size += realsize;
        return realsize;
    };

    int sendRequestFile( const string &addr, FILE* file, size_t limit, const char* req = NULL, bool withWsse = true ) {
        RETNIF( file == NULL || limit == 0, -1 );
        CURL* curl = curl_easy_init();
        RETNIF( curl == NULL, -1 );
        struct curl_slist* head_list = NULL;
        FileStruct resp_file = {file, 0, limit};
        LOGD( "sendRequestFile addr=%s, req=%s", addr.c_str(), req == NULL ? "NULL" : req );
        // cutomize header
        head_list = curl_slist_append( head_list, "Content-Type: application/json; charset=UTF-8" );

        if( withWsse ) {
            head_list = curl_slist_append( head_list, getWsseHeader().c_str() );
        }

		head_list = curl_slist_append( head_list , getCustomHeader().c_str() );
		head_list = curl_slist_append( head_list , getCustomHeader1().c_str() );

        curl_easy_setopt( curl, CURLOPT_URL, addr.c_str() );
        curl_easy_setopt( curl, CURLOPT_HTTPHEADER, head_list );

        if( req != NULL ) {
            curl_easy_setopt( curl, CURLOPT_POST, TRUE );
            curl_easy_setopt( curl, CURLOPT_POSTFIELDS, req );
        }
        else {
            curl_easy_setopt( curl, CURLOPT_HTTPGET, TRUE );
        }

        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteToFileCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, ( void* )&resp_file );
        curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );
        curl_easy_setopt( curl, CURLOPT_DEBUGFUNCTION, CurlDebugCallback );
        curl_easy_setopt( curl, CURLOPT_TIMEOUT, 55 );
        curl_easy_setopt( curl, CURLOPT_AUTOREFERER, 1 );
        curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 );
        curl_easy_setopt( curl, CURLOPT_MAXREDIRS, 1 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 30 );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, FALSE );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, FALSE );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1L);
        //curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT_MS, 2000L );
        //curl_easy_setopt( curl, CURLOPT_EXPECT_100_TIMEOUT_MS, 2000L );
        //curl_easy_setopt( curl, CURLOPT_ACCEPTTIMEOUT_MS, 2000L );
        LOGD( "sendRequestFile before curl_easy_perform" );
        int ret = curl_easy_perform( curl );
        LOGD( "sendRequestFile curl_easy_perform ret=%d", ret );

        if( ret != CURLE_OK ) {
            LOGE( "sendRequestFile error when curl_easy_perform, ret=%d", ret );
            curl_easy_cleanup( curl );
            return -1;
        }

        curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &ret );
        curl_easy_cleanup( curl );
        LOGD( "sendRequestFile ret=%d", ret );
        return ret;
    };

	static int ProgressCallback(void *progress_data, double dltotal, double dlnow, double ultotal,  double ulnow) {  
		//LOGD("ProgressCallback ->(%g %%)\n", dlnow*100.0/dltotal);  
		if ( progress_data ) {
			IDownloadCallBack* p = (IDownloadCallBack*)progress_data;
			p->downloadProgress( dltotal, dlnow );
		}		
		return 0;  
	} 
	
	int downloadFile(const string &addr,FILE* file) {
        CURL* curl = curl_easy_init();
        RETNIF( curl == NULL, -1 );
        struct curl_slist* head_list = NULL;
        FileStruct resp_file = {file, 0 , 0xFFFFFF};
		LOGD( "downloadFile down_url=%s", addr.c_str());
		
        head_list = curl_slist_append( head_list, "Content-Type: application/json; charset=UTF-8" );
		
        curl_easy_setopt( curl, CURLOPT_URL, addr.c_str() );
        curl_easy_setopt( curl, CURLOPT_HTTPHEADER, head_list );
        curl_easy_setopt( curl, CURLOPT_HTTPGET, TRUE );
		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteToFileCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, ( void* )&resp_file );
        curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );
		curl_easy_setopt( curl, CURLOPT_NOPROGRESS, 0 );  
		curl_easy_setopt( curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback );  
        curl_easy_setopt( curl, CURLOPT_PROGRESSDATA, p_down_load );  
        curl_easy_setopt( curl, CURLOPT_AUTOREFERER, 1 );
        curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 );
        curl_easy_setopt( curl, CURLOPT_MAXREDIRS, 1 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 30 );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, FALSE );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, FALSE );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1L);
        LOGD( "downloadFile before curl_easy_perform" );
        int ret = curl_easy_perform( curl );
        LOGD( "downloadFile curl_easy_perform ret=%d", ret );

        if( ret != CURLE_OK ) {
            LOGE( "downloadFile error when curl_easy_perform, ret=%d", ret );
            curl_easy_cleanup( curl );
            return -1;
        }

        curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &ret );
        curl_easy_cleanup( curl );
        LOGD( "downloadFile ret=%d", ret );
        return ret;
    };
	
public:
	virtual ~CRemoteReport()
    {};
	
	void registerDownloadCallBack( IDownloadCallBack* user ) {
        p_down_load = user;
    };

    static CRemoteReport &getInstance() {
        static CRemoteReport instance;
        return instance;
    };

    int activate(const string &userName, const string &versionCode, const string &versionName) {
        char req[JSON_SIZE_MAX + 4] = {0};
        char resp[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        LOGD( "activate()" );
        // TODO: use default setting instead
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"lang\":\"en\","
                  "\"bundle_id\":\"info.e3phone.iPhone\","
                  "\"name\":\"%s\","
				  "\"serial_num\":\"%s\","
		  		  "\"versionCode\":\"%s\","
	       	  	  "\"versionName\":\"%s\","
		  		  "\"appKey\":\"%s\","
                  "\"appSecret\":\"%s\""
                  "}",
                  userName.c_str(),
                  CubicCfgGetStr( CUBIC_CFG_serial_num ).c_str(), 
				  versionCode.c_str(), 
			      versionName.c_str(), 
			      CUBIC_REQUEST_APP_KEY, 
			      CUBIC_REQUEST_APP_SECRET );
        snprintf( addr, PATH_MAX, "%s/app.json", CubicCfgGetStr( CUBIC_CFG_push_server ).c_str() );
        int ret = sendRequest(  addr, req, resp, JSON_SIZE_MAX, false );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "activate request refused, http result=%d", ret );
		LOGD("activate resp=%s",resp );
        Document resp_dom;
        RETNIF_LOGE( resp_dom.ParseInsitu( resp ).HasParseError(), -1, "activate error when parse response: %s", resp );
        RETNIF_LOGE( !resp_dom.HasMember( "client_uuid" ) || !resp_dom["client_uuid"].IsString(), -2, "activate fail, not valid client_uuid !" );
        RETNIF_LOGE( !resp_dom.HasMember( "api_password" ) || !resp_dom["api_password"].IsString(), -3, "activate fail, not valid api_password !" );
        CubicCfgSet( CUBIC_CFG_push_uname, resp_dom["client_uuid"].GetString() );
        CubicCfgSet( CUBIC_CFG_push_upswd, resp_dom["api_password"].GetString() );
        return 0;
    };   

	string updateApp(const string &versionCode){
		char req[JSON_SIZE_MAX + 4] = {0};
		char resp[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        LOGD( "updateApp()" );
		
		snprintf( addr, PATH_MAX, "%s/ota/version.json?appKey=%s&appSecret=%s&ver=%s", 
						CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
						CUBIC_REQUEST_APP_KEY ,
						CUBIC_REQUEST_APP_SECRET,
						versionCode.c_str() );
		int ret = sendRequest(  addr, req, resp, JSON_SIZE_MAX );
		RETNIF_LOGE( ret > 299 || ret < 200, resp, "getDeviceList request refused, http result=%d", ret );
		LOGD("resp=%s",resp);
		Document resp_dom;
		RETNIF_LOGE( resp_dom.ParseInsitu( resp ).HasParseError(), "", "updateApp error when parse response: %s", resp );
		RETNIF_LOGE( !resp_dom.HasMember( "result" ) || !resp_dom["result"].IsNumber(), "", "updateApp fail, not valid result !" );
		RETNIF_LOGE( !resp_dom.HasMember( "versionCode" ) || !resp_dom["versionCode"].IsString(), "", "updateApp fail, not valid versionCode !" );
		RETNIF_LOGE( !resp_dom.HasMember( "url" ) || !resp_dom["url"].IsString(), "", "updateApp fail, not valid url !" );
		
		string json;
		Document doc;
		doc.SetObject();
        Document::AllocatorType &allocator = doc.GetAllocator();
		if(resp_dom["result"].GetInt() == 200 && resp_dom["versionCode"].GetString() > versionCode ){
			doc.AddMember( "result", 		200, 													allocator );
			doc.AddMember( "versionCode", 	Value(resp_dom["versionCode"].GetString(), allocator), 	allocator );
			doc.AddMember( "downUrl", 		Value(resp_dom["url"].GetString(), allocator), 	allocator );
		}else {
			doc.AddMember( "result", 		400, 													allocator );
			doc.AddMember( "versionCode", 	Value(versionCode.c_str(), allocator) , 				allocator );
			doc.AddMember( "downUrl", 		Value("", allocator),  									allocator );
		}
		StringBuffer buffer;
        Writer<StringBuffer> writer( buffer );
        doc.Accept( writer );
        json = buffer.GetString();
		//save content
		CubicCfgSet(CUBIC_APP_OTA_CACHE, json);
		return json;
	};

	string downloadApk(const string &path ){
		LOGD("downloadApk ...");
		string req ;
		string fname = path;
		string url;
		fname += "/MeigApp/";
		
		//create dir
		if ( !CUtil::mkdirAndParrent( CUtil::getParrentDirOfPath( fname ), S_IRWXU | S_IRWXG | S_IROTH ) ){
			LOGE("downloadApk, file not created");
			return "";
		}
		
		//get down url
		Document doc;
		req = CubicCfgGetStr(CUBIC_APP_OTA_CACHE);
		RETNIF_LOGE( doc.Parse<0>( req.c_str() ).HasParseError(), "", "downloadApk error when parse request: %s", req.c_str() );
		RETNIF_LOGE( !doc.HasMember( "downUrl" ) || !doc["downUrl"].IsString(), "", "downloadApk fail, not valid downUrl !" );
		RETNIF_LOGE( !doc.HasMember( "result" ) || !doc["result"].IsNumber(), "", "downloadApk fail, not valid result !" );
		
		if(doc["result"].GetInt() == 400 && doc["downUrl"].GetString() == NULL ){
			LOGD("this app is already New,not udapte");
			return "";
		}
		
		url = doc["downUrl"].GetString();
		fname += CUtil::getFileNameOfPath( url );
		FILE* file = fopen( fname.c_str(), "w+" );
		RETNIF_LOGE( file == NULL, "null", "downloadApk failed, file can not open" );
		int ret = downloadFile( url, file );
		fclose( file );
		if( ret < 0){
			unlink( fname.c_str() );
			return "";
		}
		LOGD("downloadApk success fpath=%s",fname.c_str() );
		return fname;
	};
};


#endif //_REMOTE_REPORT_CC_
