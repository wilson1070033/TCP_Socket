cmake_minimum_required(VERSION 3.10)
project(TCPSocket)

# 設定 C++ 標準
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 編譯伺服器程式
add_executable(server 
    server_example.cpp 
    tcp_socket.cpp
)
target_link_libraries(server ws2_32)

# 編譯客戶端程式
add_executable(client 
    client_example.cpp 
    tcp_socket.cpp
)
target_link_libraries(client ws2_32)

# 設定輸出目錄（可選）
set_target_properties(server client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
