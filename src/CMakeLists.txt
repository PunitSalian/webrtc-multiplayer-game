set(HEADER_LIST  "${WEBRTCGAME_SOURCE_DIR}/include/websocket.h" )

set(HEADER_LIST2  "${WEBRTCGAME_SOURCE_DIR}/include/network.h" )


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_LINUX=1 -DWEBRTC_POSIX=1")


# Make an automatic library - will be static or dynamic based on user setting
add_library(websocket websocket.cpp ${HEADER_LIST})

add_library(network network.cpp ${HEADER_LIST2})

# We need this directory, and users of our library will need it too
target_include_directories(websocket PUBLIC ../include)


target_include_directories(network PUBLIC ../include PRIVATE ${WEBRTCGAME_SOURCE_DIR}/local/include  ${WEBRTCGAME_SOURCE_DIR}/local/include/third_party/abseil-cpp )

# This depends on (header only) boost
target_link_libraries(websocket PRIVATE Boost::boost  OpenSSL::SSL  cpprestsdk::cpprest)

target_compile_features(websocket PUBLIC cxx_std_11)

target_link_libraries(network PUBLIC webrtc websocket Threads::Threads jsoncpp)

target_compile_features(network PUBLIC cxx_std_11)

# All users of this library will need at least C++11

# IDEs should put the headers in a nice place
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})
