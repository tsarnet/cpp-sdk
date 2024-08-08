#define CPPHTTPLIB_OPENSSL_SUPPORT

#pragma once

#include <httplib.h>

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "error.hpp"

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

        /// <summary>
        /// Creates a new TSAR client with the specified app ID, client key, and hardware ID.
        /// </summary>
        explicit client( const std::string_view app_id, const std::string_view client_key, const std::string_view hwid );

        /// <summary>
        /// Queries the TSAR API with the specified endpoint.
        /// </summary>
        nlohmann::json query( const std::string_view endpoint );

        template< class... T >
        nlohmann::json query( const std::format_string< T... > format, T&&... args );

       public:
        /// <summary>
        /// Creates a new TSAR client with the specified app ID, client key, and hostname.
        /// </summary>
        /// <param name="app_id">The ID of your TSAR app. Should be in UUID format: 00000000-0000-0000-0000-000000000000</param>
        /// <param name="client_key">The public decryption key for your TSAR app. Should be in base64 format.</param>
        /// <param name="hostname">Replace `APP_ID.tsar.app` with a custom hostname. You can use your vanity (vanity.tsar.app) or a custom domain.
        /// Make sure that your custom domain is registered in your app's settings through the management dashboard. Only assign hostnames to this
        /// variable. Do not include `https://` or route info. Examples: `website.com`, `dash.website.com`, `my.custom.site`</param>
        static result_t< std::unique_ptr< client > >
        create( const std::string_view app_id, const std::string_view client_key, const std::string_view hostname ) noexcept;

        /// <summary>
        /// Creates a new TSAR client with the specified app ID and client key.
        /// </summary>
        /// <param name="app_id">The ID of your TSAR app. Should be in UUID format: 00000000-0000-0000-0000-000000000000</param>
        /// <param name="client_key">The public decryption key for your TSAR app. Should be in base64 format.</param>
        static result_t< std::unique_ptr< client > > create( const std::string_view app_id, const std::string_view client_key ) noexcept;

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

    template< class... T >
    inline nlohmann::json client::query( const std::format_string< T... > format, T&&... args )
    {
        return query( std::vformat( format.get(), std::make_format_args( args... ) ) );
    }
}  // namespace tsar