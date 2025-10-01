// server_example.cpp
#include "tcp_socket.h"
#include <iostream>

using namespace std;

int main() {
    // 建立伺服器，監聽 8080 埠
    TCPSocket server(8080, "127.0.0.1");
    
    if (!server.is_valid()) {
        cerr << "無法建立伺服器" << endl;
        return 1;
    }
    
    if (!server.listen()) {
        cerr << "監聽失敗" << endl;
        return 1;
    }
    
    cout << "伺服器正在監聽 127.0.0.1:8080" << endl;
    
    // 接受客戶端連線
    auto client = server.accept();
    if (!client) {
        cerr << "接受連線失敗" << endl;
        return 1;
    }
    
    cout << "客戶端已連線" << endl;
    
    // 接收並回覆訊息
    string message = client->recv();
    if (!message.empty()) {
        cout << "收到: " << message << endl;
        client->send("你好，客戶端！");
    }
    
    return 0;
}
