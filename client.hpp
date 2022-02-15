//
//  client.hpp
//  proxy
//
//  Created by Elaine on 2022/2/12.
//

#ifndef client_hpp
#define client_hpp

#include <stdio.h>
#include "socket.hpp"
#include <vector>
class Client:Socket{
private:
    std::vector<char> proxy_recv_mssg;
    size_t recv_resp;
public:
    Client(const char *_hostname, const char *_port):Socket(_hostname, _port){}
    void proxy_connet_server(std::string proxy_send_server);
    //void proxy_connet_server();
    void proxy_client(int accept_fd);
    ~Client();
};
#endif /* client_hpp */
