#include "tsar.hpp"

#include <curl/curl.h>
#include <openssl/x509.h>

#include <format>
#include <iostream>

#include "base64.hpp"
#include "system.hpp"

constexpr auto app_id_size = 36;
constexpr auto client_key_size = 124;

namespace tsar
{
    ntp::client client::ntp{ "time.cloudflare.com", 123 };

    static size_t write_callback( void* ptr, size_t size, size_t nmemb, std::string* data )
    {
        data->append( ( char* )ptr, size * nmemb );
        return size * nmemb;
    }

    result_t< nlohmann::json > client::api_call( const std::string_view key, const std::string_view endpoint ) noexcept
    {
        const auto hwid = system::hwid();

        if ( !hwid )
            return std::unexpected( error( error_code_t::failed_to_get_hwid_t ) );

        const auto curl = curl_easy_init();

        if ( !curl )
            return std::unexpected( error( error_code_t::unexpected_error_t ) );

        auto formatted = std::format( "{}/{}", api_url, endpoint );

        // Add the HWID to the endpoint.
        formatted.append( std::format( "&hwid={}", *hwid ) );

        curl_easy_setopt( curl, CURLOPT_URL, formatted.c_str() );

        std::string response;
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, write_callback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &response );

        long status_code = 0;
        if ( curl_easy_perform( curl ) == CURLE_OK )
            curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &status_code );

        curl_easy_cleanup( curl );

        // If we are unable to make a request to the server then it is likely down. No need to do any error handling here.
        if ( !status_code )
            return std::unexpected( error( error_code_t::request_failed_t ) );

        switch ( status_code )
        {
            // This is the only status code we care about.
            case 200: break;  // OK

            case 400: return std::unexpected( error( error_code_t::bad_request_t ) );
            case 404: return std::unexpected( error( error_code_t::app_not_found_t ) );
            case 401: return std::unexpected( error( error_code_t::unauthorized_t ) );
            case 429: return std::unexpected( error( error_code_t::rate_limited_t ) );
            case 503: return std::unexpected( error( error_code_t::app_paused_t ) );
            default: return std::unexpected( error( error_code_t::server_error_t ) );
        }

        const auto json = nlohmann::json::parse( response, nullptr, false );

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
        if ( data_json[ "hwid" ].get< std::string >() != hwid->c_str() )
            return std::unexpected( error( error_code_t::hwid_mismatch_t ) );

        const auto timestamp = static_cast< time_t >( data_json[ "timestamp" ].get< uint64_t >() );
        const auto ntp_timestamp = ntp.request_time();

        if ( !ntp_timestamp )
            return std::unexpected( ntp_timestamp.error() );

        const auto system_time = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

        // Calculate the duration between the NTP timestamp and the system time.
        const auto duration = std::chrono::seconds( *ntp_timestamp > system_time ? *ntp_timestamp - system_time : system_time - *ntp_timestamp );

        // If the duration is greater than 30 seconds then we have a problem. The user's system time is not in sync with the NTP server.
        if ( duration > std::chrono::seconds( 30 ) || timestamp < ( system_time - 30u ) )
            return std::unexpected( error( error_code_t::old_response_t ) );

        if ( !verify_signature( key, *data, *signature ) )
            return std::unexpected( error( error_code_t::invalid_signature_t ) );

        if ( data_json.contains( "data" ) )
            return data_json[ "data" ];

        return data_json;
    }

    bool client::verify_signature( const std::string_view key, const std::string_view json, const std::string_view signature ) noexcept
    {
        const std::uint8_t* data = reinterpret_cast< const std::uint8_t* >( key.data() );
        std::size_t data_size = key.size();

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

    result_t< client > client::create( const std::string_view app_id, const std::string_view client_key ) noexcept
    {
        if ( app_id.length() != app_id_size )
            return std::unexpected( error( error_code_t::invalid_app_id_t ) );

        if ( client_key.length() != client_key_size )
            return std::unexpected( error( error_code_t::invalid_client_key_t ) );

        // Attempt to decode the client key from base64.
        const auto decoded = base64::safe_from_base64( client_key );

        if ( !decoded )
            return std::unexpected( error( error_code_t::failed_to_decode_public_key_t ) );

        // Make the initialization request to the server.
        const auto result = api_call( *decoded, std::format( "initialize?app_id={}", app_id ) );

        if ( !result )
            return std::unexpected( result.error() );

        // Set the dashboard hostname and exit
        const auto hostname = ( *result )[ "dashboard_hostname" ].template get< std::string >();

        return client( app_id, *decoded, hostname );
    }

    result_t< user > client::authenticate( bool open ) const noexcept
    {
        // Make the authentication request to the server.
        const auto result = api_call< user >( pub_key, std::format( "authenticate?app_id={}", app_id ) );

        if ( !result )
        {
            if ( result.error() == error_code_t::unauthorized_t && open )
            {
                // Try and get the HWID associated with the user's system.
                const auto hwid = system::hwid();

                if ( !hwid )
                    return std::unexpected( error( error_code_t::failed_to_get_hwid_t ) );

                // Open the user's default browser to prompt a login.
                if ( !system::open_browser( std::format( "https://{}/auth/{}", hostname, *hwid ) ) )
                    return std::unexpected( error( error_code_t::failed_to_open_browser_t ) );
            }

            return std::unexpected( result.error() );
        }

        return *result;
    }

    client::client( const std::string_view app_id, const std::string_view pub_key, const std::string_view hostname )
        : app_id( app_id ),
          pub_key( pub_key ),
          hostname( hostname )
    {
    }
}  // namespace tsar