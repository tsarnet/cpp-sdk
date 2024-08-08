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

            case error_code_t::unexpected_error_t:
            default: return "An unexpected error occurred.";
        }
    }
}  // namespace tsar