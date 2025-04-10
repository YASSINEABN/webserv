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

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

void set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void handle_http_request(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    
    buffer[bytes_read] = '\0';
        std::cout<<"request" << "\n" ;

        std::cout<< buffer << std::endl;
    
            std::cout<<"request" << "\n" ;

    std::string request(buffer);
    if (request.find("GET") != std::string::npos)
    {
        int start = 5;
        size_t end = request.find(' ',start);
        std:: string filename = request.substr(5,end-start);
        std::cout << filename << std::endl;
        if(!(std::filesystem::is_regular_file(filename)))
        {
            std::string response = "HTTP/1.1 402 File not found \r\n"
                                "Content-Type: text/html\r\n"
                                "Connection: close\r\n"
                                "\r\n"
                          "<html><body><h1>file not found!</h1></body></html>";
            
            write(client_fd, response.c_str(), response.length());
            close(client_fd);
                return ;
        }
    }

    
    std::string response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "Connection: close\r\n"
                          "\r\n"
                          "<html><body><h1>Hello from epoll web server!</h1></body></html>";
    
    write(client_fd, response.c_str(), response.length());
    close(client_fd);
}

int main() {
    int server_fd, client_fd, epoll_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct epoll_event event, events[MAX_EVENTS];
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return EXIT_FAILURE;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        return EXIT_FAILURE;
    }
    
    if (listen(server_fd, SOMAXCONN) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << "Server listening on port " << SERVER_PORT << std::endl;
    
    set_nonblocking(server_fd);
    
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::cerr << "Error creating epoll instance" << std::endl;
        return EXIT_FAILURE;
    }
    
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        std::cerr << "Error adding server socket to epoll" << std::endl;
        return EXIT_FAILURE;
    }
    
    while (true) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            std::cerr << "Error in epoll_wait" << std::endl;
            return EXIT_FAILURE;
        }
        
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_fd) {
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd == -1) {
                    continue;
                }
                
                std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) 
                          << ":" << ntohs(client_addr.sin_port) << std::endl;
                
                set_nonblocking(client_fd);
                
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                    close(client_fd);
                }
            } else {
                handle_http_request(events[i].data.fd);
            }
        }
    }
    
    close(server_fd);
    close(epoll_fd);
    return 0;
}   