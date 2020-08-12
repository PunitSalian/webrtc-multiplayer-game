#include <cpprest/ws_client.h>
#include <iostream>
#include <websocket.h>
using namespace web;
using namespace web::websockets::client;

    
Websocket::Websocket(){

}


bool Websocket::connect(std::string url ){

    
    std::cout <<  "ws://" + url << std::endl; 

    try{
        client.connect(U("ws://"+url)).wait();
    }catch(const websocket_exception& ex) {
        std::cout << ex.what()<< std::endl;
        return false;
    }
    std::cout << "Connected to the signalling server"<<std::endl;
    return true;
}

void Websocket::send(std::string data){
    websocket_outgoing_message msg;
    msg.set_utf8_message(data);
    client.send(msg).then([](pplx::task<void> t){
        try{
                //t.get();
        }
        catch(const websocket_exception& ex) {
            std::cout << ex.what();
        }
    });
        
}
    
void Websocket::setreceivehandler(std::function<void(websocket_incoming_message msg)> handler){
    client.set_message_handler(handler);
}

void Websocket::close(){
    client.close().wait();
}



