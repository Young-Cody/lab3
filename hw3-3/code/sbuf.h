#ifndef SBUF_H
#define SBUF_H
#include<iostream>
#include<WinSock2.h>
#include<Ws2tcpip.h>
#include<Windows.h>
#include"Timer.h"
#include"packet.h"
using namespace std;

//发送缓冲区
class SBuf
{
public:
    SBuf(int, SOCKET, const sockaddr*);     //初始化发送缓冲区
    ~SBuf();                                //析构发送缓冲区
    void write(const char*,int);            //应用层向发送缓冲区写数据报
    void send();                            //传输层发送数据报
    void ack();                             //接受ack数据报
    void retrans();                         //超时重传
    bool isterminated();                    //连接是否关闭
    bool close();                           //关闭连接
private:
    char** buf;                             //发送缓冲区
    int* len;                               //各数据报的长度
    int n;                                  //发送缓冲区的大小
    int N;                                  //滑动窗口的大小
    bool istermi;                           //连接是否关闭
    unsigned int base;                      //[base, nextSeqnum) 已发送未确认的数据报,长度小于滑动窗口的大小N
    unsigned int nextSeqnum;                //[nextSeqnum, nextWrite) 应用层写入缓冲区未发送的数据报
    unsigned int nextWrite;                 
    int dupAckCnt;                          //重复收到的ack                                      
    SOCKET s;                               //标识连接的socket
    sockaddr addr;                          //连接对方的地址
    Timer *timer;                           //计时器
    RTO rto;                                //计算下一次RTO
    int cwnd;                               //拥塞窗口
    int state;                              //慢启动阶段:1 拥塞避免阶段:2 快速恢复阶段:3
    int cnt;
    int ssthresh;                           //拥塞阈值
};


#endif

