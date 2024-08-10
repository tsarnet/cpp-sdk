# TSAR C++ SDK
![banner](/banner.png)

> The lastest build of the C++ SDK will only compile if building a Windows application. We are currently working on supporting other systems like Unix/Linux and MacOS.

* [Installation](#installation)
* [Contributing](#contributing)
* [Need help?](#need-help)

## Installation
This is a [CMake](https://cmake.org/) project so it is recommended to use CMake to install the SDK into your wider project. Although, you are free to download the prebuilt static libraries from any of the latest releases.

If you are using CMake, simply add the following lines to your `CMakeLists.txt` file:
```cmake
include (FetchContent) # if you don't have this already

# Fetch the latest version of the SDK
FetchContent_Declare (tsar URL https://github.com/tsarnet/cpp-sdk/releases/download/v1.0/tsar.tar.xz)
FetchContent_MakeAvailable (tsar)

# Link the SDK into your project
target_link_libraries(your_project PRIVATE tsar_sdk::tsar_sdk)
```
This library requires [OpenSSL](https://www.openssl.org/), a package that should already be installed on your system. If you get a build error regarding OpenSSL, you can install it via a package manager like [vcpkg](https://vcpkg.io/) or [conan](https://conan.io/).

## Contributing
This project definitely has room for improvement, so we are open to any contribution! Feel free to send a pull request at any time and we will review it ASAP. If you want to contribute but don't know what, take a quick look at our [issues](https://github.com/tsarnet/cpp-sdk-v2/issues) and feel free to take on any of them.

### Need help?
Join our [discord community](https://discord.com/invite/JReXjQCVPw) if you have any questions. For other contact options, please [visit here](https://tsar.cc/about/social).
