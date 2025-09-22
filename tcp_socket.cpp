// tcp_socket.cpp
#include "tcp_socket.h"

int create_tcp_socket() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("Socket creation error");
        return -1;
    }
    return sock_fd;
}

int set_socket_reuse(int sock_fd) {
    int on = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) {
        perror("Setsockopt error");
        return -1;
    }
    return 0;
}

int create_server_socket(const char* host, int port) {
    int sock_fd = create_tcp_socket();
    if (sock_fd == -1) return -1;
    
    if (set_socket_reuse(sock_fd) == -1) {
        close(sock_fd);
        return -1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_aton(host, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);
    
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding error");
        close(sock_fd);
        return -1;
    }
    
    if (listen(sock_fd, 5) == -1) {
        perror("Listening error");
        close(sock_fd);
        return -1;
    }
    
    return sock_fd;
}

int connect_to_server(const char* host, int port) {
    int sock_fd = create_tcp_socket();
    if (sock_fd == -1) return -1;
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_aton(host, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);
    
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection error");
        close(sock_fd);
        return -1;
    }
    
    return sock_fd;
}

int send_data(int sock_fd, const char* data) {
    return send(sock_fd, data, strlen(data), 0);
}

int recv_data(int sock_fd, char* buffer, int buffer_size) {
    return recv(sock_fd, buffer, buffer_size, 0);
}

void close_socket(int sock_fd) {
    close(sock_fd);
}
