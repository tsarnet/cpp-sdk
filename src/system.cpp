#include "system.hpp"

namespace tsar::system
{
#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__
    // Windows implementations
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

#elif defined __APPLE__
    // macOS implementations
    std::optional< std::string > hwid() noexcept
    {
        std::optional< std::string > result;

        // Get the IO registry entry for the platform expert
        io_service_t platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault,
                                                                   IOServiceMatching("IOPlatformExpertDevice"));

        if (platformExpert) {
            // Get the serial number
            CFTypeRef serialNumberAsCFString =
                IORegistryEntryCreateCFProperty(platformExpert,
                                               CFSTR(kIOPlatformSerialNumberKey),
                                               kCFAllocatorDefault, 0);

            if (serialNumberAsCFString) {
                if (CFGetTypeID(serialNumberAsCFString) == CFStringGetTypeID()) {
                    CFStringRef serialNumber = (CFStringRef)serialNumberAsCFString;
                    CFIndex length = CFStringGetLength(serialNumber);
                    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;

                    std::vector<char> buffer(maxSize);
                    if (CFStringGetCString(serialNumber, buffer.data(), maxSize, kCFStringEncodingUTF8)) {
                        result = std::string(buffer.data());
                    }
                }
                CFRelease(serialNumberAsCFString);
            }

            IOObjectRelease(platformExpert);
        }

        return result;
    }

    bool open_browser( const std::string_view url ) noexcept
    {
        std::string command = "open \"" + std::string(url) + "\"";
        return system(command.c_str()) == 0;
    }

    std::string get_hash() noexcept
    {
        char path[PATH_MAX];
        uint32_t size = sizeof(path);

        // Get the executable path
        if (_NSGetExecutablePath(path, &size) != 0) {
            return "";
        }

        // Resolve any symlinks to get the real path
        char realPath[PATH_MAX];
        if (realpath(path, realPath) == nullptr) {
            return "";
        }

        std::string current_exe(realPath);

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        std::ifstream file(current_exe, std::ifstream::binary);
        if (!file.good()) {
            return "";
        }

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
#endif

}  // namespace tsar::system
