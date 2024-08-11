#include "error.hpp"

namespace tsar
{
    error::error( std::error_code code ) noexcept : std::system_error( code )
    {
    }

    error::error( error_code_t code ) noexcept : error( { static_cast< std::int32_t >( code ), error_category::get() } )
    {
    }

    const error_category& error_category::get() noexcept
    {
        static error_category instance;
        return instance;
    }

    const char* error_category::name() const noexcept
    {
        return "tsar";
    }

    std::string error_category::message( int ev ) const
    {
        switch ( static_cast< error_code_t >( ev ) )
        {
            case error_code_t::failed_to_get_hwid_t: return "Failed to get the user's HWID.";
            case error_code_t::failed_to_open_browser_t: return "Failed to open browser.";
            case error_code_t::unauthorized_t: return "User is not authorized to use the application.";
            case error_code_t::request_failed_t: return "Request to TSAR server failed.";
            case error_code_t::app_not_found_t: return "App ID not found.";
            case error_code_t::app_paused_t: return "App was paused by the developer.";
            case error_code_t::user_not_found_t: return "HWID does not match to user.";
            case error_code_t::server_error_t: return "TSAR server did not return OK.";
            case error_code_t::bad_request_t: return "The request passed to the TSAR server was bad.";
            case error_code_t::rate_limited_t: return "User has been rate limited.";
            case error_code_t::failed_to_parse_body_t: return "Failed to parse body into JSON.";
            case error_code_t::failed_to_get_data_t: return "Failed to get data field from JSON.";
            case error_code_t::failed_to_get_signature_t: return "Failed to get signature field from JSON.";
            case error_code_t::failed_to_decode_data_t: return "Failed to decode data field from parsed JSON body.";
            case error_code_t::failed_to_decode_signature_t: return "Failed to decode signature field from parsed JSON body.";
            case error_code_t::failed_to_decode_public_key_t: return "Failed to decode client key from base64.";
            case error_code_t::failed_to_parse_data_t: return "Failed to parse data field into JSON.";
            case error_code_t::failed_to_get_timestamp_t: return "Failed to get timestamp field.";
            case error_code_t::failed_to_parse_timestamp_t: return "Failed to parse timestamp field into uint64_t.";
            case error_code_t::hwid_mismatch_t: return "Local HWID and HWID returned from server don't match.";
            case error_code_t::old_response_t: return "Response is old.";
            case error_code_t::invalid_signature_t: return "Signature is not authentic.";

            case error_code_t::unexpected_error_t:
            default: return "An unexpected error occurred.";
        }
    }
}  // namespace tsar