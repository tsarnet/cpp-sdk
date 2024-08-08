include(FetchContent)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Fetch and include the http library
FetchContent_Declare (httplib GIT_REPOSITORY https://github.com/yhirose/cpp-httplib)
FetchContent_MakeAvailable (httplib)

# Fetch and include the json library
FetchContent_Declare (json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable (json)

cmake_path (SET include_dir "${CMAKE_CURRENT_SOURCE_DIR}/../include")

# Add the header files to the project
set (header_files 
	"${include_dir}/tsar.hpp"
	"${include_dir}/error.hpp"
	"${include_dir}/system.hpp")

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
target_link_libraries (tsar PRIVATE httplib::httplib)
target_link_libraries (tsar PRIVATE nlohmann_json::nlohmann_json)

# Add the source files to the project
target_sources (tsar PRIVATE
	"tsar.cpp"
	"error.cpp"
	"system.cpp"
)

# Add the include directories
target_include_directories (tsar PRIVATE ${include_dir})