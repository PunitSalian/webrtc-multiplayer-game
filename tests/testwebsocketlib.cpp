#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <websocket.h>


TEST_CASE( "Client Connect pass", "[main]" ) {
    Websocket client;

    client.connect(U("localhost:9090"));

    std::string result;
    auto handler = [&result](websocket_incoming_message msg){
         result = msg.extract_string().get();
    };
    client.setreceivehandler(handler);
    client.send("Hello world");
    sleep(1);

    client.close();

    REQUIRE( result  == "Hello world" );

}

TEST_CASE( "Client Connect fail", "[main]" ) {
    Websocket client;

    bool ret = true;
    ret = client.connect(U("localhost:9091:"));


    REQUIRE(ret == false  );

}

