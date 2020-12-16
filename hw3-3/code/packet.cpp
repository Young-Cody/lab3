#include"packet.h"

//封装数据报
void makePkt(packet* p, const char* buf, unsigned int len, unsigned int seq)
{
    memset(p, 0, sizeof(packet) - MAXBUFSIZE + len);
    p->seq = seq;
    for (unsigned int i = 0; i < len; i++)
        p->data[i] = buf[i];
    p->seq = seq;
    p->dataLen = len;
    p->checkSum = ~getCheckSum(p);
}

//封装ack数据报
void makeAckPkt(packet* ackPkt, unsigned int ackNum)
{
    memset(ackPkt, 0, sizeof(packet));
    ackPkt->flag = setAck(ackPkt->flag);
    ackPkt->ack = ackNum;
    ackPkt->checkSum = ~getCheckSum(ackPkt);
}

//封装syn数据报
void makeSynPkt(packet* synPkt, unsigned short windowSize)
{
    memset(synPkt, 0, sizeof(packet));
    synPkt->flag = setSyn(synPkt->flag);
    synPkt->N = windowSize;
    synPkt->checkSum = ~getCheckSum(synPkt);
}

//封装fin数据报
void makeFinPkt(packet* finPkt, unsigned int seq)
{
    memset(finPkt, 0, sizeof(packet));
    finPkt->flag = setFin(finPkt->flag);
    finPkt->seq = seq;
    finPkt->checkSum = ~getCheckSum(finPkt);
}

//计算检验和
unsigned short getCheckSum(packet* p)
{
    unsigned int ans = 0;
    unsigned short res;
    ans += p->seq >> 16;
    ans += p->seq & 0xffff;
    ans += p->ack >> 16;
    ans += p->ack & 0xffff;
    ans += p->N;
    ans += (unsigned short)p->flag << 8;
    ans += p->checkSum;
    ans += p->dataLen;
    for (int i = 0; i < p->dataLen; i++)
        if (i % 2) ans += p->data[i];
        else ans += (unsigned short)p->data[i] << 8;
    ans = (ans >> 16) + (ans & 0xffff);
    res = (ans >> 16) + (ans & 0xffff);
    return res;
}