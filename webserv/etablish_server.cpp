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
void set_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

std::map<int, std::string> parse_request(std::string request)
{
    int start = 5;
    int index = 0;
    std::map<int, std::string> map;
    size_t end = request.find('\n');
    std::string str;
    while (end != std::string::npos)
    {

        if (!index)
        {
            str = request.substr(4, end - 14);
            map[index] = str;
            index++;
        }
        else
        {
            start = request.find(' ');
            map[index++] = request.substr(start + 1, (end - start) - 2);
        }
        request = request.substr(end + 1);

        end = request.find('\n');
    }
    return map;
}

void manage_timeout(std::map<int , struct client> &activity){
    while (1)
    {
              
            for (auto it = activity.begin(); it != activity.end();) {
                if (it->second.start)
        if (time(NULL) - it->second.timestamp > 5) { 
            close(it->first);
            it = activity.erase(it);
        } else {
            ++it;
        }
    }

}
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

}

void handle_http_request(int client_fd, std::map<int, struct client> *activity)
{
    struct client cl = (*activity)[client_fd];
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);

    if (bytes_read <= 0)
    {
        close(client_fd);
        return;
    }
    buffer[bytes_read] = '\0';
    std::map<int, std::string> map = parse_request(buffer);
    std::string request(buffer);
    std::ifstream file("./reponse/403.html");
    std::ifstream file2("./reponse/403.html");
    std::stringstream response_buffer;
    response_buffer << file.rdbuf();
    std::string html_content = response_buffer.str();
    std::string path = "./static_files" + map[0];
    if (request.find("GET") != std::string::npos)
    {
        struct client new_client;
        new_client.start = 0;

        if (access(path.c_str(), F_OK) == -1)
        {
            response_buffer << file.rdbuf();
            std::string html_content = response_buffer.str();
            std::string response = "HTTP/1.1 402 File not found \r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n"
                                   "\r\n" +
                                   html_content;
            write(client_fd, response.c_str(), response.length());
        }
        if (access(path.c_str(), R_OK) == -1)
        {
            response_buffer << file2.rdbuf();
            std::string html_content = response_buffer.str();
            std::string response = "HTTP/1.1 403 Forbidden  \r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n"
                                   "\r\n" +
                                   html_content;
            write(client_fd, response.c_str(), response.length());
        }
    }
    //     if (request.find("DELETE") != std::string::npos)
    // {
    //     int start = 5;
    //     size_t end = request.find(' ',start);
    //     std:: string filename = "/" + request.substr(5,end-start);
    //     std::cout <<"delete request is called " <<  filename << std::endl;

    //         close(client_fd);
    //             return ;
    //     }

    std::ifstream file1;

    std::cout << "hi " << map[0] << std::endl;
    std::stringstream body;
    if (map[0] == "/")
    {
        std::cout << "heloo" << std::endl;
        file1.open("./static_files/index.html");
    }
    else
    {
        file1.open(path);
    }
body << file1.rdbuf();
std::string body_str = body.str();
std::string content_length = std::to_string(body_str.length());


    struct client check = (*activity)[client_fd];

    bool keep_alive = false;
    if (map.find(6) != map.end() && map[6].find("keep-alive") != std::string::npos)
    {
        keep_alive = true;
    }

    std::string response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: " + content_length + "\r\n"
                       "Connection: " + (keep_alive ? "keep-alive" : "close") + "\r\n"
                       "\r\n" +
                       body_str;

send(client_fd, response.c_str(), response.length(), 0);

    if (keep_alive)
    {
        (*activity)[client_fd] = {1, time(NULL)};
        std::cout << "Keeping connection " << client_fd << " alive. New timestamp: " << time(NULL) << std::endl;
        return;
    }
    else
    {
        std::cout << "Closing connection " << client_fd << " (not keep-alive)" << std::endl;
        close(client_fd);
        activity->erase(client_fd);
        return;
    }

}

int etablish_server(struct sockaddr_in *server_addr)
{
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return EXIT_FAILURE;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    std::memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) == -1)
    {
        std::cerr << "Error binding socket" << std::endl;
        return EXIT_FAILURE;
    }

    if (listen(server_fd, SOMAXCONN) == -1)
    {
        std::cerr << "Error listening on socket" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Server listening on port " << SERVER_PORT << std::endl;

    set_nonblocking(server_fd);
    return server_fd;
}

int main()
{

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int server_fd = etablish_server(&server_addr);
    int client_fd, epoll_fd;
    struct epoll_event event, events[MAX_EVENTS];

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        std::cerr << "Error creating epoll instance" << std::endl;
        return EXIT_FAILURE;
    }

    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        std::cerr << "Error adding server socket to epoll" << std::endl;
        return EXIT_FAILURE;
    }
    std::map<int, struct client> activity;
std::thread timeout_thread(manage_timeout, std::ref(activity));
    timeout_thread.detach(); 
    while (true)
    {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1)
        {
            std::cerr << "Error in epoll_wait" << std::endl;
            return EXIT_FAILURE;
        }

        for (int i = 0; i < num_events; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd == -1)
                {
                    continue;
                }
                struct client cl = {0, time(NULL)};
                activity[client_fd] = cl;

                std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr)
                          << ":" << ntohs(client_addr.sin_port) << std::endl;

                set_nonblocking(client_fd);

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
                {
                    close(client_fd);
                }
            }
            else
            {
                handle_http_request(events[i].data.fd, &activity);
            }
            
        }

}
    close(server_fd);
    close(epoll_fd);
    return 0;
}