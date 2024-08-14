#pragma once

#include <chrono>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace tsar
{
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
        /// The tier of the user's subscription, set by the app's purchase flow. Default is 0.
        /// </summary>
        std::uint32_t tier;
    };

    /// <summary>
    /// Represents a user in the TSAR system.
    /// </summary>
    class user
    {
        std::string session, session_key;

       public:
        std::string id;
        std::optional< std::string > name, avatar;

        subscription_t subscription;

        /// <summary>
        /// Creates a new user from the specified JSON data. Throws if the JSON data is invalid.
        /// </summary>
        /// <param name="json">The json data.</param>
        explicit user( const nlohmann::json& json );

        /// <summary>
        /// The default constructor.
        /// </summary>
        explicit user() = default;
    };
}  // namespace tsar