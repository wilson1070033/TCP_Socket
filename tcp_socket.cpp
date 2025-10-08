// tcp_socket.cpp (Linux/POSIX version)
#include "tcp_socket.h"
#include <iostream>
#include <stdexcept>

// Client constructor
TCPSocket::TCPSocket() 
    : sock_fd(-1), is_server(false) {
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        std::cerr << "Failed to create socket: " << get_last_error() << std::endl;
    }
}

// Server constructor
TCPSocket::TCPSocket(int port, const std::string& host)
    : sock_fd(-1), is_server(true) {
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        std::cerr << "Failed to create socket: " << get_last_error() << std::endl;
        return;
    }

    // Allow address reuse
    int reuse = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR: " << get_last_error() << std::endl;
    }

    // Set up server address
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address: " << host << std::endl;
        close();
        return;
    }

    // Bind the socket
    if (::bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed: " << get_last_error() << std::endl;
        close();
        return;
    }
}

// Private constructor for accepted sockets, using tag dispatch
TCPSocket::TCPSocket(int connected_socket, accepted_socket_tag)
    : sock_fd(connected_socket), is_server(false) {
}

// Destructor
TCPSocket::~TCPSocket() {
    close();
}

// Connect to a server
bool TCPSocket::connect(const std::string& host, int port) {
    if (!is_valid()) {
        std::cerr << "Invalid socket" << std::endl;
        return false;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address: " << host << std::endl;
        return false;
    }

    if (::connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed: " << get_last_error() << std::endl;
        return false;
    }
    return true;
}

// Start listening
bool TCPSocket::listen(int backlog) {
    if (!is_valid() || !is_server) {
        std::cerr << "Invalid socket or not a server" << std::endl;
        return false;
    }

    if (::listen(sock_fd, backlog) < 0) {
        std::cerr << "Listen failed: " << get_last_error() << std::endl;
        return false;
    }
    return true;
}

// Accept a client connection
std::unique_ptr<TCPSocket> TCPSocket::accept() {
    if (!is_valid() || !is_server) {
        std::cerr << "Invalid socket or not a server" << std::endl;
        return nullptr;
    }

    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket_fd = ::accept(sock_fd, (struct sockaddr*)&client_addr, &client_len);

    if (client_socket_fd < 0) {
        std::cerr << "Accept failed: " << get_last_error() << std::endl;
        return nullptr;
    }
    // Use the tagged constructor to create the new socket object
    return std::unique_ptr<TCPSocket>(new TCPSocket(client_socket_fd, accepted_socket_tag{}));
}

// Send data
int TCPSocket::send(const std::string& data) {
    if (!is_valid()) {
        std::cerr << "Invalid socket" << std::endl;
        return -1;
    }

    int bytes_sent = ::send(sock_fd, data.c_str(), data.length(), 0);
    if (bytes_sent < 0) {
        std::cerr << "Send failed: " << get_last_error() << std::endl;
    }
    return bytes_sent;
}

// Receive data
std::string TCPSocket::recv(int buffer_size) {
    if (!is_valid()) {
        std::cerr << "Invalid socket" << std::endl;
        return "";
    }

    char* buffer = new char[buffer_size];
    memset(buffer, 0, buffer_size);

    int bytes_received = ::recv(sock_fd, buffer, buffer_size, 0);

    if (bytes_received < 0) {
        std::cerr << "Receive failed: " << get_last_error() << std::endl;
        delete[] buffer;
        return "";
    }

    if (bytes_received == 0) {
        // Connection closed by peer
        delete[] buffer;
        return "";
    }

    std::string result(buffer, bytes_received);
    delete[] buffer;
    return result;
}

// Close the socket
void TCPSocket::close() {
    if (sock_fd >= 0) {
        ::close(sock_fd);
        sock_fd = -1;
    }
}

// Check if the socket is valid
bool TCPSocket::is_valid() const {
    return sock_fd >= 0;
}

// Get the last error message
std::string TCPSocket::get_last_error() {
    return strerror(errno);
}