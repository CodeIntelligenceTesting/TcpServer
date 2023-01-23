#include <iostream>
#include "../include/tcp_client.h"
#include "../include/common.h"

// $ nm build/tcp_server | grep TcpServer.*start
extern "C" pipe_ret_t __wrap__ZN9TcpServer5startEiib(int port, int maxNumOfClients, bool removeDeadClientsAutomatically) {
    std::cout << std::endl << "Function 'TcpServer::start' wrapped" << std::endl << std::endl;
    return pipe_ret_t::success();
}