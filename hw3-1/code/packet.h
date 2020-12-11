#ifndef PACKET_H
#define PACKET_H

#define MAXBUFSIZE 2048
#define isAck(n) (n & 0x10)   //判断flag中的ack是否为1
#define isSyn(n) (n & 0x08)   //判断flag中的syn是否为1
#define isFin(n) (n & 0x04)   //判断flag中的fin是否为1
#define getAckNum(n) ((n & 0x02) >> 1) //得到确认号
#define getSeqNum(n) (n & 0x01) //得到序列号
#define setAck(n) (n | 0x10)  //设置flag中的ack为1
#define setSyn(n) (n | 0x08)  //设置flag中的ack为1
#define setFin(n) (n | 0x04)  //设置flag中的ack为1
#define setAckNum(n, i) (n | (i << 1)) //设置确认号
#define setSeqNum(n, i) (n | i) //设置序列号

//可靠数据传输协议
struct packet
{
    unsigned char saf;              //序列号、确认号、标志字段(ACK,SYN,FIN) 0:seqnum 1:acknum 2:finflag 3:synflag 4:ackflag
    unsigned short checkSum;        //检验和
    unsigned short dataLen;         //数据长度，以字节为单位
    unsigned char data[MAXBUFSIZE]; //数据
};



#endif
