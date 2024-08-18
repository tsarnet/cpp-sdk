#include <iostream>
#include <thread>

#include "tsar.hpp"

#undef max

constexpr auto app_id = "f911842b-5b3d-4c59-b5d1-4adb8f71557b";
constexpr auto client_key =
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEvJrwPvdeDUcV8Qr02tzgFrp+8qfCV/vG1HcQJYYV8u5vYUfGABMAYT0qOQltXEX9DTcB2fzLfwQnl7yiAaNruQ==";

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

    std::println( std::cout, "[+] Authentication success, welcome {}!", user->username.value_or( "N/A" ) );
    std::println( std::cout, " *  user id: {}", user->id );
    std::println( std::cout, " *  username: {}", user->username.value_or( "N/A" ) );
    std::println( std::cout, " *  avatar: {}", user->avatar.value_or( "N/A" ) );
    std::println( std::cout, " *  subscription id: {}", user->subscription.id );
    std::println( std::cout, " *  subscription expires: {}", user->subscription.expires.value_or( std::chrono::system_clock::time_point::max() ) );

    tsar::result_t< void > status;

    // Important: user->heartbeat() should be called periodically to keep the session alive.
    while ( status = user->heartbeat() )
    {
        std::println( std::cout, "[*] Heartbeat success" );
        std::this_thread::sleep_for( std::chrono::seconds( 10 ) );
    }

    error( "Heartbeat failed", status.error() );
    return 1;
}
