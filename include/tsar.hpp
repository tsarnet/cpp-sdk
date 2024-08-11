#define CPPHTTPLIB_OPENSSL_SUPPORT

#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "error.hpp"
#include "ntp_client.hpp"

namespace tsar
{

    /// <summary>
    /// The result type for the TSAR API.
    /// </summary>
    template< typename T >
    using result_t = std::expected< T, error >;

    /// <summary>
    /// Represents a user in the TSAR API.
    /// </summary>
    struct user_t
    {
        /// <summary>
        /// The user identifier.
        /// </summary>
        std::string id;

        /// <summary>
        /// The user's username.
        /// </summary>
        std::optional< std::string > username;

        /// <summary>
        /// The user's avatar URL.
        /// </summary>
        std::optional< std::string > avatar;
    };

    /// <summary>
    /// Represents a subscription in the TSAR API.
    /// </summary>
    struct subscription_t
    {
        /// <summary>
        /// The subscription identifier.
        /// </summary>
        std::string id;

        /// <summary>
        /// Timestamp of when the subscription expires
        /// </summary>
        std::optional< std::chrono::system_clock::time_point > expires;

        /// <summary>
        /// The user associated with the subscription.
        /// </summary>
        user_t user;

        /// <summary>
        /// The tier of the user's subscription, set by the app's purchase flow. Default is 0.
        /// </summary>
        std::uint32_t tier;
    };

    /// <summary>
    /// The validation data for the TSAR API.
    /// </summary>
    struct validation_data_t
    {
        /// <summary>
        /// The hardware ID of the current user.
        /// </summary>
        std::string hwid;

        /// <summary>
        /// The current timestamp.
        /// </summary>
        std::chrono::system_clock::time_point timestamp;
    };

    /// <summary>
    /// The TSAR client class. This class interacts with the API after it has been initialized.
    /// </summary>
    class client
    {
        std::string app_id, pub_key, hwid;
        NTPClient ntp_client;

        /// <summary>
        /// The session identifier for the current client.
        /// </summary>
        std::string session;

        /// <summary>
        /// The subscription for the current client.
        /// </summary>
        subscription_t subscription;

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
        /// Validates the current session. Returns true if the session is valid.
        /// 
        /// Note: This function does not throw exceptions. It will return an error if the session is invalid.
        /// </summary>
        result_t< validation_data_t > validate() noexcept;

        /// <summary>
        /// Gets the current subscription for the client.
        /// </summary>
        constexpr subscription_t& get_subscription() noexcept
        {
            return subscription;
        }
    };

    void from_json( const nlohmann::json& j, user_t& u );
    void from_json( const nlohmann::json& j, subscription_t& s );
    void from_json( const nlohmann::json& j, validation_data_t& v );

}  // namespace tsar