#include"sbuf.h"

/*
    n:滑动窗口大小
    s:连接socket
    addr:连接对方地址
*/
SBuf::SBuf(int n, SOCKET s, const sockaddr* addr)
{
    this->n = 2 * n;
    this->s = s;
    this->addr = *addr;
    buf = new char* [2 * n];
    len = new int[2 * n];
    memset(len, 0, 2 * n);
    memset(buf, 0, sizeof(char*) * n * 2);
    istermi = false;
    rto.DevRTT = 0;
    rto.EstimatedRTT = 15;
    rto.rto = 15;
    timer = new Timer[this->n];
    N = n;
    base = nextSeqnum = nextWrite = 1;
    dupAckCnt = 0;
}

SBuf::~SBuf()
{
    delete[]len;
    delete[]buf;
}
/*
    将数据写入发送缓冲区
    buf:应用层数据
    len:数据长度
*/
void SBuf::write(const char* buf, int len)
{
    while((nextWrite - base) >= n);
    this->buf[nextWrite % n] = new char[len];
    this->len[nextWrite % n] = len;
    for (int i = 0; i < len; i++)
        this->buf[nextWrite % n][i] = buf[i];
    nextWrite++;
}

//传输层将应用层数据组装成数据报并发送
void SBuf::send()
{
    if (nextSeqnum < base + N && nextSeqnum < nextWrite)    //滑动窗口中还有可以发送的数据
    {                            
        timer[nextSeqnum % n].setTimeOut(rto.rto);                         //开始发送时，开启定时器
        timer[nextSeqnum % n].startTimer();
        timer[nextSeqnum % n].flag = true;
        packet p;
        makePkt(&p, buf[nextSeqnum % n], len[nextSeqnum % n], nextSeqnum);
        sendto(s, (char*)&p, len[nextSeqnum % n] + sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
        nextSeqnum++;
    }
}
 
//接受ack数据报
void SBuf::ack()
{
    sockaddr from = addr;
    int fromlen = sizeof(sockaddr);
    packet p;
    if(recvfrom(s, (char*)&p, sizeof(packet) - MAXBUFSIZE, 0, &from, &fromlen) < 0) return;
    if (check(&p) && isAck(p.flag))
    {
        if(p.ack >= base && p.ack < nextSeqnum)
        {
            for (unsigned int i = base; i<= p.ack; i++)
            {
                if (timer[i % n].flag && i % 5 == 0)
                    rto.addSampleRTT(timer[i % n].getDiff());
                timer[i % n].stopTimer();
                delete []buf[i % n];            //删除被确认的数据
            }
            base = p.ack + 1;                   //base = ack + 1
        }
        else if(++dupAckCnt == 3)               //三次重复ack，快速重传
        {
            packet p;
            makePkt(&p, buf[base % n], len[base % n], base);
            sendto(s, (char*)&p, len[base % n] + sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
            dupAckCnt = 0;
        }
    }
}

//超时重传
void SBuf::retrans()
{
    for (unsigned int i = base; i < nextSeqnum; i++)        //定时器超时，重传[base, nextSeqnum)中的数据
    {
        if(timer[i % n].isStart && timer[i % n].testTimeOut())
        {
            timer[i % n].setTimeOut(timer[i % n].timeout * 1.1);
            timer[i % n].startTimer();
            timer[i % n].flag = false;
            packet p;
            makePkt(&p, buf[i % n], len[i % n], i);
            sendto(s, (char*)&p, len[i % n] + sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
        }
    }
}

bool SBuf::isterminated()
{
    bool t;
    t = istermi;
    return t;
}

bool SBuf::close()
{
    bool flag = true;
    if(base == nextWrite)
    {
        istermi = true;
        flag = false;
        packet p;
        makeFinPkt(&p, nextSeqnum);
        packet ackPkt;
        sockaddr from;
        int fromlen = sizeof(sockaddr);
        while (1)
        {
            sendto(s, (char*)&p, sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));                      //发送SYN报文段
            if (recvfrom(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &from, &fromlen) < 0) continue;
            if (check(&ackPkt) && isAck(ackPkt.flag) && ackPkt.ack == nextSeqnum) break;
        }
    }
    return flag;
}
