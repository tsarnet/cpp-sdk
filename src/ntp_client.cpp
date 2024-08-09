#include "ntp_client.hpp"

#include <stdio.h>
#include <sys/types.h>

#include <cstring>
#include <iostream>
#ifdef _WIN32
#include <WinSock2.h>
#define close( X ) closesocket( X )
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>

NTPClient::NTPClient( std::string hostname, uint16_t port ) : hostname_( hostname ), port_( port ), socket_fd( 0 ), socket_client{}
{
#ifdef _WIN32
    WSADATA wsaData = { 0 };
    ( void )WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
#endif;
}

void NTPClient::build_connection()
{
    // Creating socket file descriptor
    if ( ( socket_fd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
        return;

    memset( &socket_client, 0, sizeof( socket_client ) );

    const auto ntp_server_ip = hostname_to_ip( hostname_ );

    timeval timeout_time_value{};
    timeout_time_value.tv_sec = 5;  // timeout in seconds
    setsockopt( socket_fd, SOL_SOCKET, SO_RCVTIMEO, ( const char * )&timeout_time_value, sizeof( timeout_time_value ) );

    // Filling server information
    socket_client.sin_family = AF_INET;
    socket_client.sin_port = htons( port_ );
    socket_client.sin_addr.s_addr = inet_addr( ntp_server_ip.c_str() );
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

    if ( connect( socket_fd, ( struct sockaddr * )&socket_client, sizeof( socket_client ) ) < 0 )
        return 0;

    NTPPacket packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    packet.li_vn_mode = 0x1b;

#ifdef _WIN32
    response = sendto( socket_fd, ( char * )&packet, sizeof( NTPPacket ), 0, ( struct sockaddr * )&socket_client, sizeof( socket_client ) );
#else
    response = write( socket_fd, ( char * )&packet, sizeof( NTPPacket ) );
#endif

    if ( response < 0 )
        return 0;

    // reset the packet buffer
    memset( &packet, 0, sizeof( NTPPacket ) );

#ifdef _WIN32
    response = recv( socket_fd, ( char * )&packet, sizeof( NTPPacket ), 0 );
#else
    response = read( socket_fd, ( char * )&packet, sizeof( NTPPacket ) );
#endif

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
    hostent *hostname = gethostbyname( host.c_str() );
    if ( hostname )
        return std::string( inet_ntoa( **( in_addr ** )hostname->h_addr_list ) );
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