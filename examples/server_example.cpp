///////////////////////////////////////////////////////////
/////////////////////SERVER EXAMPLE////////////////////////
///////////////////////////////////////////////////////////

#ifdef SERVER_EXAMPLE

#include <iostream>
#include <csignal>
#include <condition_variable>
#include <string.h>

#include "../include/tcp_server.h"


// declare the server
TcpServer server;

// declare a server observer which will receive incomingPacketHandler messages.
// the server supports multiple observers
server_observer_t observer;

bool shouldSaveMsg = false;
char msgBuffer[10] = {0};

std::mutex mtx;
std::condition_variable server_ready;
bool canAcceptNextClient = true;

// accept a single client.
// if you wish to accept multiple clients, call this function in a loop
// (you might want to use a thread to accept clients without blocking)
void acceptClient() {
    try {
        std::cout << "waiting for incoming client...\n";
        std::string clientIP = server.acceptClient(0);
        std::cout << "accepted new client with IP: " << clientIP << "\n" <<
                  "== updated list of accepted clients ==" << "\n";
        server.printClients();
    } catch (const std::runtime_error &error) {
        std::cout << "Accepting client failed: " << error.what() << "\n";
    }
}


// observer callback. will be called for every new message received by clients
// with the requested IP address
void onIncomingMsg(const std::string &clientIP, const char * msg, size_t size) {
    std::string msgStr = msg;
    if (msgStr == "S") {
        shouldSaveMsg = true;
    } else if (shouldSaveMsg) {
        if (strncmp(msgStr.c_str(), "M", 1) == 0) {
            memcpy(msgBuffer, msg, size);
        }
        shouldSaveMsg = false;
    }
    // print client message
    std::cout << "Observer1 got client msg: " << msgStr << "\n";
}

// observer callback. will be called when client disconnects
void onClientDisconnected(const std::string &ip, const std::string &msg) {
    std::cout << "Client: " << ip << " disconnected. Reason: " << msg << "\n";
    {
        std::lock_guard<std::mutex> lock(mtx);
        canAcceptNextClient = true;
        server_ready.notify_one();
    }
}

void printMenu() {
    std::cout << "\n\nselect one of the following options: \n" <<
              "1. send all clients a message\n" <<
              "2. print list of accepted clients\n" <<
              "3. send message to a specific client\n" <<
              "4. close server and exit\n";
}

int getMenuSelection() {
    int selection = 0;
    std::cin >> selection;
    if (!std::cin) {
        throw std::runtime_error("invalid menu input. expected a number, but got something else");
    }
    std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
    return selection;
}

/**
 * handle menu selection and return true in case program should terminate
 * after handling selection
 */
bool handleMenuSelection(int selection) {
    static const int minSelection = 1;
    static const int maxSelection = 4;
    if (selection < minSelection || selection > maxSelection) {
        std::cout << "invalid selection: " << selection <<
                  ". selection must be b/w " << minSelection << " and " << maxSelection << "\n";
        return false;
    }
    switch (selection) {
        case 1: { // send all clients a message
            std::string msg;
            std::cout << "type message to send to all connected clients:\n";
            getline(std::cin, msg);
            pipe_ret_t sendingResult = server.sendToAllClients(msg.c_str(), msg.size());
            if (sendingResult.isSuccessful()) {
                std::cout << "sent message to all clients successfully\n";
            } else {
                std::cout << "failed to sent message: " << sendingResult.message() << "\n";
            }
            break;
        }
        case 2: { // print list of accepted clients
            server.printClients();
            break;
        }
        case 3: { // send message to a specific client
            std::cout << "enter client IP:\n";
            std::string clientIP;
            std::cin >> clientIP;
            std::cout << "enter message to send:\n";
            std::string message;
            std::cin >> message;
            pipe_ret_t result = server.sendToClient(clientIP, message.c_str(), message.size());
            if (!result.isSuccessful()) {
                std::cout << "sending failed: " << result.message() << "\n";
            } else {
                std::cout << "sending succeeded\n";
            }
            break;
        };
        case 4: { // close server
            pipe_ret_t sendingResult = server.close();
            if (sendingResult.isSuccessful()) {
                std::cout << "closed server successfully\n";
            } else {
                std::cout << "failed to close server: " << sendingResult.message() << "\n";
            }
            return true;
        }
        default: {
            std::cout << "invalid selection: " << selection <<
                      ". selection must be b/w " << minSelection << " and " << maxSelection << "\n";
        }
    }
    return false;
}

int main()
{
    // start server on port 65123
    pipe_ret_t startRet = server.start(65123);
    if (startRet.isSuccessful()) {
        std::cout << "Server setup succeeded\n";
    } else {
        std::cout << "Server setup failed: " << startRet.message() << "\n";
        return EXIT_FAILURE;
    }

    // configure and register observer
    observer.incomingPacketHandler = onIncomingMsg;
    observer.disconnectionHandler = onClientDisconnected;
    observer.wantedIP = "127.0.0.1";
    server.subscribe(observer);

    while (true) {
        acceptClient();
        {
            std::lock_guard<std::mutex> lock(mtx);
            canAcceptNextClient = false;
        }
        std::unique_lock<std::mutex> lock(mtx);
        server_ready.wait(lock, []{return canAcceptNextClient;});
    }
}

#endif