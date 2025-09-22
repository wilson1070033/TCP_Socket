// tcp_socket.h
#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// 建立 TCP Socket
int create_tcp_socket();

// 設定 Socket 選項（解決 "Address already in use" 問題）
int set_socket_reuse(int sock_fd);

// 建立伺服器 Socket 並綁定
int create_server_socket(const char* host, int port);

// 連接到伺服器
int connect_to_server(const char* host, int port);

// 傳送資料
int send_data(int sock_fd, const char* data);

// 接收資料
int recv_data(int sock_fd, char* buffer, int buffer_size);

// 關閉 Socket
void close_socket(int sock_fd);

#endif
