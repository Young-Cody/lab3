#ifndef RBUF_H
#define RBUF_H
#include<iostream>
#include<Windows.h>
#include"packet.h"
using namespace std;

//接收缓冲区
class RBuf
{
public:
    RBuf(int, SOCKET, SOCKET, sockaddr);    //初始化接收缓冲区
    ~RBuf();
    void read(char*, int);                  //应用层读接受缓冲区的数据
    bool recv();                            //rdt接收数据，放入缓冲区
    void getSocket(SOCKET*);                //返回连接socket
private:
    char** buf;                             //缓冲区
    int n;                                  //缓冲区大小
    unsigned int front;                     //缓冲区队首，指向应用层下一个要读的数据报
    unsigned int rear;                      //缓冲区队尾，指向按序接收到的最后一个数据报，之后是失序数据报
    unsigned int expected;                  //接收缓冲期待接收的下一数据报的序列号
    SOCKET cs;                              //连接socket
    SOCKET s;                               //服务器socket
    sockaddr addr;                          //对方的地址
    HANDLE mutex;                           //互斥信号量
    HANDLE slots;                           //缓冲区剩余位置计数信号量
    HANDLE items;                           //缓冲区按序接收到的数据报数目计数信号量
};

#endif
