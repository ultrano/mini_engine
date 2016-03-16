#include "MNPrimaryType.h"
#include "MNLog.h"
#include <fcntl.h>

#ifdef PLATFORM_WIN32

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <io.h>
struct _WinSockHolder
{
    _WinSockHolder()
    {
        WSADATA wsaData;
        int retval = WSAStartup(MAKEWORD(2,2),&wsaData);
    }
    ~_WinSockHolder() { WSACleanup(); }
};
#define WINSOCKHOLDER static _WinSockHolder _winsockholder;

#else

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <unistd.h>
#define WINSOCKHOLDER 1;

#endif

THashString::THashString()
	: m_hash(0)
{
}

THashString::THashString(const tstring& str)
	: m_str(str)
{
	m_hash = makeHash(m_str);
}

thash32 THashString::makeHash(const tstring& str)
{
	return makeHash(&str[0], str.length());
}

thash32 THashString::makeHash(const char* str, tsize len)
{
	thash32 b = 378551;
	thash32 a = 63689;
	thash32 hash = 0;

	for (std::size_t i = 0; i < len; i++)
	{
		hash = hash * a + str[i];
		a = a * b;
	}

	return (hash & 0x7FFFFFFF);
}

TSocket::TSocket()
{
    WINSOCKHOLDER;
    sock = socket( AF_INET, SOCK_STREAM, 0 );
    signal(SIGPIPE, SIG_IGN);
    connected = false;
}

void TSocket::connect(const tstring& ip, int port, bool blocking)
{
    
    sockaddr_in serv = {0};
    serv.sin_family  = AF_INET;
    serv.sin_port    = htons( port );
    serv.sin_addr.s_addr = inet_addr( ip.c_str() );
    
    int ret = ::connect( sock, (sockaddr*)&serv, sizeof( serv ) );
    connected = ( ret == 0 );
    if ( connected == false )
        MNLog( "fail to connect" );
    else
    {
        bool ret = false;
#ifdef PLATFORM_WIN32
        unsigned long mode = blocking ? 0 : 1;
        ret = (ioctlsocket(pimpl->sock, FIONBIO, &mode) == 0);
#else
        int flags = fcntl( sock, F_GETFL, 0);
        if (flags < 0) return ;
        
        flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
        ret = (fcntl(sock, F_SETFL, flags) == 0);
#endif
        if ( !ret )
        {
            MNLog( blocking? "fail to block socket":"fail to non-block socket" );
        }
    }
}

void TSocket::close()
{
    if (sock != 0)
    {
#ifdef PLATFORM_WIN32
        closesocket( sock );
#else
        ::close( sock );
#endif
    }
}

bool TSocket::sendBuffer(tbyte* buf, tint32 len)
{
    if ( buf == NULL ) return false;
    if ( len == 0 ) return false;
    if ( isConnected() == false ) return false;
    
    len = send( sock, (const void*)buf, len, 0);
    if (len < 0) connected = false;
    
    return ( len > 0 );
}

bool TSocket::readBuffer(tbyte* buf, tint32 len)
{
    if ( buf == NULL ) return false;
    if ( len == 0 ) return false;
    if ( isConnected() == false ) return false;
    
    len = recv(sock, (void*)&buf[0], len, 0 );
    if (len == 0) connected = false;
    return (len > 0);
}

bool TSocket::isConnected()
{
    return connected;
}