//
//  main.cpp
//  proxy
//
//  Created by Elaine on 2022/2/11.
//

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "proxy.hpp"
#include <pthread/pthread.h>
using namespace std;
int main(){
    Proxy test;
    test.proxy_bindListen_accept();
    
    test.proxyServer_serverClient(NULL);
}
