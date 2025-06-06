#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <map>
#include <thread>   

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024
#define SERVER_PORT 8080



struct client
{
    int start;
    time_t timestamp;
};

void handle_get(int , std::map<int, client>*, std::map<int, std::string>);


// class request 
// {
    
// }

#endif 