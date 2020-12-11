#ifndef RDT_H
#define RDT_H
#include"packet.h"
#include<WinSock2.h>
#include<Ws2tcpip.h>
#include<iostream>
#include<map>
using namespace std;

#define connTimeOut 75000                       //建立连接超时时间
#define check(p) (getCheckSum(p) == 0xffff)     //检查校验和

namespace rdt{

int bind(SOCKET s, const sockaddr *addr);           //将socket和地址绑定
int connect(SOCKET s, const sockaddr *name);        //建立连接
int send(SOCKET s, const char *buf, int len);       //应用层发送数据
int accept(SOCKET s);                               //接受连接
int close(SOCKET s);                                //关闭连接
int recv_deliver(SOCKET s, char *buf, int len);     //接受数据并返回给应用层
void makePkt(SOCKET s, packet *p, const char *buf, int len);        //封装数据包
void makeAckPkt(SOCKET s, packet *ackPkt, unsigned char ackNum);    //封装ack数据包   

}

struct RTO{
    void addSampleRTT(double);  //增加一次RTT，估算下一次RTO
    double EstimatedRTT;
    double DevRTT;
    double rto;
};

unsigned short getCheckSum(packet *);   //计算校验和


#endif