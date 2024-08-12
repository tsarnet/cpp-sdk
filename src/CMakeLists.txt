include(FetchContent)

# Ensure that static libraries are used
set (OPENSSL_USE_STATIC_LIBS TRUE)
set (CURL_USE_STATIC_LIBS TRUE)

# Find the required packages
find_package (OpenSSL REQUIRED)
find_package (CURL REQUIRED)

# Fetch and include the json library
FetchContent_Declare (json 
	URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz 
	DOWNLOAD_EXTRACT_TIMESTAMP true)
FetchContent_MakeAvailable (json)

cmake_path (SET include_dir "${CMAKE_CURRENT_SOURCE_DIR}/../include")

# Add the header files to the project
set (header_files 
	"${include_dir}/base64.hpp"
	"${include_dir}/tsar.hpp"
	"${include_dir}/error.hpp"
	"${include_dir}/system.hpp"
	"${include_dir}/ntp_client.hpp")

# Add the library to the project
add_library (tsar_core INTERFACE)

# Add the include directory to the project
target_sources (tsar_core INTERFACE ${header_files})

# Add the include directories
target_include_directories(tsar_core SYSTEM INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library (tsar STATIC)
target_link_libraries (tsar PUBLIC tsar_core)
target_link_libraries (tsar PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries (tsar PRIVATE CURL::libcurl)
target_link_libraries (tsar PUBLIC nlohmann_json::nlohmann_json)


# Add the source files to the project
target_sources (tsar PRIVATE
	"tsar.cpp"
	"error.cpp"
	"system.cpp"
	"ntp_client.cpp"
)

# Add the include directories
target_include_directories (tsar PRIVATE ${include_dir} ${OPENSSL_INCLUDE_DIR})