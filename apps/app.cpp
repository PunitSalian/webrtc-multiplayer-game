#include <iostream>
#include <network.h>
int main() {
    //Game_network g;
    //g.connect("Punit","JOhn");
    std::string name = "Punit";
    GameNetwork::NetworkInit(name);
    std::vector<std::string> inp= {"John"};
    GameNetwork::Connect(inp);

    while(1);

    return 0;
}
