add_executable(app app.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_LINUX=1 -DWEBRTC_POSIX=1")

target_link_libraries(app PRIVATE network )
