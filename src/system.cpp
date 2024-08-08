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
}  // namespace tsar::system
