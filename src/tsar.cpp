#include "tsar.hpp"

#include <httplib.h>
#include <openssl/decoder.h>

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

            case httplib::StatusCode::BadRequest_400: return std::unexpected( error( error_code_t::bad_request_t ) );
            case httplib::StatusCode::NotFound_404: return std::unexpected( error( error_code_t::app_not_found_t ) );
            case httplib::StatusCode::Unauthorized_401: return std::unexpected( error( error_code_t::user_not_found_t ) );
            case httplib::StatusCode::TooManyRequests_429: return std::unexpected( error( error_code_t::rate_limited_t ) );
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

        const auto signature = base64::safe_from_base64( json[ "signature" ].get< std::string >() );

        if ( !signature )
            return std::unexpected( error( error_code_t::failed_to_decode_signature_t ) );

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

        const auto timestamp = static_cast< time_t >( data_json[ "timestamp" ].get< uint64_t >() );
        const auto ntp_timestamp = ntp_client.request_time();

        const auto system_time = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

        // Calculate the duration between the NTP timestamp and the system time.
        const auto duration = std::chrono::seconds( ntp_timestamp > system_time ? ntp_timestamp - system_time : system_time - ntp_timestamp );

        // If the duration is greater than 30 seconds then we have a problem. The user's system time is not in sync with the NTP server.
        if ( duration > std::chrono::seconds( 30 ) || timestamp < ( system_time - 30u ) )
            return std::unexpected( error( error_code_t::old_response_t ) );

        if ( !verify_signature( *data, *signature ) )
            return std::unexpected( error( error_code_t::invalid_signature_t ) );

        return data_json;
    }

    bool client::verify_signature( const std::string_view json, const std::string_view signature ) const noexcept
    {
        const std::uint8_t* data = reinterpret_cast< const std::uint8_t* >( pub_key.data() );
        std::size_t data_size = pub_key.size();

        // Create the EVP_PKEY structure from the public key
        EVP_PKEY* pkey = d2i_PUBKEY( nullptr, &data, static_cast< long >( data_size ) );
        if ( !pkey )
            return false;

        // Create an ECDSA_SIG structure and manually set r and s
        ECDSA_SIG* ecdsa_sig = ECDSA_SIG_new();
        if ( !ecdsa_sig )
        {
            EVP_PKEY_free( pkey );
            return false;
        }

        const auto half_len = static_cast< std::int32_t >( signature.size() / 2 );

        // Convert the signature components to BIGNUMs. We need to do this because the signature is in raw format.
        const auto r = BN_bin2bn( reinterpret_cast< const std::uint8_t* >( signature.data() ), half_len, nullptr );
        const auto s = BN_bin2bn( reinterpret_cast< const std::uint8_t* >( signature.data() + half_len ), half_len, nullptr );

        if ( !r || !s )
        {
            ECDSA_SIG_free( ecdsa_sig );
            EVP_PKEY_free( pkey );
            return false;
        }

        ECDSA_SIG_set0( ecdsa_sig, r, s );  // ECDSA_SIG now owns r and s

        // Convert ECDSA_SIG to DER format
        int der_len = i2d_ECDSA_SIG( ecdsa_sig, nullptr );
        if ( der_len <= 0 )
            return false;

        std::vector< std::uint8_t > der_sig( der_len );
        auto der_sig_ptr = der_sig.data();

        // Now we encode the ECDSA_SIG structure into DER format
        if ( i2d_ECDSA_SIG( ecdsa_sig, &der_sig_ptr ) != der_len )
        {
            ECDSA_SIG_free( ecdsa_sig );
            EVP_PKEY_free( pkey );
            return false;
        }

        // Verify the signature using the DER-encoded ECDSA signature
        EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
        if ( !mdctx )
        {
            ECDSA_SIG_free( ecdsa_sig );
            EVP_PKEY_free( pkey );
            return false;
        }

        EVP_DigestVerifyInit( mdctx, nullptr, EVP_sha256(), nullptr, pkey );
        EVP_DigestVerifyUpdate( mdctx, json.data(), json.size() );

        const auto result = EVP_DigestVerifyFinal( mdctx, der_sig.data(), der_len );

        ECDSA_SIG_free( ecdsa_sig );
        EVP_MD_CTX_free( mdctx );
        EVP_PKEY_free( pkey );

        return result == 1;
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

        c->session = ( *result )[ "session" ].get< std::string >();
        c->subscription = ( *result )[ "subscription" ].template get< subscription_t >();

        return c;
    }

    std::unique_ptr< client > client::create( const std::string_view app_id, const std::string_view client_key )
    {
        return create( app_id, client_key, std::format( "{}.tsar.app", app_id ) );
    }

    bool client::validate()
    {
        const auto result = query( std::format( "validate?app={}&session={}&hwid={}", app_id, session, hwid ) );

        if ( !result )
        {
            switch ( static_cast< error_code_t >( result.error().code().value() ) )
            {
                case error_code_t::unauthorized_t: return false;

                default: throw result.error();
            }
        }

        return true;
    }

    client::client( const std::string_view app_id, const std::string_view client_key, const std::string_view hwid )
        : app_id( app_id ),
          hwid( hwid ),
          http_client( "https://tsar.cc" ),
          ntp_client( "time.cloudflare.com", 123 )
    {
        if ( const auto k = base64::safe_from_base64( client_key ) )
            pub_key = *k;
        else
            throw error( error_code_t::failed_to_decode_public_key_t );
    }

    void from_json( const nlohmann::json& j, user_t& u )
    {
        j.at( "id" ).get_to( u.id );

        if ( !j[ "username" ].is_null() )
            u.username = j.at( "username" ).get< std::string >();
        else
            u.username = std::nullopt;

        if ( !j[ "avatar" ].is_null() )
            u.avatar = j.at( "avatar" ).get< std::string >();
        else
            u.avatar = std::nullopt;
    }

    void from_json( const nlohmann::json& j, subscription_t& s )
    {
        j.at( "id" ).get_to( s.id );

        if ( !j[ "expires" ].is_null() )
            s.expires = std::chrono::system_clock::from_time_t( j.at( "expires" ).get< uint64_t >() );
        else
            s.expires = std::nullopt;

        j.at( "user" ).get_to( s.user );
        j.at( "tier" ).get_to( s.tier );
    }
}  // namespace tsar