#include "tsar.hpp"

#include <iostream>
#include <thread>

#undef max

constexpr auto app_id = "35f5bc1b-9a6d-442f-8813-6579e443d4be";
constexpr auto client_key =
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEJ+N6c07hrVdt1FzX3+baICfl7nDIWJ+uMEyN5SA09ulN37mc7maQ8kOBwse9Yr4To5FIKMMx/BQwDTPu+TqvVQ==";

static void error( const std::string_view title, const tsar::error& err ) noexcept
{
    std::cerr << title << " [" << err.code() << "]: " << err.what() << std::endl;
}

int main()
{
    const auto client = tsar::client::create( app_id, client_key );

    if ( !client )
	{
        error( "Failed to create client", client.error() );
		return 1;
	}

    return 0;
}