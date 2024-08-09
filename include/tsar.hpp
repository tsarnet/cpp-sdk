#define CPPHTTPLIB_OPENSSL_SUPPORT

#pragma once

#include <httplib.h>

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "error.hpp"
#include "ntp_client.hpp"

namespace tsar
{
    struct validation_data_t;

    /// <summary>
    /// The result type for the TSAR API.
    /// </summary>
    template< typename T >
    using result_t = std::expected< T, error >;

    /// <summary>
    /// The TSAR client class. This class interacts with the API after it has been initialized.
    /// </summary>
    class client
    {
        std::string app_id, client_key, hwid;
        httplib::Client http_client;
        NTPClient ntp_client;

        /// <summary>
        /// Creates a new TSAR client with the specified app ID, client key, and hardware ID.
        /// </summary>
        explicit client( const std::string_view app_id, const std::string_view client_key, const std::string_view hwid );

        /// <summary>
        /// Queries the TSAR API with the specified endpoint.
        /// </summary>
        result_t< nlohmann::json > query( const std::string_view endpoint ) noexcept;

        /// <summary>
        /// Verifies the signature of the JSON data using the ECDSA algorithm.
        /// </summary>
        bool verify_signature( const std::string_view json, const std::string_view signature ) const noexcept;

       public:
        /// <summary>
        /// Creates a new TSAR client with the specified app ID, client key, and hostname.
        /// </summary>
        /// <param name="app_id">The ID of your TSAR app. Should be in UUID format: 00000000-0000-0000-0000-000000000000</param>
        /// <param name="client_key">The public decryption key for your TSAR app. Should be in base64 format.</param>
        /// <param name="hostname">Replace `APP_ID.tsar.app` with a custom hostname. You can use your vanity (vanity.tsar.app) or a custom domain.
        /// Make sure that your custom domain is registered in your app's settings through the management dashboard. Only assign hostnames to this
        /// variable. Do not include `https://` or route info. Examples: `website.com`, `dash.website.com`, `my.custom.site`</param>
        static std::unique_ptr< client > create( const std::string_view app_id, const std::string_view client_key, const std::string_view hostname );

        /// <summary>
        /// Creates a new TSAR client with the specified app ID and client key.
        /// </summary>
        /// <param name="app_id">The ID of your TSAR app. Should be in UUID format: 00000000-0000-0000-0000-000000000000</param>
        /// <param name="client_key">The public decryption key for your TSAR app. Should be in base64 format.</param>
        static std::unique_ptr< client > create( const std::string_view app_id, const std::string_view client_key );

        /// <summary>
        /// Validates the current client session.
        /// </summary>
        /// <returns></returns>
        result_t< validation_data_t > validate();
    };

    /// <summary>
    /// Data returned by the server when validating a client session.
    /// </summary>
    struct validation_data_t
    {
        /// <summary>
        /// The hardware ID of the client.
        /// </summary>
        std::string hwid;

        /// <summary>
        /// The timestamp of the validation.
        /// </summary>
        std::uint64_t timestamp;
    };
}  // namespace tsar