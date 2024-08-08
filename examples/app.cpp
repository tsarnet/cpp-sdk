#include <iostream>

#include "tsar.hpp"

int main()
{
    if ( const auto client = tsar::client::create( "f911842b-5b3d-4c59-b5d1-4adb8f71557b", "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEvJrwPvdeDUcV8Qr02tzgFrp+8qfCV/vG1HcQJYYV8u5vYUfGABMAYT0qOQltXEX9DTcB2fzLfwQnl7yiAaNruQ==" ); !client.has_value() )
    {
        std::cerr << "Error [" << client.error().code() << "]: " << client.error().what() << std::endl;
        return 1;
    }

    return 0;
}