add_executable(app app.cpp)

set(CMAKE_CXX_STANDARD 14)

#target_compile_features(app PRIVATE cxx_std_14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_LINUX=1 -DWEBRTC_POSIX=1")


target_include_directories(app PUBLIC ../include PRIVATE ${WEBRTCGAME_SOURCE_DIR}/local/include  ${WEBRTCGAME_SOURCE_DIR}/local/include/third_party/abseil-cpp )
#include_directories("${WEBRTCGAME_SOURCE_DIR}/local/include")
#include_directories("${WEBRTCGAME_SOURCE_DIR}/local/include/third_party/abseil-cpp")
#link_directories("${WEBRTCGAME_SOURCE_DIR}/local/lib")
target_link_libraries(app PRIVATE network )
