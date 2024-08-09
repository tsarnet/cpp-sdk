cmake_minimum_required (VERSION 3.8)

# Set our C++ standard
set (CMAKE_CXX_STANDARD 23)

set(OPENSSL_ROOT_DIR "C:\\Program Files\\vcpkg\\installed\\x64-windows")

# Set the project name
project (tsar LANGUAGES CXX)

# Set CMake output directories 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define the library targets
add_subdirectory (src)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # We're in the root, define additional targets for developers.
    option (BUILD_PACKAGE    "whether or not to build a package" ON)
    option (BUILD_EXAMPLES   "whether or not examples should be built" ON)

    if (BUILD_PACKAGE)
        set (package_files include/ src/ CMakeLists.txt LICENSE)
        add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-src.zip
            COMMAND ${CMAKE_COMMAND} -E tar c ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-src.zip --format=zip -- ${package_files}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${package_files})
        add_custom_target (${PROJECT_NAME}_package DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-src.zip)
    endif ()

    if (BUILD_EXAMPLES)
		add_subdirectory (examples)
    endif ()
endif ()