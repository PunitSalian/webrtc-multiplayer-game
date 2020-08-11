#include <cpprest/ws_client.h>
using namespace web;
using namespace web::websockets::client;

class Websocket{
    public:
    Websocket();
    bool connect(std::string url );

    void send(std::string data);


    void setreceivehandler(std::function<void(websocket_incoming_message msg)> handler);

    void close();

    private:

    websocket_callback_client client;
};
