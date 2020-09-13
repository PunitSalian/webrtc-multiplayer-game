#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <websocket.h>

TEST_CASE("Client Connect pass", "[main]") {
  Websocket client;

  auto r1 = client.connect(U("localhost:9090"));

  std::string result;
  auto handler = [&result](websocket_incoming_message msg) {
    result = msg.extract_string().get();
  };

  client.setreceivehandler(handler);

  auto r2 = client.send("Hello world");

  sleep(1);

  REQUIRE(r1 == true);

  REQUIRE(r2 == true);

  REQUIRE(result == "Hello world");
}

TEST_CASE("Client Connect fail", "[main]") {
  Websocket client;

  bool ret = true;
  ret = client.connect(U("localhost:9091:"));

  REQUIRE(ret == false);
}

TEST_CASE("Client close fail", "[main]") {
  Websocket client;

  bool ret = true;

  ret = client.close();

  REQUIRE(ret == false);
}
