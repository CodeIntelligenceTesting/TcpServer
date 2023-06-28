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

// observer callback. will be called for every new message received by clients
// with the requested IP address
void onIncomingMsg(const std::string &clientIP, const char *msg, size_t size) {
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

int main() {
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
    server_ready.wait(lock, [] { return canAcceptNextClient; });
  }

  return 0;
}

#endif
///////////////////////////////////////////////////////////
/////////////////////CLIENT EXAMPLE////////////////////////
///////////////////////////////////////////////////////////

#ifdef CLIENT_EXAMPLE

#include <iostream>
#include <csignal>
#include "../include/tcp_client.h"

TcpClient client;

// on sig_exit, close client
void sig_exit(int s) {
  std::cout << "Closing client...\n";
  pipe_ret_t finishRet = client.close();
  if (finishRet.isSuccessful()) {
    std::cout << "Client closed.\n";
  } else {
    std::cout << "Failed to close client.\n";
  }
  exit(0);
}

// observer callback. will be called for every new message received by the server
void onIncomingMsg(const char *msg, size_t size) {
  std::cout << "Got msg from server: " << msg << "\n";
}

// observer callback. will be called when server disconnects
void onDisconnection(const pipe_ret_t &ret) {
  std::cout << "Server disconnected: " << ret.message() << "\n";
}

int main() {
  // register to SIGINT to close client when user press ctrl+c
  signal(SIGINT, sig_exit);

  // configure and register observer
  client_observer_t observer;
  observer.wantedIP = "127.0.0.1";
  observer.incomingPacketHandler = onIncomingMsg;
  observer.disconnectionHandler = onDisconnection;
  client.subscribe(observer);

  // connect client to an open server
  bool connected = false;
  while (!connected) {
    pipe_ret_t connectRet = client.connectTo("127.0.0.1", 65123);
    connected = connectRet.isSuccessful();
    if (connected) {
      std::cout << "Client connected successfully\n";
    } else {
      std::cout << "Client failed to connect: " << connectRet.message() << "\n"
                << "Make sure the server is open and listening\n\n";
      sleep(2);
      std::cout << "Retrying to connect...\n";
    }
  }

  // send messages to server
  while (true) {
    // Fuzzed input for sending a message to the server
    std::string message = /* fuzzed message */;
    pipe_ret_t sendRet = client.sendMsg(message.c_str(), message.size());
    if (!sendRet.isSuccessful()) {
      std::cout << "Failed to send message: " << sendRet.message() << "\n";
    } else {
      std::cout << "Message was sent successfully\n";
    }
  }

  return 0;
}

#endif
