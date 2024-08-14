#include "user.hpp"

#include "error.hpp"

namespace tsar
{
    static void from_json( const nlohmann::json& json, subscription_t& subscription )
    {
        json.at( "id" ).get_to( subscription.id );

        subscription.expires = json[ "expires" ].is_null()
                                   ? std::nullopt
                                   : std::optional{ std::chrono::system_clock::from_time_t( json.at( "expires" ).get< std::time_t >() ) };

        json.at( "tier" ).get_to( subscription.tier );
    }

    user::user( const nlohmann::json& json )
    {
        try
        {
            json.at( "id" ).get_to( id );

            name = json[ "name" ].is_null() ? std::nullopt : std::optional{ json.at( "name" ).get< std::string >() };
            avatar = json[ "avatar" ].is_null() ? std::nullopt : std::optional{ json.at( "avatar" ).get< std::string >() };

            json.at( "subscription" ).get_to( subscription );

            json.at( "session" ).get_to( session );
            json.at( "session_key" ).get_to( session_key );
        }
        catch ( const std::exception& )
        {
            throw error( error_code_t::failed_to_parse_body_t );
        }
    }
}  // namespace tsar