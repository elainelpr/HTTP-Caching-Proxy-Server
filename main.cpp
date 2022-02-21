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
#include "client.hpp"
using namespace std;
int main(){
    Server server_test("0.0.0.0", "12345");
    server_test.Accept();
    server_test.Receive_test();
    char *addrClient = server_test.get_addrClient();
    Client client_test(addrClient,"80");
    int connectFd = server_test.get_connectfd();
    string proxySendServer = server_test.get_proxySendSerevr();
    client_test.proxy_connet_server(proxySendServer,connectFd);
}
