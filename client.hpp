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

class Client:Socket{
private:
public:
    Client(const char *_hostname, const char *_port):Socket(_hostname, _port){}
    void proxy_connet_server( std::string proxy_send_server);
    ~Client();
};
#endif /* client_hpp */
