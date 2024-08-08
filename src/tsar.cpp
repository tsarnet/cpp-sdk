#include "tsar.hpp"

#include <httplib.h>

#include <format>
#include <print>

#include "system.hpp"

namespace tsar
{
    nlohmann::json client::query( const std::string_view endpoint )
    {
        const auto result = http_client.Get( endpoint.data() );

        if ( !result )
        {
        }

        std::cout << result.value().body << std::endl;

        return result.value().body;
    }

    result_t< std::unique_ptr< client > >
    client::create( const std::string_view app_id, const std::string_view client_key, const std::string_view hostname ) noexcept
    {
        const auto hwid = system::hwid();

        if ( !hwid )
            return std::unexpected( error( error_code_t::failed_to_get_hwid_t ) );

        std::unique_ptr< client > c( new client( app_id, client_key, *hwid ) );
        std::cout << c->query( "/api/client/initialize?app={}&hwid={}", app_id, *hwid ) << std::endl;
        return c;
    }

    result_t< std::unique_ptr< client > > client::create( const std::string_view app_id, const std::string_view client_key ) noexcept
    {
        return create( app_id, client_key, std::format( "{}.tsar.app", app_id ) );
    }

    result_t< validation_data_t > client::validate()
    {
        return result_t< validation_data_t >();
    }

    client::client( const std::string_view app_id, const std::string_view client_key, const std::string_view hwid )
        : app_id( app_id ),
          client_key( client_key ),
          hwid( hwid ),
          http_client( "https://tsar.cc" )
    {
    }
}  // namespace tsar