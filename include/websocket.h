#include <cpprest/ws_client.h>

#define SUCCESS true
#define FAILURE false

using namespace web;
using namespace web::websockets::client;



class Websocket{
    public:
    Websocket();
    bool connect(std::string url );

    bool  send(std::string data);


    void setreceivehandler(std::function<void(websocket_incoming_message msg)> handler);

    bool close();

    private:

    bool iswebsocketconnected;

    websocket_callback_client client;
};
