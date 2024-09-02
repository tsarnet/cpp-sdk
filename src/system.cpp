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

    std::string get_hash() noexcept
    {
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);

        std::string current_exe(path);


        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        
        std::ifstream file(current_exe, std::ifstream::binary);

        const size_t bufferSize = 32768;
        std::vector<char> buffer(bufferSize);
        while (file.good()) {
            file.read(buffer.data(), buffer.size());
            SHA256_Update(&sha256, buffer.data(), file.gcount());
        }
        
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
}  // namespace tsar::system
