#ifndef _SIP_SERVICE_CC_
#define _SIP_SERVICE_CC_ 1

#include "cubic_inc.h"

#include "CFramework.cc"
#include "CThread.cc"

#include "CRingBuffer.cc"
#include "SipCaller.cc"
#include "CStringTool.cc"


#ifdef CUBIC_LOG_TAG
#undef CUBIC_LOG_TAG
#endif //CUBIC_LOG_TAG
#define CUBIC_LOG_TAG "SipService"


using namespace std;

class SipService : public ICubicApp, public ISipUser
{
private:
    int         m_fd_vioce_pipe_src;    // to user
    int         m_fd_vioce_pipe_sink;   // from user
    CRingBuffer m_ring_buffer;


public:
    SipService()
        : m_fd_vioce_pipe_src( -1 )
        , m_fd_vioce_pipe_sink( -1 )
        , m_ring_buffer(SipCaller::SOUND_FRAME_SIZE*10)
    {
        SipCaller::GetInstance()->regUser( this );

        do{
            unlink( CUBIC_SIP_VOICE_PIPE_SRC );
            mkfifo( CUBIC_SIP_VOICE_PIPE_SRC, 0777 );

            unlink( CUBIC_SIP_VOICE_PIPE_SINK );
            mkfifo( CUBIC_SIP_VOICE_PIPE_SINK, 0777 );

            m_fd_vioce_pipe_src = open( CUBIC_SIP_VOICE_PIPE_SRC, O_RDWR|O_NONBLOCK );
            BREAKIF_LOGE( m_fd_vioce_pipe_src <= 0, "can not open voice pipe for srouce, %d, %s", m_fd_vioce_pipe_src, strerror(errno) );

            m_fd_vioce_pipe_sink = open( CUBIC_SIP_VOICE_PIPE_SINK, O_RDWR|O_NONBLOCK );
            BREAKIF_LOGE( m_fd_vioce_pipe_sink <= 0, "can not open voice pipe for sink, %d, %s", m_fd_vioce_pipe_sink, strerror(errno) );
        }while( false );
    };

    ~SipService() {
        if( m_fd_vioce_pipe_src > 0 )
        {
            close(m_fd_vioce_pipe_src);
            m_fd_vioce_pipe_src = -1;
        }

        if( m_fd_vioce_pipe_sink > 0 )
        {
            close(m_fd_vioce_pipe_sink);
            m_fd_vioce_pipe_sink = -1;
        }

        unlink( CUBIC_SIP_VOICE_PIPE_SRC );
        unlink( CUBIC_SIP_VOICE_PIPE_SINK );
    };

    // **************************** call as ICubicApp ********************************
    virtual bool onInit() {
        LOGD( "%s onInit: %d", CUBIC_THIS_APP, getpid() );

        if( CubicStatGetI(CUBIC_STAT_net_connected) == 1 )
        {
            CubicPost( CUBIC_THIS_APP, CUBIC_MSG_SIP_REGISTER );
        }
        LOGE( "SipService onInit success" );
        return true;
    };

    virtual void onDeInit() {
        SipCaller::GetInstance()->inputEvent( SipCaller::EVT_DEINIT );
        return;
    };
	
	virtual bool onStunInit(){
		SipCaller::evt_init arg;
		memset( &arg, 0, sizeof( arg ) );
		LOGE( "onStunInit.init" );
		string proxy = CubicCfgGetStr( CUBIC_CFG_sip_protocol );

		if( proxy == "SCTP" ) {
			arg.transport_type = PJSIP_TRANSPORT_SCTP;
		}
		else if( proxy == "UDP" ) {
			arg.transport_type = PJSIP_TRANSPORT_UDP;
		}
		else if( proxy == "TLS" ) {
			arg.transport_type = PJSIP_TRANSPORT_TLS;
		}
		else {
			arg.transport_type = PJSIP_TRANSPORT_TCP;
		}

		string def_stun = CubicCfgGet( CUBIC_CFG_sip_defailt_stun_addr, ( string )"127.0.0.1" );
		strncpy( arg.stun, def_stun.c_str(), SipCaller::ARG_STR_LEN );
		RETNIF_LOGE( 0 != SipCaller::GetInstance()->inputEvent( SipCaller::EVT_INIT, &arg ),
					 false, "SipService onInit FAILED, caller init failed" );
		// get config of stun server
		vector<string> stun_configs = CubicCfgEnum( CUBIC_CFG_sip_stun );
		vector<string> stun_servers;

		for( size_t i = 0; i < stun_configs.size(); i++ ) {
			string stun_server = CubicCfgGetV( CUBIC_CFG_sip_stun_addr, ( string )"127.0.0.1", stun_configs[i].c_str() );
			CONTINUEIF( stun_server.length() <= 0 || stun_server == "null" );
			stun_servers.push_back( stun_server );
		}

		SipCaller::GetInstance()->setStunServers( stun_servers );
		LOGE( "onStunInit. success" );
		return true;
	}

