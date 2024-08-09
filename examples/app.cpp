#include <iostream>

#include "tsar.hpp"

constexpr auto app_id = "f911842b-5b3d-4c59-b5d1-4adb8f71557b";
constexpr auto client_key =
    R"(
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEvJrwPvdeDUcV8Qr02tzgFrp+8qfCV/vG1HcQJYYV8u5vYUfGABMAYT0qOQltXEX9DTcB2fzLfwQnl7yiAaNruQ==
-----END PUBLIC KEY-----
)";

int main()
{
    try
    {
        const auto client = tsar::client::create( app_id, client_key );
    }
    catch ( const tsar::error& e )
    {
        std::cerr << "Error [" << e.code() << "]: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}