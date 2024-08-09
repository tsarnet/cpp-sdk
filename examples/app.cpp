#include <iostream>

#include "tsar.hpp"

constexpr auto app_id = "f911842b-5b3d-4c59-b5d1-4adb8f71557b";
constexpr auto client_key =
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEvJrwPvdeDUcV8Qr02tzgFrp+8qfCV/vG1HcQJYYV8u5vYUfGABMAYT0qOQltXEX9DTcB2fzLfwQnl7yiAaNruQ==";

int main()
{
    try
    {
        const auto client = tsar::client::create( app_id, client_key );

        const auto& subscription = client->get_subscription();

        std::println( std::cout, "[+] Authentication success, welcome {}!", subscription.user.username.value_or( "N/A" ) );

        while ( client->validate() )
        {
            std::println( std::cout, "[+] Heartbeat success" );

            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
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