    virtual int onMessage( const string &str_src_app_name, int n_msg_id, const void* p_data ) {
        RETNIF_LOGE( SipCaller::GetInstance() == NULL, -1, "SipService::onMessage error SipCaller is null !" );

        switch( n_msg_id ) {
        case CUBIC_MSG_SIP_REGISTER:
            LOGD( "SipService::CUBIC_MSG_SIP_REGISTER" );
            {
				onStunInit();
				
                SipCaller::evt_register arg;
                string protocol     = CubicCfgGetStr( CUBIC_CFG_sip_protocol );
                string uname        = CubicCfgGetStr( CUBIC_CFG_sip_uname );
                string upswd        = CubicCfgGetStr( CUBIC_CFG_sip_upswd );
                string domain       = CubicCfgGetStr( CUBIC_CFG_sip_domain );
                string proxy        = CubicCfgGetStr( CUBIC_CFG_sip_proxy );
                string registrar    = CubicCfgGetStr( CUBIC_CFG_sip_registrar );
                uint16_t mport_start = CubicCfgGet( CUBIC_CFG_sip_mport_start, ( uint16_t )0 );
                uint16_t mport_end  = CubicCfgGet( CUBIC_CFG_sip_mport_end, ( uint16_t )0 );
                string proto_head;
                string transport_type;
                memset( &arg, 0, sizeof( arg ) );

                if( protocol == "UDP" ) {
                    proto_head = "sip";
                    transport_type = "udp";
                }
                else if( protocol == "TLS" ) {
                    proto_head = "sips";
                    transport_type = "tcp";
                }
                else {
                    proto_head = "sip";
                    transport_type = "tcp";
                }

                snprintf( arg.id, SipCaller::ARG_STR_LEN,
                          "%s:%s@%s;transport=%s", proto_head.c_str(), uname.c_str(), domain.c_str(), transport_type.c_str() );
                snprintf( arg.registrar, SipCaller::ARG_STR_LEN,
                          "%s:%s;transport=%s", proto_head.c_str(), registrar.c_str(), transport_type.c_str() );
                snprintf( arg.realm, SipCaller::ARG_STR_LEN, "*" );
                snprintf( arg.username, SipCaller::ARG_STR_LEN, "%s", uname.c_str() );
                snprintf( arg.password, SipCaller::ARG_STR_LEN, "%s", upswd.c_str() );
                snprintf( arg.proxy, SipCaller::ARG_STR_LEN,
                          "%s:%s;transport=%s", proto_head.c_str(), proxy.c_str(), transport_type.c_str() );
                arg.port = mport_start;
                arg.port_range = mport_end - mport_start;

                if( CStatMachine::ERR_NO_ERROR !=  SipCaller::GetInstance()->inputEvent( SipCaller::EVT_REGISTER, &arg ) ) {
                    CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_REGISTER_LOST );
                }
            }
            break;

        case CUBIC_MSG_SIP_CALL_DIAL:
            LOGD( "SipService::CUBIC_MSG_SIP_CALL_DIAL" );
            {
                cubic_msg_sip_call_dial* data = ( cubic_msg_sip_call_dial* )p_data;
                BREAKIF_LOGE( data == NULL, "SipService::onMessage error argument is null !" );
                SipCaller::evt_dial arg;
                memset( &arg, 0, sizeof( arg ) );
                strncpy( arg.uri, data->peer_uuid, SipCaller::ARG_STR_LEN );

                if( CStatMachine::ERR_NO_ERROR !=  SipCaller::GetInstance()->inputEvent( SipCaller::EVT_DAIL, &arg ) ) {
                    CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_CALL_HANGUPED );
                }
            }
            break;

        case CUBIC_MSG_SIP_CALL_ACCEPT:
            LOGD( "SipService::CUBIC_MSG_SIP_CALL_ACCEPT" );

