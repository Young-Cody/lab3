#ifndef RDT_H
#define RDT_H

#include<WinSock2.h>
#include<Ws2tcpip.h>
#include<Windows.h>
#include<iostream>
#include<map>
#include"sbuf.h"
#include"rbuf.h"
#include"connection.h"
#include"packet.h"
using namespace std;
#pragma comment(lib, "ws2_32")

#define connTimeOut 75000;	//建立连接超时时间

DWORD WINAPI sendThread(LPVOID);				//发送数据报线程
DWORD WINAPI recvThread(LPVOID);				//接收数据报线程
DWORD WINAPI listenThread(LPVOID);				//服务器监听连接线程

namespace rdt{
int bind(SOCKET, const sockaddr *);				//将socket和地址绑定
int connect(SOCKET, const sockaddr *, int);		//建立连接
int listen(SOCKET, int);						//监听并接受连接
int send(SOCKET, const char *, int);			//应用层发送数据，将数据写入发送缓冲区
SOCKET accept(SOCKET);							//应用层接受连接
int close(SOCKET);                              //关闭连接
int recv(SOCKET, char *, int);					//将接收缓冲区中的数据返回给应用层
}
#endif
