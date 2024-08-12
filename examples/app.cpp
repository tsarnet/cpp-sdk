#include "tsar.hpp"

#include <iostream>
#include <thread>

#undef max

constexpr auto app_id = "f911842b-5b3d-4c59-b5d1-4adb8f71557b";
constexpr auto client_key =
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEvJrwPvdeDUcV8Qr02tzgFrp+8qfCV/vG1HcQJYYV8u5vYUfGABMAYT0qOQltXEX9DTcB2fzLfwQnl7yiAaNruQ==";

int main()
{
    try
    {
        const auto client = tsar::client::create( app_id, client_key, "test" );

        const auto& subscription = client->get_subscription();

        std::println( std::cout, "[+] Authentication success, welcome {}!", subscription.user.username.value_or( "N/A" ) );
        std::println( std::cout, " *  subscription id: {}", subscription.id );
        std::println( std::cout, " *  subscription expires: {}", subscription.expires.value_or( std::chrono::system_clock::time_point::max() ) );
        std::println( std::cout, " *  user id: {}", subscription.id );
        std::println( std::cout, " *  username: {}", subscription.user.username.value_or( "N/A" ) );
        std::println( std::cout, " *  avatar: {}", subscription.user.avatar.value_or( "N/A" ) );

        while ( const auto result = client->validate() )
        {
            std::println( std::cout, "[+] Heartbeat success:" );
            std::println( std::cout, " *  hwid: {}", result.value().hwid );
            std::println( std::cout, " *  timestamp: {}", result.value().timestamp );

            std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
        }

        std::println( std::cout, "[-] Heartbeat failed. Session has expired." );
    }
    catch ( const tsar::error& e )
    {
        std::cerr << "[-] Error [" << e.code() << "]: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}