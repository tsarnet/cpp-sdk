#include "ntp_client.hpp"

#include <sys/types.h>

#include <cstring>
#include <iostream>
#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>  // For getaddrinfo and inet_ntop
#define close( X ) closesocket( X )
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>

#include <chrono>

NTPClient::NTPClient( std::string hostname, uint16_t port ) : hostname_( hostname ), port_( port ), socket_fd( 0 ), socket_client{}
{
#ifdef _WIN32
    WSADATA wsaData = { 0 };
    ( void )WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
#endif
}

void NTPClient::build_connection()
{
    // Creating socket file descriptor
    if ( ( socket_fd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
        return;

    memset( &socket_client, 0, sizeof( socket_client ) );

    const auto ntp_server_ip = hostname_to_ip( hostname_ );

    if ( ntp_server_ip.empty() )
        return;

    //timeval timeout_time_value{};
    //timeout_time_value.tv_sec = 30;  // timeout in seconds
    //setsockopt( socket_fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast< const char * >( &timeout_time_value ), sizeof( timeout_time_value ) );
    //setsockopt( socket_fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast< const char * >( &timeout_time_value ), sizeof( timeout_time_value ) );

    // Filling server information
    socket_client.sin_family = AF_INET;
    socket_client.sin_port = htons( port_ );
    inet_pton( AF_INET, ntp_server_ip.c_str(), &socket_client.sin_addr );
}

NTPClient::~NTPClient()
{
    close_socket();

#ifdef _WIN32
    WSACleanup();
#endif
}

time_t NTPClient::request_time()
{
    int response;  // return result from writing/reading from the socket

    build_connection();

    if ( connect( socket_fd, reinterpret_cast< struct sockaddr * >( &socket_client ), sizeof( socket_client ) ) < 0 )
        return 0;

    NTPPacket packet{};
    packet.li_vn_mode = 0x23;

    response = sendto(
        socket_fd,
        reinterpret_cast< const char * >( &packet ),
        sizeof( NTPPacket ),
        0,
        reinterpret_cast< struct sockaddr * >( &socket_client ),
        sizeof( socket_client ) );
    if ( response < 0 )
        return 0;

    // reset the packet buffer
    memset( &packet, 0, sizeof( NTPPacket ) );
    
    response = recvfrom( socket_fd, reinterpret_cast< char * >( &packet ), sizeof( NTPPacket ), 0, nullptr, nullptr );
    if ( response < 0 )
    {
        close_socket();
        return 0;
    }

    // These two fields contain the time-stamp seconds as the packet left the NTP
    // server. The number of seconds correspond to the seconds passed since 1900.
    // ntohl() converts the bit/byte order from the network's to host's
    // "endianness".

    packet.transmited_timestamp_sec = ntohl( packet.transmited_timestamp_sec );            // Time-stamp seconds.
    packet.transmited_timestamp_sec_frac = ntohl( packet.transmited_timestamp_sec_frac );  // Time-stamp fraction of a second.

    // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch)
    // from when the packet left the server. Subtract 70 years worth of seconds
    // from the seconds since 1900. This leaves the seconds since the UNIX epoch
    // of 1970.
    // (1900)---------(1970)**********(Time Packet Left the Server)

    // seconds since UNIX epoch
    uint32_t txTm = packet.transmited_timestamp_sec - NTP_TIMESTAMP_DELTA;

    return txTm;
}

std::string NTPClient::hostname_to_ip( const std::string &host )
{
    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM;  // Datagram socket (UDP)

    int status = getaddrinfo( host.c_str(), nullptr, &hints, &res );
    if ( status != 0 )
    {
        std::cerr << "Error resolving hostname: " << gai_strerror( status ) << std::endl;
        return {};
    }

    char ip_str[ INET_ADDRSTRLEN ];
    if ( res )
    {
        inet_ntop( AF_INET, &( reinterpret_cast< sockaddr_in * >( res->ai_addr )->sin_addr ), ip_str, INET_ADDRSTRLEN );
        freeaddrinfo( res );
        return std::string( ip_str );
    }

    return {};
}

void NTPClient::close_socket()
{
    if ( socket_fd != -1 )
    {
        close( socket_fd );
        socket_fd = -1;
    }
}
