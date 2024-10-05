#include "user.hpp"

#include "base64.hpp"

namespace tsar
{
    static void from_json( const nlohmann::json& json, subscription_t& subscription )
    {
        json.at( "id" ).get_to( subscription.id );

        if ( json[ "expires" ].is_null() )
            subscription.expires = std::nullopt;
        else
            subscription.expires = std::chrono::system_clock::from_time_t( json.at( "expires" ).get< std::time_t >() );

        json.at( "tier" ).get_to( subscription.tier );
    }

    result_t< nlohmann::json > user::api_query( const std::string_view endpoint ) const noexcept
    {
        return client::api_call( session_key, std::format( "{}?session={}", endpoint, session ) );
    }

    user::user( const nlohmann::json& json )
    {
        try
        {
            json.at( "id" ).get_to( id );

            if ( json[ "name" ].is_null() )
                name = std::nullopt;
            else
                name = json.at( "name" ).get< std::string >();

            if ( json[ "avatar" ].is_null() )
                avatar = std::nullopt;
            else
                avatar = json.at( "avatar" ).get< std::string >();

            json.at( "subscription" ).get_to( subscription );

            json.at( "session" ).get_to( session );

            const auto key = base64::safe_from_base64( json.at( "session_key" ).get< std::string >() );

            if ( !key )
                throw error( error_code_t::failed_to_decode_session_key_t );

            session_key = *key;
        }
        catch ( const std::exception& )
        {
            throw error( error_code_t::failed_to_parse_body_t );
        }
    }

    result_t< void > user::heartbeat() const noexcept
    {
        const auto result = api_query( "heartbeat" );

        if ( !result )
            return std::unexpected( result.error() );

        return {};
    }
}  // namespace tsar
