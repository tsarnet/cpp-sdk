#include "tsar.hpp"

#include <httplib.h>
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <format>
#include <print>

#include "base64.hpp"
#include "ntp_client.hpp"
#include "system.hpp"

namespace tsar
{
    result_t< nlohmann::json > client::query( const std::string_view endpoint ) noexcept
    {
        const auto result = http_client.Get( std::format( "/api/client/{}", endpoint ) );

        // If we are unable to make a request to the server then it is likely down. No need to do any error handling here.
        if ( !result )
            return std::unexpected( error( error_code_t::request_failed_t ) );

        switch ( result->status )
        {
            // This is the only status code we care about.
            case httplib::StatusCode::OK_200: break;

            case httplib::StatusCode::NotFound_404: return std::unexpected( error( error_code_t::app_not_found_t ) );
            case httplib::StatusCode::Unauthorized_401: return std::unexpected( error( error_code_t::user_not_found_t ) );
            case httplib::StatusCode::ServiceUnavailable_503: return std::unexpected( error( error_code_t::app_paused_t ) );
            default: return std::unexpected( error( error_code_t::server_error_t ) );
        }

        const auto json = nlohmann::json::parse( result->body, nullptr, false );

        if ( json.is_discarded() )
            return std::unexpected( error( error_code_t::failed_to_parse_body_t ) );

        if ( !json[ "data" ].is_string() )
            return std::unexpected( error( error_code_t::failed_to_get_data_t ) );

        if ( !json[ "signature" ].is_string() )
            return std::unexpected( error( error_code_t::failed_to_get_signature_t ) );

        const auto signature = base64::from_base64( json[ "signature" ].get< std::string >() );
        const auto data = base64::safe_from_base64( json[ "data" ].get< std::string >() );

        if ( !data )
            return std::unexpected( error( error_code_t::failed_to_decode_data_t ) );

        const auto data_json = nlohmann::json::parse( *data, nullptr, false );

        if ( data_json.is_discarded() )
            return std::unexpected( error( error_code_t::failed_to_parse_body_t ) );

        if ( !data_json[ "hwid" ].is_string() )
            return std::unexpected( error( error_code_t::failed_to_parse_data_t ) );

        if ( !data_json[ "timestamp" ].is_number_unsigned() )
            return std::unexpected( error( error_code_t::failed_to_get_timestamp_t ) );

        // Verify that the HWID matches the user's HWID.
        if ( data_json[ "hwid" ].get< std::string >() != hwid.c_str() )
            return std::unexpected( error( error_code_t::hwid_mismatch_t ) );

        const auto timestamp = data_json[ "timestamp" ].get< uint64_t >();
        const auto ntp_timestamp = ntp_client.request_time();

        const auto system_time = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

        // Calculate the duration between the NTP timestamp and the system time.
        const auto duration = std::chrono::seconds( ntp_timestamp > system_time ? ntp_timestamp - system_time : system_time - ntp_timestamp );

        // If the duration is greater than 30 seconds then we have a problem. The user's system time is not in sync with the NTP server.
        if ( duration > std::chrono::seconds( 30 ) || timestamp < ( system_time - 30u ) )
            return std::unexpected( error( error_code_t::old_response_t ) );

        if ( !verify_signature( *data, signature ) )
            return std::unexpected( error( error_code_t::invalid_signature_t ) );

        return data_json;
    }

    bool client::verify_signature( const std::string_view json, const std::string_view signature ) const noexcept
    {
        // Compute the SHA256 hash of the JSON data.
        const auto digest = SHA256( reinterpret_cast< const std::uint8_t* >( json.data() ), json.length(), nullptr );

        // Create the EC_KEY object from the client key.
        const auto bio = BIO_new_mem_buf( client_key.data(), client_key.length() );
        const auto key = PEM_read_bio_EC_PUBKEY( bio, nullptr, nullptr, nullptr );

        return ECDSA_verify(
                   0, digest, SHA256_DIGEST_LENGTH, reinterpret_cast< const std::uint8_t* >( signature.data() ), signature.length(), key ) ==
               1;
    }

    std::unique_ptr< client > client::create( const std::string_view app_id, const std::string_view client_key, const std::string_view hostname )
    {
        const auto hwid = system::hwid();

        if ( !hwid )
            throw error( error_code_t::failed_to_get_hwid_t );

        std::unique_ptr< client > c( new client( app_id, client_key, *hwid ) );

        const auto result = c->query( std::format( "initialize?app={}&hwid={}", app_id, *hwid ) );

        if ( !result )
        {
            switch ( static_cast< error_code_t >( result.error().code().value() ) )
            {
                case error_code_t::user_not_found_t:
                {
                    // If the user is not found then we open the browser to authenticate. They will authenticate with their HWID and restart the
                    // application.
                    if ( !system::open_browser( std::format( "https://{}/auth/{}", hostname, *hwid ) ) )
                        throw error( error_code_t::failed_to_open_browser_t );

                    throw error( error_code_t::unauthorized_t );
                }

                default: throw result.error();
            }
        }

        std::cout << result->dump( 4 ) << std::endl;

        return c;
    }

    std::unique_ptr< client > client::create( const std::string_view app_id, const std::string_view client_key )
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
          http_client( "https://tsar.cc" ),
          ntp_client( "time.cloudflare.com", 123 )
    {
    }
}  // namespace tsar