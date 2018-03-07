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
//#include "OtaAppService.cc"
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <rapidjson/document.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <openssl/crypto.h>
#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <vector>
#include <iomanip>

#define CUBIC_REQUEST_APP_KEY "2017fbd152bf43c796219ad494cc010d"
#define CUBIC_REQUEST_APP_SECRET "22f7f12b2348444aadb8aeb9ecd7a359"
#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "CRemoteReport"


using namespace rapidjson;
using namespace std;


class CRemoteReport
{
private:
    static const int JSON_SIZE_MAX = 1600;

    static string getSHA256( const void* data, int size ) {
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

    static string encodeToBase64( const char* input, int size ) {
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


    static size_t calcDecodeLength( const char* b64input ) { //Calculates the length of a decoded string
        size_t len = strlen( b64input ),
               padding = 0;

        if ( b64input[len - 1] == '=' && b64input[len - 2] == '=' ) //last two chars are =
        { padding = 2; }
        else if ( b64input[len - 1] == '=' ) //last char is =
        { padding = 1; }

        return ( len * 3 ) / 4 - padding;
    }

    static string decodeFromBase64( char* input, int size ) {
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

    static string getWsseHeader() {
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

	static string getCustomHeader(){
		string uname = CubicCfgGetStr( CUBIC_CFG_push_uname );
		string x_uuid = "client_uuid: ";
		x_uuid += uname;
		LOGD("getCustomHeader=%s", x_uuid.c_str() );
		return x_uuid;
	}

	static string getCustomHeader1(){
                string psw = CubicCfgGetStr( CUBIC_CFG_push_upswd );
                string x_api_psw = "api_password: ";
                x_api_psw += psw;
                LOGD("getCustomHeader1=%s", x_api_psw.c_str() );
                return x_api_psw;
        }

    static int CurlDebugCallback(
        CURL* handle,
        curl_infotype type,
        char* data,
        size_t size,
        void* userptr ) {
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

    static int sendRequest( const string &addr, const char* req = NULL, char* resp = NULL, int resp_sz = 0, bool withWsse = true ) {
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
        LOGD( "WriteToFileCallback size=%u", size );
        RETNIF_LOGE( realsize + file->size > file->max, -1,
                     "WriteToFileCallback failed for over size, last size:%d", file->size );
        RETNIF_LOGE( 0 >= fwrite( contents, size, nmemb, file->file ), -1,
                     "WriteToFileCallback failed when write file" );
        file->size += realsize;
        return realsize;
    };

    static int sendRequestFile( const string &addr, FILE* file, size_t limit, const char* req = NULL, bool withWsse = true ) {
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


    static int askVMUpload( string &out_upload_url, string &out_upload_key ) {
        char resp[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        snprintf( addr, PATH_MAX, "%s/groups/%s/voice_message_server.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_group ).c_str() );
        int ret = sendRequest(  addr, NULL, resp, JSON_SIZE_MAX );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "askVMUpload request refused, http result=%d", ret );
        Document resp_dom;
        RETNIF_LOGE( resp_dom.ParseInsitu( resp ).HasParseError(), -1, "activate error when parse response: %s", resp );
        RETNIF_LOGE( !resp_dom.HasMember( "voice_message_server" ), -2, "activate fail, not valid voice_message_server !" );
        RETNIF_LOGE( !resp_dom["voice_message_server"].HasMember( "url" ) ||
                     !resp_dom["voice_message_server"]["url"].IsString(), -3, "activate fail, not valid url !" );
        RETNIF_LOGE( !resp_dom["voice_message_server"].HasMember( "url" ) ||
                     !resp_dom["voice_message_server"]["upload_key"].IsString(), -4, "activate fail, not valid upload_key !" );
        out_upload_url = resp_dom["voice_message_server"]["url"].GetString();
        out_upload_key = resp_dom["voice_message_server"]["upload_key"].GetString();
        return 0;
    };

    static int uploadVMFile(
        const string &in_upload_url,
        const string &in_upload_key,
        const string &in_upload_file,
        int64_t in_duration,
        string &out_uuid,
        string &out_url,
        string &out_create ) {
        CURL* curl = curl_easy_init();
        RETNIF( curl == NULL, -1 );
        struct curl_slist* head_list = NULL;
        struct curl_httppost* post = NULL;
        struct curl_httppost* last = NULL;
        BufferStruct resp_buf = {NULL, 0, 0};
        char resp[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        string client_id = CubicCfgGetStr( CUBIC_CFG_push_uname );
        string latitude = CubicStatGetStr( CUBIC_STAT_location_lat );
        string longitude = CubicStatGetStr( CUBIC_STAT_location_long );
        string header_client_id = "X-CLIENT-UUID: ";
        string header_upload_key = "X-UPLOAD-KEY: ";
        string duration = CStringTool::toString( in_duration / 1000 ); // us to ms
        // prepare key
        snprintf( addr, PATH_MAX, "%s", in_upload_url.c_str() );
        header_client_id += client_id;
        header_upload_key += in_upload_key;
        LOGD( "uploadVMFile: url:%s  key:%s  file:%s duration:%s", addr, in_upload_key.c_str(), in_upload_file.c_str(), duration.c_str()  );
        // cutomize header
        //head_list = curl_slist_append(head_list, "Content-Type: multipart/form-data" );
        head_list = curl_slist_append( head_list, header_client_id.c_str() );
        head_list = curl_slist_append( head_list, header_upload_key.c_str() );
        //   head_list = curl_slist_append(head_list, "X-Api-Key: "+CubicGetConfig(CUBIC_CFG_push_api_key));
        // post content
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "X-CLIENT-UUID",
                      CURLFORM_PTRCONTENTS,     client_id.c_str(),
                      CURLFORM_CONTENTSLENGTH,  client_id.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "X-UPLOAD-KEY",
                      CURLFORM_PTRCONTENTS,     in_upload_key.c_str(),
                      CURLFORM_CONTENTSLENGTH,  in_upload_key.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "upload_key",
                      CURLFORM_PTRCONTENTS,     in_upload_key.c_str(),
                      CURLFORM_CONTENTSLENGTH,  in_upload_key.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "voiceTime",
                      CURLFORM_PTRCONTENTS,     duration.c_str(),
                      CURLFORM_CONTENTSLENGTH,  duration.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "voice_message[client_uuid]",
                      CURLFORM_PTRCONTENTS,     client_id.c_str(),
                      CURLFORM_CONTENTSLENGTH,  client_id.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "voice_message[latitude]",
                      CURLFORM_PTRCONTENTS,     latitude.c_str(),
                      CURLFORM_CONTENTSLENGTH,  latitude.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "voice_message[longitude]",
                      CURLFORM_PTRCONTENTS,     longitude.c_str(),
                      CURLFORM_CONTENTSLENGTH,  longitude.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "voice_message[file]",
                      CURLFORM_FILE,     in_upload_file.c_str(),
                      CURLFORM_CONTENTTYPE,  "audio/aac",
                      CURLFORM_END );
        // prepare buffer
        resp_buf.buf = resp;
        resp_buf.size = 0;
        resp_buf.max = JSON_SIZE_MAX;
        // setup all options
        curl_easy_setopt( curl, CURLOPT_URL, addr );
        curl_easy_setopt( curl, CURLOPT_HTTPHEADER, head_list );
        //curl_easy_setopt( curl, CURLOPT_POST, TRUE );
        curl_easy_setopt( curl, CURLOPT_HTTPPOST, post );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteFixBufferCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, ( void* )&resp_buf );
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
        LOGD( "uploadVMFile before curl_easy_perform" );
        int ret = curl_easy_perform( curl );
        LOGD( "uploadVMFile curl_easy_perform ret=%d", ret );
        curl_formfree( post );

        if( ret != CURLE_OK ) {
            LOGE( "uploadVMFile error when curl_easy_perform, ret=%d", ret );
            curl_easy_cleanup( curl );
            return -1;
        }

        curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &ret );
        curl_easy_cleanup( curl );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "uploadVMFile request refused, http result=%d", ret );
        Document resp_dom;
        LOGI( "uploadVMFile resp:%s", resp );
        RETNIF_LOGE( resp_dom.ParseInsitu( resp ).HasParseError(), -1, "uploadVMFile error when parse response: %s", resp );
        Value &message = resp_dom;

        if( resp_dom.HasMember( "send_voice_message" ) ) {
            LOGD( "uploadVMFile has member send_voice_message" );
            message = resp_dom["send_voice_message"];
        }

        RETNIF_LOGE( !message.HasMember( "uuid" ) ||
                     !message["uuid"].IsString(), -3, "uploadVMFile fail, not valid uuid !" );
        RETNIF_LOGE( !message.HasMember( "download_url" ) ||
                     !message["download_url"].IsString(), -4, "uploadVMFile fail, not valid url !" );
        RETNIF_LOGE(  ( !message.HasMember( "created" ) || !message["created"].IsString() ) &&
                      ( !message.HasMember( "created_at" ) || !message["created_at"].IsString() ),
                      -6, "uploadVMFile fail, not valid created !" );
        out_uuid = message["uuid"].GetString();
        out_url = message["download_url"].GetString();

        if( message.HasMember( "created_at" ) ) {
            out_create = message["created_at"].GetString();
        }
        else {
            out_create = message["created"].GetString();
        }

        LOGD( "uploadVMFile sucess !" );
        return 0;
    };

    static int reportVM( const string &in_uuid, const string &in_url, const string &in_key ) {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        LOGD( "reportVM in_uuid=%s;  in_url=%s;  in_key=%s",
              in_uuid.c_str(), in_url.c_str(), in_key.c_str() );
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"voice_message_upload_complete\":"
                  "{"
                  "\"uuid\":\"%s\","
                  "\"url\":\"%s\","
                  "\"upload_key\":\"%s\","
                  "\"client_uuid\":\"%s\","
                  "\"latitude\":%s,"
                  "\"longitude\":%s"
                  "}"
                  "}",
                  in_uuid.c_str(),
                  in_url.c_str(),
                  in_key.c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_uname ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_lat ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str() );
        snprintf( addr, PATH_MAX, "%s/groups/%s/voice_message_upload_complete.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_group ).c_str() );
        int ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportHelp request refused, http result=%d", ret );
        return 0;
    };

public:
    static int activate(const string &userName, const string &versionCode, const string &versionName) {
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

        if( resp_dom.HasMember( "sip" ) ) {
            Value &sip = resp_dom["sip"];

            if( sip.HasMember( "domain" ) ) {
                CubicCfgSet( CUBIC_CFG_sip_domain, sip["domain"].GetString() );
            }

            if( sip.HasMember( "proxy" ) ) {
                CubicCfgSet( CUBIC_CFG_sip_proxy, sip["proxy"].GetString() );
            }

            if( sip.HasMember( "registrar" ) ) {
                CubicCfgSet( CUBIC_CFG_sip_registrar, sip["registrar"].GetString() );
            }

            if( sip.HasMember( "username" ) ) {
                CubicCfgSet( CUBIC_CFG_sip_uname, sip["username"].GetString() );
            }

            if( sip.HasMember( "password" ) ) {
                CubicCfgSet( CUBIC_CFG_sip_upswd, sip["password"].GetString() );
            }
        }

        return 0;
    };

    static int reportLocation( bool isHeartbeat = false ) {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"events\":["
                  "{"
                  "\"type\":\"%s\","
                  "\"at\":\"%s\","
                  "\"geo\":{"
                  "\"lat\":%s,"
                  "\"long\":%s,"
                  "\"lon\":%s,"
                  "\"vel\":%s,"
                  "\"alt\":%s,"
                  "\"dir\":%s"
                  "}"
                  "}"
                  "]"
                  "}",
                  isHeartbeat ? "heartbeat" : "tracking",
                  CUtil::getTimeString().c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_lat ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_vel ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_alt ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_dir ).c_str()  );
        snprintf( addr, PATH_MAX, "%s/clients/%s/device_events.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_uname ).c_str() );
        int ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportLocation request refused, http result=%d", ret );
        return 0;
    };

    static int reportGeoFence( int fenceId, bool isEntry ) {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"events\":["
                  "{"
                  "\"type\":\"geo_fence\","
                  "\"at\":\"%s\","
                  "\"geo\":{"
                  "\"lat\":%s,"
                  "\"long\":%s,"
                  "\"lon\":%s,"
                  "\"vel\":%s,"
                  "\"alt\":%s,"
                  "\"dir\":%s"
                  "},"
                  "\"gf\":{"
                  "\"name\":\"%s\","
                  "\"lat\":%s,"
                  "\"long\":%s,"
                  "\"lon\":%s,"
                  "\"rad\":%s,"
                  "\"dir\":\"%s\""
                  "}"
                  "}"
                  "]"
                  "}",
                  CUtil::getTimeString().c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_lat ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_vel ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_alt ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_dir ).c_str(),
                  CubicCfgGetVStr( CUBIC_CFG_loc_fence_name, fenceId ).c_str(),
                  CubicCfgGetV( CUBIC_CFG_loc_fence_lat,  ( string )"0.0",  fenceId ).c_str(),
                  CubicCfgGetV( CUBIC_CFG_loc_fence_long, ( string )"0.0",  fenceId ).c_str(),
                  CubicCfgGetV( CUBIC_CFG_loc_fence_long, ( string )"0.0",  fenceId ).c_str(),
                  CubicCfgGetV( CUBIC_CFG_loc_fence_rad,  ( string )"0.0",  fenceId ).c_str(),
                  isEntry ? "in" : "out"  );
        snprintf( addr, PATH_MAX, "%s/clients/%s/device_events.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_uname ).c_str() );
        int ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportGeoFence request refused, http result=%d", ret );
        return 0;
    };

    static int reportEnvironment(float temperature, float pressure, float humidity) {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"events\":["
                  "{"
                  "\"type\":\"environment\","
                  "\"at\":\"%s\","
                  "\"geo\":{"
                  "\"lat\":%s,"
                  "\"long\":%s,"
                  "\"lon\":%s,"
                  "\"vel\":%s,"
                  "\"alt\":%s,"
                  "\"dir\":%s"
                  "},"
                  "\"env\":{"
                  "\"temp\":%f,"
                  "\"hum\":%f,"
                  "\"press\":%f"
                  "}"
                  "}"
                  "]"
                  "}",
                  CUtil::getTimeString().c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_lat ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_vel ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_alt ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_dir ).c_str(),
                  temperature,
                  humidity,
                  pressure );
        snprintf( addr, PATH_MAX, "%s/clients/%s/device_events.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_uname ).c_str() );
        int ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportEnvironment request refused, http result=%d", ret );
        return 0;
    };

    static int reportShock( float x, float y, float z ) {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"events\":["
                  "{"
                  "\"type\":\"shock\","
                  "\"at\":\"%s\","
                  "\"geo\":{"
                  "\"lat\":%s,"
                  "\"long\":%s,"
                  "\"lon\":%s,"
                  "\"vel\":%s,"
                  "\"alt\":%s,"
                  "\"dir\":%s"
                  "},"
                  "\"vac\":{"
                  "\"x\":%.2f,"
                  "\"y\":%.2f,"
                  "\"z\":%.2f"
                  "}"
                  "}"
                  "]"
                  "}",
                  CUtil::getTimeString().c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_lat ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_vel ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_alt ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_dir ).c_str(),
                  x, y, z );
        snprintf( addr, PATH_MAX, "%s/clients/%s/device_events.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_uname ).c_str() );
        int ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportShock request refused, http result=%d", ret );
        return 0;
    };

