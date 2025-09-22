# TCP Socket in C++

這是一個使用 C++ 語言實作的 TCP Socket 函式庫，封裝了基本的 TCP 連線功能，讓使用者可以輕鬆地建立 TCP 伺服器和客戶端。

## 專案特色

  * **簡易的 API**: 提供清晰且易於使用的函式，用於建立、綁定、監聽、連接和關閉 Socket。
  * **伺服器與客戶端**: 同時支援 TCP 伺服器端與客戶端的實作。
  * **位址重用**: 內建 `SO_REUSEADDR` 設定，避免因 "Address already in use" 錯誤而導致伺服器無法立即重啟。
  * **錯誤處理**: 透過 `perror` 提供基本的錯誤訊息輸出。

## API 文件

這個函式庫提供了以下函式來操作 TCP Socket：

### `int create_tcp_socket()`

  * **功能**: 建立一個新的 TCP Socket。
  * **回傳值**:
      * 成功時，回傳一個代表 Socket 的檔案描述符 (file descriptor)。
      * 失敗時，回傳 `-1`。

### `int set_socket_reuse(int sock_fd)`

  * **功能**: 設定 Socket 選項，允許位址和通訊埠的重用。這主要用於解決伺服器重啟時可能遇到的 "Address already in use" 問題。
  * **參數**:
      * `sock_fd`: 要設定的 Socket 檔案描述符。
  * **回傳值**:
      * 成功時，回傳 `0`。
      * 失敗時，回傳 `-1`。

### `int create_server_socket(const char* host, int port)`

  * **功能**: 建立一個 TCP 伺服器 Socket，並將其綁定到指定的 IP 位址和通訊埠，最後進入監聽狀態。
  * **參數**:
      * `host`: 一個指向字串的指標，代表要綁定的 IP 位址 (例如 "127.0.0.1")。
      * `port`: 要綁定的通訊埠號。
  * **回傳值**:
      * 成功時，回傳一個已在監聽狀態的伺服器 Socket 檔案描述符。
      * 失敗時，回傳 `-1`。

### `int connect_to_server(const char* host, int port)`

  * **功能**: 建立一個客戶端 Socket，並連接到指定的遠端伺服器。
  * **參數**:
      * `host`: 一個指向字串的指標，代表遠端伺服器的 IP 位址。
      * `port`: 遠端伺服器的通訊埠號。
  * **回傳值**:
      * 成功時，回傳一個已連接的客戶端 Socket 檔案描述符。
      * 失敗時，回傳 `-1`。

### `int send_data(int sock_fd, const char* data)`

  * **功能**: 透過指定的 Socket 傳送資料。
  * **參數**:
      * `sock_fd`: 用於傳送資料的 Socket 檔案描述符。
      * `data`: 一個指向字串的指標，代表要傳送的資料。
  * **回傳值**:
      * 成功時，回傳已傳送的位元組數。
      * 失敗時，回傳 `-1`。

### `int recv_data(int sock_fd, char* buffer, int buffer_size)`

  * **功能**: 從指定的 Socket 接收資料。
  * **參數**:
      * `sock_fd`: 用於接收資料的 Socket 檔案描述符。
      * `buffer`: 一個字元陣列，用於儲存接收到的資料。
      * `buffer_size`: `buffer` 的大小。
  * **回傳值**:
      * 成功時，回傳接收到的位元組數。
      * 如果對方已關閉連線，回傳 `0`。
      * 失敗時，回傳 `-1`。

### `void close_socket(int sock_fd)`

  * **功能**: 關閉指定的 Socket 連線。
  * **參數**:
      * `sock_fd`: 要關閉的 Socket 檔案描述符。

## 如何使用

您可以將 `tcp_socket.h` 和 `tcp_socket.cpp` 加入您的專案中。以下是建立一個簡單伺服器和客戶端的範例。

### 伺服器範例 (`server_example.cpp`)

```cpp
#include "tcp_socket.h"
#include <iostream>

int main() {
    const char* host = "127.0.0.1";
    int port = 8080;
    
    int server_fd = create_server_socket(host, port);
    if (server_fd == -1) {
        std::cerr << "無法建立伺服器 Socket" << std::endl;
        return 1;
    }
    
    std::cout << "伺服器正在監聽 " << host << ":" << port << std::endl;
    
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    
    if (client_fd == -1) {
        perror("Accept error");
        close_socket(server_fd);
        return 1;
    }
    
    char buffer[1024] = {0};
    int bytes_received = recv_data(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_received > 0) {
        std::cout << "收到來自客戶端的訊息: " << buffer << std::endl;
        send_data(client_fd, "你好，客戶端！");
    }
    
    close_socket(client_fd);
    close_socket(server_fd);
    
    return 0;
}
```

### 客戶端範例 (`client_example.cpp`)

```cpp
#include "tcp_socket.h"
#include <iostream>

int main() {
    const char* host = "127.0.0.1";
    int port = 8080;
    
    int sock_fd = connect_to_server(host, port);
    if (sock_fd == -1) {
        std::cerr << "無法連接到伺服器" << std::endl;
        return 1;
    }
    
    const char* message = "你好，伺服器！";
    send_data(sock_fd, message);
    
    char buffer[1024] = {0};
    int bytes_received = recv_data(sock_fd, buffer, sizeof(buffer) - 1);
    if (bytes_received > 0) {
        std::cout << "收到來自伺服器的回覆: " << buffer << std::endl;
    }
    
    close_socket(sock_fd);
    
    return 0;
}
```

## 編譯與執行

您可以使用 g++ 編譯器來編譯範例程式。

```bash
# 編譯伺服器
g++ server_example.cpp tcp_socket.cpp -o server

# 編譯客戶端
g++ client_example.cpp tcp_socket.cpp -o client
```

首先執行伺服器，然後在另一個終端機視窗中執行客戶端。

```bash
# 執行伺服器
./server

# 執行客戶端
./client
```
