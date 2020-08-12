
# Testing library
FetchContent_Declare(
catch2
GIT_REPOSITORY https://github.com/catchorg/Catch2.git
GIT_TAG        v2.9.1
)
FetchContent_MakeAvailable(catch2)
# Adds Catch2::Catch2


add_executable(testwebsocketlib testwebsocketlib.cpp)
# I'm using C++17 in the test
target_compile_features(testwebsocketlib PRIVATE cxx_std_17)
# Should be linked to the main library, as well as the Catch2 testing library

target_link_libraries(testwebsocketlib PRIVATE websocket  Catch2::Catch2)

# If you register a test, then ctest and make test will run it.
add_test(NAME testwebsocketlib COMMAND testwebsocketlib --success)
