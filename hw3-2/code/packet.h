#ifndef PACKET
#define PACKET

#include<iostream>
using namespace std;

#define MAXBUFSIZE 1024       //数据的最大长度
#define check(p) (getCheckSum(p) == 0xffff) //检查校验和
#define isAck(n) (n & 0x04)   //判断flag中的ack是否为1
#define isSyn(n) (n & 0x02)   //判断flag中的syn是否为1
#define isFin(n) (n & 0x01)   //判断flag中的fin是否为1
#define setAck(n) (n | 0x04)  //设置flag中的ack为1
#define setSyn(n) (n | 0x02)  //设置flag中的syn为1
#define setFin(n) (n | 0x01)  //设置flag中的fin为1

//传输层协议
struct packet
{
    unsigned int seq;                   //序列号
    unsigned int ack;                   //确认号
    unsigned short N;                   //发送窗口长度
    unsigned char flag;                 //标志字段(ACK,SYN,FIN) 0:fin 1:syn 2:ack
    unsigned short checkSum;            //检验和
    unsigned short dataLen;             //数据长度，以字节为单位
    unsigned char data[MAXBUFSIZE];     //数据
};

void makePkt(packet*, const char*, unsigned int, unsigned int);         //封装数据报
void makeAckPkt(packet*, unsigned int);                                 //封装ack数据报   
void makeSynPkt(packet*, unsigned short);                               //封装syn数据报
void makeFinPkt(packet*, unsigned int);                                 //封装fin数据报
unsigned short getCheckSum(packet*);                                    //计算校验和

#endif
