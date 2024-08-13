# TSAR C++ SDK
![banner](/banner.png)

> The lastest build of the C++ SDK will only compile if building a Windows application. We are currently working on supporting other systems like Unix/Linux and MacOS.

* [Installation](#installation)
    * [Static Libraries](#static-libraries)
* [Usage](#usage)
* [Contributing](#contributing)
* [Need help?](#need-help)

## Installation
This is a [CMake](https://cmake.org/) project so it is recommended to use CMake to install the SDK into your wider project. Although, you are free to download the prebuilt static libraries from any of the latest releases.

If you are using CMake, simply add the following lines to your `CMakeLists.txt` file:
```cmake
include (FetchContent) # if you don't have this already

# Fetch the latest version of the SDK
FetchContent_Declare (tsar URL https://github.com/tsarnet/cpp-sdk/releases/latest/download/tsar-src.zip)
FetchContent_MakeAvailable (tsar)

# Link the SDK into your project
target_link_libraries(your_project PRIVATE tsar)
```
This library requires [OpenSSL](https://www.openssl.org/) and [cURL](https://curl.se/), packages that should already be installed on your system. If you get a build error you can install them via [vcpkg](https://vcpkg.io/) like so:
```
vcpkg install openssl curl
```
Then run the following command in the project's root directory to set up the CMake project. Remember to replace `<path-to-vcpkg>` with the path to vcpkg. This depends on where you have installed it.
```
cmake . -D CMAKE_TOOLCHAIN_FILE=C:\<path-to-vcpkg>\scripts\buildsystems\vcpkg.cmake
```
### Static Libraries
If you are not a CMake user, you will have to manually link the SDK to your project.

## Usage
We've designed this library to be lightweight and easy to use. Feel free to take a peek at any of our [examples](/examples), all of them are pretty straightforward. A simple usage example has been attached below:
```cpp
#include "tsar.hpp"

// Get these credentials from: https://tsar.cc/app/*/settings
constexpr auto app_id = "f911842b-5b3d-4c59-b5d1-4adb8f71557b";
constexpr auto client_key =
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEvJrwPvdeDUcV8Qr02tzgFrp+8qfCV/vG1HcQJYYV8u5vYUfGABMAYT0qOQltXEX9DTcB2fzLfwQnl7yiAaNruQ==";

int main()
{
    try
    {
        // Create the client with the provided credentials. 
        const auto client = tsar::client::create( app_id, client_key );

        // Retrieve the subscription associated with the current user.
        const auto& subscription = client->get_subscription();

        std::println( std::cout, "[+] Authentication success, welcome {}!", subscription.user.username.value_or( "N/A" ) );

        // Now we check the client's heartbeat. If this method ever returns false, then the current user's session has expired.
        // This method sends a request to the server every time its called.
        while ( client->validate() )
        {
            std::println( std::cout, "[+] Heartbeat success" );

            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        }

        std::println( std::cout, "[-] Heartbeat failed. Session has expired." );
    }
    catch ( const tsar::error& e )
    {
        std::cerr << "[-] Error [" << e.code() << "]: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
``` 

## Contributing
This project definitely has room for improvement, so we are open to any contribution! Feel free to send a pull request at any time and we will review it ASAP. If you want to contribute but don't know what, take a quick look at our [issues](https://github.com/tsarnet/cpp-sdk-v2/issues) and feel free to take on any of them.

### Need help?
Join our [discord community](https://discord.com/invite/5xKTNFdQ) if you have any questions. For other contact options, please [visit here](https://tsar.cc/about/social).
