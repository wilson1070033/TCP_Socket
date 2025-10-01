// tcp_socket.h
#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <memory>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// TCP Socket 類別，封裝 Windows Socket API
class TCPSocket {
private:
    SOCKET sock_fd;
    bool is_server;
    bool wsa_initialized;
    bool initialize_wsa();
    void cleanup_wsa();
public:
    // 客戶端建構函式
    TCPSocket();    
    // 伺服器建構函式
    TCPSocket(int port, const string& host = "0.0.0.0");    
    // 解構函式
    ~TCPSocket();    
    // 連接到伺服器（客戶端）
    bool connect(const string& host, int port);
    // 開始監聽（伺服器）
    bool listen(int backlog = 5);    
    // 接受客戶端連線（伺服器）
    unique_ptr<TCPSocket> accept();    
    // 傳送資料
    int send(const string& data);
    // 接收資料
    string recv(int buffer_size = 1024);    
    // 關閉連線
    void close();    
    // 檢查 Socket 是否有效
    bool is_valid() const;
    // 取得最後的錯誤訊息
    static string get_last_error();
private:
    // 用於 accept() 建立已連接的 Socket
    TCPSocket(SOCKET connected_socket);
};

#endif // TCP_SOCKET_H