            if( CStatMachine::ERR_NO_ERROR !=  SipCaller::GetInstance()->inputEvent( SipCaller::EVT_ACCEPT ) ) {
                CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_CALL_HANGUPED );
            }

            break;

        case CUBIC_MSG_SIP_CALL_HANGUP:
            LOGD( "SipService::CUBIC_MSG_SIP_CALL_HANGUP" );
            SipCaller::GetInstance()->inputEvent( SipCaller::EVT_HANGUP );
            break;

        default:
            break;
        };

        return 0;
    };


    // **************************** call as ISipUser ********************************

    virtual int onIncommingCall(
        const string &wsse,
        const string &caller,
        const string &pstn_number,
        const string &recordid,
        const string &push_call ) {
        LOGD( "SipService::onIncommingCall(caller=%s)", caller.c_str() );
        CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_CALL_RECEIVED );
        return 0;
    };

    virtual int onAccept( string info ) {
        LOGD( "SipService::onAccept(%s)", info.c_str() );
        CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_CALL_ACCEPTED );
        return 0;
    };

    virtual int onHangup() {
        LOGD( "SipService::onHangup()" );
        CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_CALL_HANGUPED );
        return 0;
    };

    virtual int onDTMF( int code ) {
        LOGD( "SipService::onDTMF(code=%d)", code );
        return 0;
    };

    virtual int onStatChange( int stat ) {
        LOGD( "SipService::onStatChange(stat=%d)", stat );
        CubicStatSet( CUBIC_STAT_sip_stat, stat );
        int registered = 0;

        if( stat > SipCaller::STAT_REGISTERING ) {
            registered = 1;
        }

        RETNIF( registered == CubicStatGetI( CUBIC_STAT_sip_registered ), 0 );
        CubicStatSet( CUBIC_STAT_sip_registered, registered );

        if( registered == 0 ) {
            CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_REGISTER_LOST );
        }
        else {
            CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_REGISTER_SUCCESS );
        }

        return 0;
    };


    virtual int getSoundFrame( void* data, int size )
    {
        uint8_t buf[SipCaller::SOUND_FRAME_SIZE + 4];
        int ret = 0;

        RETNIF( m_fd_vioce_pipe_sink < 0, -1 );
        memset(buf, 0, sizeof(buf));
        if( (ret = read(m_fd_vioce_pipe_sink, buf, SipCaller::SOUND_FRAME_SIZE)) > 0 )
        {
            m_ring_buffer.put( buf, ret );
        }

        RETNIF( m_ring_buffer.remain() < size, 0 );
        m_ring_buffer.get( data, size );
        return size;
    };

    virtual int putSoundFrame( const void* data, int size )
    {
        RETNIF( m_fd_vioce_pipe_src < 0, -1 );
        return write( m_fd_vioce_pipe_src, data, size );
    };

    virtual int onRegCode( int code ) {
        LOGD( "SipService::onRegCode(code=%d)", code );
        CubicStatSet( CUBIC_STAT_sip_reg_code, code );
        return 0;
    };

    virtual int onCallResult( int code ) {
        LOGD( "SipService::onCallResult(code=%d)", code );
        CubicStatSet( CUBIC_STAT_sip_result, code );
        return 0;
    }

    virtual int onNotifyActive() {
        LOGD( "SipService::onNotifyActive" );
        CubicPost( CUBIC_APP_NAME_CORE, CUBIC_MSG_KEEP_TRACKING );
        return 0;
    };

    virtual int onNotifyVoiceMessage(    const string &group_id, const string &from_client,
                                         const string &created, const string &url ) {
        cubic_msg_vm_received arg;
        LOGD( "SipService::onNotifyVoiceMessage" );
        memset( &arg, 0, sizeof( arg ) );
        strncpy( arg.group_uuid,        group_id.c_str(),       CUBIC_UUID_LEN_MAX );
        strncpy( arg.sender_uuid,       from_client.c_str(),    CUBIC_UUID_LEN_MAX );
        strncpy( arg.created,           created.c_str(),        CUBIC_TIME_LEN_MAX );
        strncpy( arg.voice_message_url, url.c_str(),            CUBIC_URL_LEN_MAX );
        CubicPostReq( CUBIC_APP_NAME_VM_SERVICE, CUBIC_MSG_VM_RECEIVED, arg );
        return 0;
    };

};

IMPLEMENT_CUBIC_APP( SipService );

#endif

