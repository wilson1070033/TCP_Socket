# TCP Socket for Windows (C++)

這是一個專為 Windows 平台設計的 TCP Socket 函式庫，採用物件導向方式封裝 Winsock API，提供類似 Python socket 的簡潔介面，讓開發者能夠輕鬆建立 TCP 伺服器與客戶端應用程式。

---

## 專案特色

- **物件導向設計**：採用 C++ 類別封裝，提供清晰的 API 介面
- **自動資源管理**：使用 RAII 模式，自動管理 Socket 生命週期與 WSA 初始化
- **簡潔易用**：API 設計參考 Python socket，降低學習曲線
- **完整錯誤處理**：提供詳細的錯誤訊息與錯誤代碼
- **Windows 原生支援**：基於 Winsock2 API 開發，完全相容 Windows 平台
- **位址重用設定**：內建 `SO_REUSEADDR` 選項，避免埠號佔用問題

---

## 系統需求

- **作業系統**：Windows 7 或更新版本
- **編譯器**：支援 C++11 或更新標準的編譯器（Visual Studio 2015+、MinGW、Clang）
- **依賴函式庫**：Winsock2 (`ws2_32.lib`)

---

## API 文件

### TCPSocket 類別

#### 建構函式

**客戶端模式**
```cpp
TCPSocket()
```
建立一個客戶端 Socket，用於連接到遠端伺服器。

**伺服器模式**
```cpp
TCPSocket(int port, const std::string& host = "0.0.0.0")
```
- `port`：要監聽的通訊埠號
- `host`：要綁定的 IP 位址（預設為 "0.0.0.0"，接受所有介面的連線）

建立一個伺服器 Socket 並綁定到指定的位址與通訊埠。

---

#### 成員函式

**connect()**
```cpp
bool connect(const std::string& host, int port)
```
連接到指定的遠端伺服器（客戶端使用）。

- **參數**：
  - `host`：伺服器 IP 位址
  - `port`：伺服器通訊埠號
- **回傳值**：成功回傳 `true`，失敗回傳 `false`

---

**listen()**
```cpp
bool listen(int backlog = 5)
```
開始監聽連線請求（伺服器使用）。

- **參數**：
  - `backlog`：等待連線佇列的最大長度，預設為 5
- **回傳值**：成功回傳 `true`，失敗回傳 `false`

---

**accept()**
```cpp
std::unique_ptr<TCPSocket> accept()
```
接受一個客戶端連線（伺服器使用）。

- **回傳值**：成功回傳新的 `TCPSocket` 智慧指標，失敗回傳 `nullptr`

---

**send()**
```cpp
int send(const std::string& data)
```
傳送字串資料。

- **參數**：
  - `data`：要傳送的字串資料
- **回傳值**：成功回傳傳送的位元組數，失敗回傳 `-1`

---

**recv()**
```cpp
std::string recv(int buffer_size = 1024)
```
接收資料。

- **參數**：
  - `buffer_size`：接收緩衝區大小，預設為 1024 位元組
- **回傳值**：接收到的字串資料，若連線關閉或失敗則回傳空字串

---

**close()**
```cpp
void close()
```
關閉 Socket 連線。

---

**is_valid()**
```cpp
bool is_valid() const
```
檢查 Socket 是否處於有效狀態。

- **回傳值**：Socket 有效回傳 `true`，否則回傳 `false`

---

**get_last_error()**
```cpp
static std::string get_last_error()
```
取得最後的 Winsock 錯誤訊息（靜態方法）。

- **回傳值**：包含錯誤描述與錯誤代碼的字串

---

## 使用範例

### 伺服器範例

建立一個簡單的 Echo 伺服器，接收客戶端訊息並回覆。

```cpp
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
        client->send("Echo: " + message);
    }
    
    return 0;
}
```

---

### 客戶端範例

建立一個客戶端，連接到伺服器並傳送訊息。

```cpp
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
    client.send("Hello, Server!");
    
    // 接收回覆
    string response = client.recv();
    if (!response.empty()) {
        cout << "收到: " << response << endl;
    }
    
    return 0;
}
```

---

## 編譯指南

### 使用 Visual Studio

1. 建立新的 C++ 主控台專案
2. 將 `tcp_socket.h`、`tcp_socket.cpp` 以及範例檔案加入專案
3. 確認專案設定中已連結 `ws2_32.lib`
4. 編譯並執行

### 使用 g++ (MinGW)

```bash
# 編譯伺服器
g++ -std=c++11 server_example.cpp tcp_socket.cpp -o server.exe -lws2_32

# 編譯客戶端
g++ -std=c++11 client_example.cpp tcp_socket.cpp -o client.exe -lws2_32
```

### 使用 CMake

建立 `CMakeLists.txt` 檔案：

```cmake
cmake_minimum_required(VERSION 3.10)
project(TCPSocket)

set(CMAKE_CXX_STANDARD 11)

# 伺服器
add_executable(server server_example.cpp tcp_socket.cpp)
target_link_libraries(server ws2_32)

# 客戶端
add_executable(client client_example.cpp tcp_socket.cpp)
target_link_libraries(client ws2_32)
```

執行編譯：

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

---

## 執行範例

1. 先啟動伺服器：
```bash
./server.exe
```

2. 在另一個終端機視窗啟動客戶端：
```bash
./client.exe
```

您應該會看到伺服器接收訊息並回覆，客戶端也會顯示收到的回覆。

---

## 注意事項

- 本函式庫僅支援 IPv4 通訊協定
- Socket 物件會在解構時自動關閉連線並清理資源
- `accept()` 方法回傳的是智慧指標，會自動管理記憶體
- 錯誤訊息會輸出到 `std::cerr`，並可透過 `get_last_error()` 取得詳細資訊
- 建議在生產環境中加入更完善的錯誤處理與日誌記錄機制

---

## 授權條款

本專案採用 MIT 授權條款。您可以自由使用、修改與分發本程式碼。

---

## 技術支援

如有任何問題或建議，請透過 GitHub Issues 回報。
