#include "tcp_socket.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

// 全域變數，用於執行緒安全的輸出
mutex cout_mutex;

// 處理單一客戶端的函式
void handle_client(unique_ptr<TCPSocket> client, int client_id) {
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "[客戶端 #" << client_id << "] 已連線" << endl;
    }
    
    // 持續接收並回覆訊息
    while (true) {
        string message = client->recv();
        
        if (message.empty()) {
            lock_guard<mutex> lock(cout_mutex);
            cout << "[客戶端 #" << client_id << "] 已斷線" << endl;
            break;
        }
        
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "[客戶端 #" << client_id << "] 收到: " << message << endl;
        }
        
        // 回覆訊息
        string response = "伺服器回覆給客戶端 #" + to_string(client_id) + ": " + message;
        client->send(response);
    }
}

int main() {
    // 建立伺服器
    TCPSocket server(8080, "127.0.0.1");
    
    if (!server.is_valid()) {
        cerr << "無法建立伺服器" << endl;
        return 1;
    }
    
    if (!server.listen(10)) {  // 允許 10 個等待連線
        cerr << "監聽失敗" << endl;
        return 1;
    }
    
    cout << "伺服器正在監聽 127.0.0.1:8080" << endl;
    cout << "等待客戶端連線..." << endl;
    
    vector<thread> threads;  // 儲存執行緒
    int client_counter = 0;
    
    // 持續接受客戶端連線
    while (true) {
        auto client = server.accept();
        
        if (!client) {
            cerr << "接受連線失敗: " << TCPSocket::get_last_error() << endl;
            continue;  // 繼續等待下一個客戶端
        }
        
        client_counter++;
        
        // 為每個客戶端建立新執行緒
        threads.emplace_back(handle_client, move(client), client_counter);
    }
    
    // 等待所有執行緒結束（實際上不會執行到這裡）
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    return 0;
}
