#ifndef NETWORK_H
#define NETWORK_H

#include <api/create_peerconnection_factory.h>
#include <api/data_channel_interface.h>
#include <api/peer_connection_interface.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <websocket.h>


namespace GameNetwork {

bool NetworkInit(std::string& );

bool Connect(std::vector<std::string> &);

void NewConnection(std::string &name, std::string &offer);

void Senddata(std::string &name, std::string &data);

void Disconnect(std::vector<std::string> &);

} // namespace GameNetwork

#endif
