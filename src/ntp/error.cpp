#include "ntp/error.hpp"

namespace tsar::ntp
{
    /// <summary>
    /// The error category for the NTP client.
    /// </summary>
    const error_category& error_category::get() noexcept
    {
        static error_category instance;
        return instance;
    }

    /// <summary>
    /// Gets the name of the error category.
    /// </summary>
    const char* error_category::name() const noexcept
    {
        return "tsar::ntp";
    }

    /// <summary>
    /// Gets the message for the specified error code.
    /// </summary>
    std::string error_category::message( int ev ) const
    {
        switch ( static_cast< error_code_t >( ev ) )
        {
            case error_code_t::failed_to_build_connection_t: return "Failed to build a connection to the server.";
            case error_code_t::failed_to_resolve_hostname_t: return "Failed to resolve the hostname.";
            case error_code_t::failed_to_send_packet_t: return "Failed to send a packet to the server.";
            case error_code_t::failed_to_receive_packet_t: return "Failed to receive a packet from the server.";
            default: return "An unknown error occurred.";
        }
    }
}  // namespace tsar::ntp