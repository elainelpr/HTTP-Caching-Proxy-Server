//
//  main.cpp
//  proxy
//
//  Created by Elaine on 2022/2/11.
//

#include <iostream>
#include "server.hpp"
#include <string>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "socket.hpp"
#include "client.hpp"
using namespace std;
int main(){
    Server server_test("192.168.1.38", "12345");
    server_test.Accept();
    server_test.Receive_test();
    Client client_test(server_test.addr_client,"80");
    client_test.proxy_connet_server(server_test.proxy_send_server);
    
}
