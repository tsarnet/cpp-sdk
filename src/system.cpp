#include "system.hpp"

namespace tsar::system
{
    std::optional< std::string > hwid() noexcept
    {
        char szBuffer[ BUFSIZ ]{};
        DWORD dwSize = sizeof( szBuffer );

        if ( RegGetValue( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid", RRF_RT_REG_SZ, NULL, szBuffer, &dwSize ) )
            return std::nullopt;

        return std::string( szBuffer, dwSize );
    }

    bool open_browser( const std::string_view url ) noexcept
    {
        return reinterpret_cast< std::uintptr_t >( ShellExecute( NULL, "open", url.data(), NULL, NULL, SW_SHOWNORMAL ) ) > 32;
    }
}  // namespace tsar::system
