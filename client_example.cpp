// client_example.cpp
#include "tcp_socket.h"
#include <iostream>

using namespace std;

int main() {
    // 建立客戶端
    TCPSocket client;
    
    if (!client.is_valid()) {
        cerr << "無法建立客戶端" << endl;
        return 1;
    }
    
    // 連接到伺服器
    if (!client.connect("127.0.0.1", 8080)) {
        cerr << "連接失敗" << endl;
        return 1;
    }
    
    cout << "已連接到伺服器" << endl;
    
    // 傳送訊息
    client.send("你好，伺服器！");
    
    // 接收回覆
    string response = client.recv();
    if (!response.empty()) {
        cout << "收到: " << response << endl;
    }
    
    return 0;
}
