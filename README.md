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

// You should have gotten these values after creating your app
// You can find them in your app's configuration settings
constexpr auto app_id = "00000000-0000-0000-0000-000000000000";
constexpr auto client_key = "MFk...";

int main()
{
  // This will create a new client & perform a hash check on your binary
  const auto client = tsar::client::create(app_id, client_key);

  if (!client)
  {
      error( "[AUTH] Failed to create client: ", client.error() );
      return 1;
  }

  std::println(std::cout, "[AUTH] Attempting to authenticate client...");

  // Check if user is authorized. By default the user's browser opens when auth fails. Passing `false` as an initial argument disables that.
  auto user = client->authenticate();

  // If they aren't authorized, continue to check until they've authenticated themselves in their browser.
  while (!user)
  {
      // Only continue the loop if the error type is "Unauthorized".
      if (user.error() != tsar::error_code_t::unauthorized_t)
      {
          error("[AUTH] Failed to authenticate: ", user.error());
          return 1;
      }

      std::this_thread::sleep_for(std::chrono::seconds(3)); // Keep a delay of at least 3 seconds to prevent rate-limiting.

      // Make sure to use false for any authenticate() function that's inside a loop, or else the browser will keep opening nonstop.
      user = client->authenticate(false);
  }

  // At this point the user is authenticated
  std::println(std::cout, "[AUTH] Successfully authenticated.");
  std::println(std::cout, "[AUTH] Welcome, {}.", user->username.value_or(user->id));

  // Start a heartbeat loop to continue checking if the user is authorized (we recommend running this in a background thread)
  //
  // **MAKE SURE THE LOOP RUNS ONLY ONCE EVERY 10 - 30 SECONDS**
  // Otherwise, your users might get rate-limited.
  //
  // Using a heartbeat thread will allow you to delete user sessions and have them be kicked off of your software live.
  // Additionally, if their subscription expires they will also be kicked during the heartbeat check.
  tsar::result_t< void > status;

  while (status = user->heartbeat())
  {
      std::println(std::cout, "[AUTH] Heartbeat success.");
      std::this_thread::sleep_for( std::chrono::seconds(20));
  }

  error("[AUTH] Heartbeat failed: ", status.error());
  return 1;
}
```

## Contributing

This project definitely has room for improvement, so we are open to any contribution! Feel free to send a pull request at any time and we will review it ASAP. If you want to contribute but don't know what, take a quick look at our [issues](https://github.com/tsarnet/cpp-sdk-v2/issues) and feel free to take on any of them.

### Need help?

Join our [discord community](https://tsar.cc/discord) if you have any questions. For other contact options, please [visit here](https://tsar.cc/about/social).
