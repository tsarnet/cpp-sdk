#pragma once

#include <optional>
#include <string_view>

#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__
#include <windows.h>
#else
// Note: This is a temporary solution while the API is in development. Eventually we will support other platforms.
#error "This code can only be compiled on Windows."
#endif

/// <summary>
/// System utilities for the TSAR API.
/// </summary>
namespace tsar::system
{
    /// <summary>
    /// Gets the hardware ID of the current system.
    /// </summary>
    extern std::optional< std::string > hwid() noexcept;

    /// <summary>
    /// Opens a browser window with the specified URL.
    /// </summary>
    /// <param name="url">The URL to visit.</param>
    /// <returns>True if successful.</returns>
    extern bool open_browser( const std::string_view url ) noexcept;

}  // namespace tsar::system