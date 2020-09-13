#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>

#include <vector>
#include <string>

#include <api/create_peerconnection_factory.h>
class GameNetwork{

    public:

    void NetworkInit(std::string&);
    
    bool Connect(std::vector<std::string>);
    
    void Senddata(std::string& data);

    void Disconnect(std::vector<std::string>);

    private:
    
    int peercount;
};

#endif