    static int reportBattery() {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"events\":["
                  "{"
                  "\"type\":\"battery\","
                  "\"at\":\"%s\","
                  "\"geo\":{"
                  "\"lat\":%s,"
                  "\"long\":%s,"
                  "\"lon\":%s,"
                  "\"vel\":%s,"
                  "\"alt\":%s,"
                  "\"dir\":%s"
                  "},"
                  "\"bat\":%s"
                  "}"
                  "]"
                  "}",
                  CUtil::getTimeString().c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_lat ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_vel ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_alt ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_dir ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_bat_percent ).c_str() );
        snprintf( addr, PATH_MAX, "%s/clients/%s/device_events.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_uname ).c_str() );
        int ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportBattery request refused, http result=%d", ret );
        return 0;
    };


    static int reportPowerOff() {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"events\":["
                  "{"
                  "\"type\":\"power_off\","
                  "\"at\":\"%s\","
                  "\"geo\":{"
                  "\"lat\":%s,"
                  "\"long\":%s,"
                  "\"lon\":%s,"
                  "\"vel\":%s,"
                  "\"alt\":%s,"
                  "\"dir\":%s"
                  "}"
                  "}"
                  "]"
                  "}",
                  CUtil::getTimeString().c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_lat ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_vel ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_alt ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_dir ).c_str() );
        snprintf( addr, PATH_MAX, "%s/clients/%s/device_events.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_uname ).c_str() );
        int ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportPowerOff request refused, http result=%d", ret );
        return 0;
    };


    static int reportHelp() {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"events\":["
                  "{"
                  "\"type\":\"help\","
                  "\"at\":\"%s\","
                  "\"geo\":{"
                  "\"lat\":%s,"
                  "\"long\":%s,"
                  "\"lon\":%s,"
                  "\"vel\":%s,"
                  "\"alt\":%s,"
                  "\"dir\":%s"
                  "}"
                  "}"
                  "]"
                  "}",
                  CUtil::getTimeString().c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_lat ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_long ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_vel ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_alt ).c_str(),
                  CubicStatGetStr( CUBIC_STAT_location_dir ).c_str() );
        snprintf( addr, PATH_MAX, "%s/clients/%s/device_events.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_uname ).c_str() );
        int ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportHelp request refused, http result=%d", ret );
        return 0;
    };
	
	static string getDeviceList(){
		char req[JSON_SIZE_MAX + 4] = {0};
        char resp[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        LOGD( "getDeviceList()" );
		
		snprintf( addr, PATH_MAX, "%s/device/devices.json", CubicCfgGetStr( CUBIC_CFG_push_server ).c_str() );
		int ret = sendRequest(  addr, req, resp, JSON_SIZE_MAX );
		RETNIF_LOGE( ret > 299 || ret < 200, resp, "getDeviceList request refused, http result=%d", ret );
		LOGD("resp=%s",resp);
		return resp;
	};
	
	static int uploadDeviceLog(const string &file_path) {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        int ret = 0;
        ret = uploadLogFile( file_path.c_str() );
		RETNIF_LOGE( ret != 0, ret, "uploadDeviceLog uploadLogFile failed result=%d", ret );
		ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportHelp request refused, http result=%d", ret );
        return 0;
    };
	
	static int uploadLogFile( const string &in_upload_file) {
        CURL* curl = curl_easy_init();
        RETNIF( curl == NULL, -1 );
        struct curl_slist* head_list = NULL;
        struct curl_httppost* post = NULL;
        struct curl_httppost* last = NULL;
        BufferStruct resp_buf = {NULL, 0, 0};
        char resp[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        string client_id = CubicCfgGetStr( CUBIC_CFG_push_uname );
		string serial_number = CubicCfgGetStr( CUBIC_CFG_serial_num );
        string header_client_id = "X-CLIENT-UUID: ";
		string type = "0";
        // prepare key
		snprintf( addr, PATH_MAX, "%s/uploadlog.json", CubicCfgGetStr(CUBIC_CFG_push_server ).c_str());
        header_client_id += client_id;
        // cutomize header
        head_list = curl_slist_append( head_list, header_client_id.c_str() );
        // post content
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "uuid",
                      CURLFORM_PTRCONTENTS,     client_id.c_str(),
                      CURLFORM_CONTENTSLENGTH,  client_id.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "serial_number",
                      CURLFORM_PTRCONTENTS,     serial_number.c_str(),
                      CURLFORM_CONTENTSLENGTH,  serial_number.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "type",
                      CURLFORM_PTRCONTENTS,     type.c_str(),
                      CURLFORM_CONTENTSLENGTH,  type.length(),
                      CURLFORM_END );
        curl_formadd( &post, &last,
                      CURLFORM_COPYNAME, "file",
                      CURLFORM_FILE,     in_upload_file.c_str(),
                      CURLFORM_CONTENTTYPE,  "text/plain",
                      CURLFORM_END );
        // prepare buffer
        resp_buf.buf = resp;
        resp_buf.size = 0;
        resp_buf.max = JSON_SIZE_MAX;
        // setup all options
        curl_easy_setopt( curl, CURLOPT_URL, addr );
        curl_easy_setopt( curl, CURLOPT_HTTPHEADER, head_list );
        //curl_easy_setopt( curl, CURLOPT_POST, TRUE );
        curl_easy_setopt( curl, CURLOPT_HTTPPOST, post );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteFixBufferCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, ( void* )&resp_buf );
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
        LOGD( "uploadLogFile before curl_easy_perform" );
        int ret = curl_easy_perform( curl );
        LOGD( "uploadLogFile curl_easy_perform ret=%d", ret );
        curl_formfree( post );
		
		if( ret != CURLE_OK ) {
			LOGE( "uploadLogFile error when curl_easy_perform, ret=%d", ret );
            curl_easy_cleanup( curl );
            return -1;
        }

        curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &ret );
        curl_easy_cleanup( curl );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "uploadLogFile request refused, http result=%d", ret );
       
        LOGD( "uploadLogFile sucess !" );
        return 0;
    };


    static int uploadVoiceMessage( const string &file, int64_t duration ) {
        string upload_key;
        string upload_url;
        string file_uuid;
        string file_url;
        string file_create;
        int ret = 0;
        LOGD( "uploadVoiceMessage file:%s, %lld", file.c_str(), duration );
        ret = askVMUpload( upload_url, upload_key );
        RETNIF_LOGE( ret != 0, ret, "uploadVoiceMessage askVMUpload failed result=%d", ret );
        ret = uploadVMFile( upload_url, upload_key, file, duration, file_uuid, file_url, file_create );
        RETNIF_LOGE( ret != 0, ret, "uploadVoiceMessage uploadVMFile failed result=%d", ret );
        ret = reportVM( file_uuid, file_url, upload_key );
        RETNIF_LOGE( ret != 0, ret, "uploadVoiceMessage reportVM failed result=%d", ret );
        LOGD( "uploadVoiceMessage done!" );
        return 0;
    };

    static int joinGroup( const string &group, const string &token ) {
        char req[JSON_SIZE_MAX + 4] = {0};
        char resp[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        LOGD( "joinGroup(), group:%s, token:%s", group.c_str(), token.c_str() );
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"client_uuid\":\"%s\""
                  "}",
                  CubicCfgGetStr( CUBIC_CFG_push_uname ).c_str() );
        snprintf( addr, PATH_MAX, "%s/groups/%s/candidates/%s/request_member.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  group.c_str(),
                  token.c_str() );
        int ret = sendRequest(  addr, req, resp, JSON_SIZE_MAX );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "joinGroup request refused, http result=%d", ret );
        Document resp_dom;
        RETNIF_LOGE( resp_dom.ParseInsitu( resp ).HasParseError(), -1, "joinGroup error when parse response: %s", resp );
        RETNIF_LOGE( !resp_dom.HasMember( "reason" ) ||
                     !resp_dom["reason"].IsString(), -2, "joinGroup fail, not valid reason !" );
        string reason = resp_dom["reason"].GetString();

        if( reason != "Accepted" ) {
            LOGE( "joinGroup refused by server, reson:%s", reason.c_str() );
            return -3;
        }

        LOGE( "joinGroup success add to group:%s", group.c_str() );
        CubicCfgSet( CUBIC_CFG_push_group, group );
        CubicCfgSet( CUBIC_CFG_push_group, group );
        return 0;
    };



    static string downloadVMFile( const string &url ) {
        LOGD( "downloadVMFile url=%s", url.c_str() );
        string fname = CUBIC_VOICE_MSG_CACHE"/";
        fname += CUtil::getFileNameOfPath( url );
        FILE* file = fopen( fname.c_str(), "w+" );
        RETNIF_LOGE( file == NULL, "null", "downloadVMFile failed, file can not open" );
        int ret = sendRequestFile( url, file, 0x400000 );
        LOGD( "downloadVMFile ret=%d", ret );
        fclose( file );

        if( ret < 0 ) {
            unlink( fname.c_str() );
            return "";
        }

        return fname;
    };

    static string downloadOTAFile( const string &url ) {
        LOGD( "downloadOTAFile url=%s", url.c_str() );
        string fname = CUBIC_OTA_CACHE"/";
 //       fname += CUtil::getFileNameOfPath( url );
		fname += "mamot.json";
        FILE* file = fopen( fname.c_str(), "w+" );
        RETNIF_LOGE( file == NULL, "null", "downloadOTAFile failed, file can not open" );
        int ret = sendRequestFile( url, file, 0x400000 );
        LOGD( "downloadOTAFile ret=%d", ret );
        fclose( file );

        if( ret < 0 ) {
            unlink( fname.c_str() );
            return "";
        }

        return fname;
    };

    static vector<string> getVMList( uint8_t max = 0 ) {
        static const int VM_RESPONSE_MAX = 1024 * 64; // 64k
        char resp[VM_RESPONSE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        vector<string> result;
        snprintf( addr, PATH_MAX, "%s/groups/%s/voice_messages.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_group ).c_str() );
        int ret = sendRequest(  addr, NULL, resp, VM_RESPONSE_MAX );
        RETNIF_LOGE( ret > 299 || ret < 200, result, "getVMList request refused, http result=%d", ret );
        Document resp_dom;
        RETNIF_LOGE( resp_dom.ParseInsitu( resp ).HasParseError(), result, "polling error when parse response: %s", resp );
        RETNIF_LOGE( !resp_dom.HasMember( "voice_messages" ) || !resp_dom["voice_messages"].IsArray(), result, "polling fail, not valid voice_messages !" );
        rapidjson::Value &valArray = resp_dom["voice_messages"];
        size_t i = 0;
        string self_name = CubicCfgGetStr( CUBIC_CFG_push_uname );

        if( max > 0 && valArray.Capacity() > max ) { i = valArray.Capacity() - max; }

        for( ; i < valArray.Capacity(); i++ ) {
            CONTINUEIF( !( valArray[i].HasMember( "download_url" ) ) || !( valArray[i].HasMember( "client_uuid" ) ) );
            CONTINUEIF( self_name == valArray[i]["client_uuid"].GetString() );
            result.push_back( valArray[i]["download_url"].GetString() );
        }

        return result;
    };

    static int reportNewFirmware( const string &newfirm ) {
        char req[JSON_SIZE_MAX + 4] = {0};
        char addr[PATH_MAX + 4] = {0};
        snprintf( req, JSON_SIZE_MAX,
                  "{"
                  "\"current_version\":\"%s\","
                  "\"lastest_version\":\"%s\""
                  "}",
                  CubicStatGetStr( CUBIC_CFG_version_num ).c_str(),
                  newfirm.c_str() );
        snprintf( addr, PATH_MAX, "%s/groups/%s/firmwares.json",
                  CubicCfgGetStr( CUBIC_CFG_push_server ).c_str(),
                  CubicCfgGetStr( CUBIC_CFG_push_group ).c_str() );
        int ret = sendRequest(  addr, req );
        RETNIF_LOGE( ret > 299 || ret < 200, ret, "reportNewFirmware request refused, http result=%d", ret );
        return 0;
    };

	static string updateApp(const string &versionCode , const string &fpath){
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
		if(resp_dom["result"].GetInt() == 200 && resp_dom["versionCode"].GetString() > versionCode ){
			string url = resp_dom["url"].GetString();
			LOGD("updata app, start download apk ,url=%s",url.c_str() );
			//OtaAppService::loadApk(url);
			//	downloadApk(url);
			return url;
			//return resp;
		}
		return resp;
	};

	static string downloadApk(const string &url ){
		LOGD("downloadApk ...");
		string fname = "/storage/sdcard0/Meig/";
		fname += CUtil::getFileNameOfPath( url );
		LOGD("fname path =%s",fname.c_str() );
		FILE* file = fopen( fname.c_str(), "w+" );
		RETNIF_LOGE( file == NULL, "null", "downloadApk failed, file can not open" );
		int ret = sendRequestFile( url, file, 0x400000 );
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
