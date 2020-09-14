# Works with 3.11 and tested through 3.16
cmake_minimum_required(VERSION 3.11...3.16)

project(WEBRTCGAME
    VERSION 0.1
    DESCRIPTION "Simple Webrtc LAN game"
    LANGUAGES CXX)


# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here
    
    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
    include(CTest)

    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
	add_subdirectory(docs)
    else()
	message(STATUS "Doxygen not found, not building docs")
    endif()
endif()


# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14; use our shim
if(${CMAKE_VERSION} VERSION_LESS 3.14)
    include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()


# Accumulator library
# This is header only, so could be replaced with git submodules or FetchContent

# Adds Boost::boost
find_package(Boost REQUIRED)

# Adds OpenSSL::Crypto
find_package(OpenSSL REQUIRED)

find_package(Threads REQUIRED)

find_package(jsoncpp REQUIRED)

#blehhh we need to make sure to set the dir as the  .cmake config doesn't lie in the default search path of cmake 
set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
find_package(cpprestsdk REQUIRED)

#find_library(CURL_LIBRARY NAMES webrtc HINTS  "${WEBRTCGAME_SOURCE_DIR}/local/lib")
set(MY_PUBLIC_HEADERS "${WEBRTCGAME_SOURCE_DIR}/local/include" "${WEBRTCGAME_SOURCE_DIR}/local/include/third_party/abseil-cpp") 

add_library(webrtc STATIC IMPORTED)
set_target_properties(webrtc PROPERTIES
    IMPORTED_LOCATION "${WEBRTCGAME_SOURCE_DIR}/local/lib/libwebrtc.a"
    PUBLIC_HEADER "${MY_PUBLIC_HEADERS}"
 )

#set_target_properties(webrtc PROPERTIES PUBLIC_HEADER "${MY_PUBLIC_HEADERS}" )

include_directories( "${WEBRTCGAME_SOURCE_DIR}/local/include")
include_directories("${WEBRTCGAME_SOURCE_DIR}/local/include/third_party/abseil-cpp")
# The compiled library code is here


add_subdirectory(src)

# The executable code is here
add_subdirectory(apps)

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
    add_subdirectory(tests)
endif()

