#define CPPHTTPLIB_OPENSSL_SUPPORT

#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "ntp/client.hpp"

namespace tsar
{
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
        static result_t< std::unique_ptr< client > > create( const std::string_view app_id, const std::string_view client_key ) noexcept;
    };

}  // namespace tsar