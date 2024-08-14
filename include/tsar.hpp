#pragma once

#include <memory>
#include <string>

#include "ntp/client.hpp"
#include "user.hpp"

namespace tsar
{
    struct validate_result_t
	{
		st
	};

    /// <summary>
    /// The TSAR client class. This class interacts with the API after it has been initialized.
    /// </summary>
    class client final
    {
        /// <summary>
        /// The URL of the TSAR API.
        /// </summary>
        constexpr static auto api_url = "https://tsar.dev/api/client";

        /// <summary>
        /// The NTP client used to get the current time.
        /// </summary>
        static ntp::client ntp;

        std::string app_id, pub_key, hostname;

        /// <summary>
        /// Creates a new TSAR client with the specified app ID and public key.
        /// </summary>
        explicit client( const std::string_view app_id, const std::string_view pub_key, const std::string_view hostname );

        /// <summary>
        /// Queries the TSAR API with the specified endpoint.
        /// </summary>
        static result_t< nlohmann::json > query( const std::string_view key, const std::string_view endpoint ) noexcept;

        template< typename T >
        static result_t< T > query( const std::string_view key, const std::string_view endpoint ) noexcept;

        /// <summary>
        /// Verifies the signature of the JSON data using the ECDSA algorithm.
        /// </summary>
        static bool verify_signature( const std::string_view key, const std::string_view json, const std::string_view signature ) noexcept;

       public:
        /// <summary>
        /// Creates a new TSAR client with the specified app ID and client key.
        /// </summary>
        /// <param name="app_id">The ID of your TSAR app. Should be in UUID format: 00000000-0000-0000-0000-000000000000</param>
        /// <param name="client_key">The public decryption key for your TSAR app. Should be in base64 format.</param>
        static result_t< client > create( const std::string_view app_id, const std::string_view client_key ) noexcept;

        /// <summary>
        /// Attemps to authenticate the client with the TSAR API. If the user's HWID is not authorized, the function opens the user's default browser
        /// to prompt a login.
        /// </summary>
        /// <param name="open">Whether to open the user's default browser to prompt a login.</param>
        /// <returns>The user.</returns>
        result_t< user > authenticate( bool open = true ) const noexcept;
    };

    template< typename T >
    inline result_t< T > client::query( const std::string_view key, const std::string_view endpoint ) noexcept
    {
        const auto result = query( key, endpoint );

        if ( result )
        {
            try
            {
                return T{ *result };
            }
            catch ( const error& e )
            {
                return std::unexpected( e );
            }
        }

        return std::unexpected( result.error() );
    }

}  // namespace tsar