
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")
set(HEADER_LIST  "${WEBRTCGAME_SOURCE_DIR}/include/websocket.h" )

# Make an automatic library - will be static or dynamic based on user setting
add_library(websocket websocket.cpp ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(websocket PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(websocket PRIVATE Boost::boost  OpenSSL::SSL  cpprestsdk::cpprest)

# All users of this library will need at least C++11
target_compile_features(websocket PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})
