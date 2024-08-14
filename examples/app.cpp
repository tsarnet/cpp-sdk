#include <iostream>
#include <thread>

#include "tsar.hpp"

#undef max

constexpr auto app_id = "35f5bc1b-9a6d-442f-8813-6579e443d4be";
constexpr auto client_key =
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEJ+N6c07hrVdt1FzX3+baICfl7nDIWJ+uMEyN5SA09ulN37mc7maQ8kOBwse9Yr4To5FIKMMx/BQwDTPu+TqvVQ==";

static void error( const std::string_view title, const tsar::error& err ) noexcept
{
    std::cerr << "[-] " << title << " [" << err.code() << "]: " << err.what() << std::endl;
}

int main()
{
    const auto client = tsar::client::create( app_id, client_key );

    if ( !client )
    {
        error( "Failed to create client", client.error() );
        return 1;
    }

    std::println( std::cout, "[*] Checking for authorization" );

    auto user = client->authenticate();

    while ( !user )
    {
        if ( user.error() != tsar::error_code_t::unauthorized_t )
        {
            error( "Failed to authenticate", user.error() );
            return 1;
        }

        std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
        user = client->authenticate( false );
    }

    std::println( std::cout, "[+] Authentication success, welcome {}!", user->name.value_or( "N/A" ) );
    std::println( std::cout, " *  user id: {}", user->id );
    std::println( std::cout, " *  username: {}", user->name.value_or( "N/A" ) );
    std::println( std::cout, " *  avatar: {}", user->avatar.value_or( "N/A" ) );
    std::println( std::cout, " *  subscription id: {}", user->subscription.id );
    std::println( std::cout, " *  subscription expires: {}", user->subscription.expires.value_or( std::chrono::system_clock::time_point::max() ) );
    return 0;
}