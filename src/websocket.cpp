#include <cpprest/ws_client.h>
#include <iostream>
#include <websocket.h>
using namespace web;
using namespace web::websockets::client;

    
Websocket::Websocket(){
    iswebsocketconnected = false;
}


bool Websocket::connect(std::string url ){

    
    std::cout <<  "ws://" + url << std::endl; 

    try{
        client.connect(U("ws://"+url)).wait();
    }catch(const websocket_exception& ex) {
        iswebsocketconnected = false;
        std::cout << ex.what()<< std::endl;
        return false;
    }
    iswebsocketconnected = true;

    std::cout << "Connected to the signalling server"<<std::endl;
    return true;
}

bool Websocket::send(std::string data){
    websocket_outgoing_message msg;

    if(!iswebsocketconnected){
        return false;
    }
    msg.set_utf8_message(data);
    client.send(msg).then([](pplx::task<void> t){
        try{
                //t.get();
        }
        catch(const websocket_exception& ex) {
            std::cout << ex.what();
        }
    });
        
    return true;
}
    
void Websocket::setreceivehandler(std::function<void(websocket_incoming_message msg)> handler){
    client.set_message_handler(handler);
}

bool Websocket::close(){
     if(!iswebsocketconnected){
        return false;
     }
     try{
        client.close().wait();
     }catch(const websocket_exception& ex){
        std::cout << ex.what()<< std::endl;
        return false;
     }
    
     iswebsocketconnected = false;

     return true;
}



