// tcp_socket.h (Linux/POSIX version)
#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <string>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// Tag for dispatching to the private constructor
struct accepted_socket_tag {};

// TCP Socket class, a wrapper for POSIX Sockets
class TCPSocket {
private:
    int sock_fd;
    bool is_server;

public:
    // Client constructor
    TCPSocket();    
    // Server constructor
    TCPSocket(int port, const std::string& host = "0.0.0.0");
    // Destructor
    ~TCPSocket();    
    // Connect to a server (for clients)
    bool connect(const std::string& host, int port);
    // Start listening (for servers)
    bool listen(int backlog = 5);    
    // Accept a client connection (for servers)
    std::unique_ptr<TCPSocket> accept();
    // Send data
    int send(const std::string& data);
    // Receive data
    std::string recv(int buffer_size = 1024);
    // Close the connection
    void close();    
    // Check if the socket is valid
    bool is_valid() const;
    // Get the last error message
    static std::string get_last_error();

private:
    // Private constructor for accepted sockets, using tag dispatch
    TCPSocket(int connected_socket, accepted_socket_tag);
};

#endif // TCP_SOCKET_H