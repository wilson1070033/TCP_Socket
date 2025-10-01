// tcp_socket.cpp
#include "tcp_socket.h"
#include <iostream>

using namespace std;

// 初始化 WSA
bool TCPSocket::initialize_wsa() {
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        cerr << "WSAStartup 失敗，錯誤代碼: " << result << endl;
        return false;
    }
    wsa_initialized = true;
    return true;
}

// 清理 WSA
void TCPSocket::cleanup_wsa() {
    if (wsa_initialized) {
        WSACleanup();
        wsa_initialized = false;
    }
}

// 客戶端建構函式
TCPSocket::TCPSocket() 
    : sock_fd(INVALID_SOCKET), is_server(false), wsa_initialized(false) {
    
    if (!initialize_wsa()) return;
    
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == INVALID_SOCKET) {
        cerr << "建立 Socket 失敗: " << get_last_error() << endl;
        cleanup_wsa();
    }
}

// 伺服器建構函式
TCPSocket::TCPSocket(int port, const string& host) 
    : sock_fd(INVALID_SOCKET), is_server(true), wsa_initialized(false) {
    
    if (!initialize_wsa()) return;
    
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == INVALID_SOCKET) {
        cerr << "建立 Socket 失敗: " << get_last_error() << endl;
        cleanup_wsa();
        return;
    }
    
    // 允許位址重用
    int reuse = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, 
                   (const char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
        cerr << "設定 SO_REUSEADDR 失敗: " << get_last_error() << endl;
    }
    
    // 設定伺服器位址
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        cerr << "無效的 IP 位址: " << host << endl;
        close();
        return;
    }
    
    // 綁定 Socket
    if (::bind(sock_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "綁定失敗: " << get_last_error() << endl;
        close();
        return;
    }
}

// 用於 accept() 的私有建構函式
TCPSocket::TCPSocket(SOCKET connected_socket) 
    : sock_fd(connected_socket), is_server(false), wsa_initialized(false) {
}

// 解構函式
TCPSocket::~TCPSocket() {
    close();
    cleanup_wsa();
}

// 連接到伺服器
bool TCPSocket::connect(const string& host, int port) {
    if (!is_valid()) {
        cerr << "Socket 無效" << endl;
        return false;
    }
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        cerr << "無效的 IP 位址: " << host << endl;
        return false;
    }
    
    if (::connect(sock_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "連接失敗: " << get_last_error() << endl;
        return false;
    }
    
    return true;
}

// 開始監聽
bool TCPSocket::listen(int backlog) {
    if (!is_valid() || !is_server) {
        cerr << "Socket 無效或非伺服器" << endl;
        return false;
    }
    
    if (::listen(sock_fd, backlog) == SOCKET_ERROR) {
        cerr << "監聽失敗: " << get_last_error() << endl;
        return false;
    }
    
    return true;
}

// 接受客戶端連線
unique_ptr<TCPSocket> TCPSocket::accept() {
    if (!is_valid() || !is_server) {
        cerr << "Socket 無效或非伺服器" << endl;
        return nullptr;
    }
    
    sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    SOCKET client_socket = ::accept(sock_fd, (sockaddr*)&client_addr, &client_len);
    
    if (client_socket == INVALID_SOCKET) {
        cerr << "接受連線失敗: " << get_last_error() << endl;
        return nullptr;
    }
    
    return unique_ptr<TCPSocket>(new TCPSocket(client_socket));
}

// 傳送資料
int TCPSocket::send(const string& data) {
    if (!is_valid()) {
        cerr << "Socket 無效" << endl;
        return -1;
    }
    
    int bytes_sent = ::send(sock_fd, data.c_str(), static_cast<int>(data.length()), 0);
    
    if (bytes_sent == SOCKET_ERROR) {
        cerr << "傳送失敗: " << get_last_error() << endl;
        return -1;
    }
    
    return bytes_sent;
}

// 接收資料
string TCPSocket::recv(int buffer_size) {
    if (!is_valid()) {
        cerr << "Socket 無效" << endl;
        return "";
    }
    
    char* buffer = new char[buffer_size + 1];
    memset(buffer, 0, buffer_size + 1);
    
    int bytes_received = ::recv(sock_fd, buffer, buffer_size, 0);
    
    if (bytes_received == SOCKET_ERROR) {
        cerr << "接收失敗: " << get_last_error() << endl;
        delete[] buffer;
        return "";
    }
    
    if (bytes_received == 0) {
        delete[] buffer;
        return "";
    }
    
    string result(buffer, bytes_received);
    delete[] buffer;
    
    return result;
}

// 關閉 Socket
void TCPSocket::close() {
    if (sock_fd != INVALID_SOCKET) {
        closesocket(sock_fd);
        sock_fd = INVALID_SOCKET;
    }
}

// 檢查 Socket 是否有效
bool TCPSocket::is_valid() const {
    return sock_fd != INVALID_SOCKET;
}

// 取得錯誤訊息
string TCPSocket::get_last_error() {
    int error_code = WSAGetLastError();
    
    char* message = nullptr;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&message,
        0,
        nullptr
    );
    
    string result = message ? message : "未知錯誤";
    LocalFree(message);
    
    return result + " (錯誤代碼: " + to_string(error_code) + ")";
}
