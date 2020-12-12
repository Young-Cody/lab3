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
    flag = false;
    rto.EstimatedRTT = 10;
    rto.rto = 10;
    timer = new Timer();
    N = n;
    base = nextSeqnum = nextWrite = 1;
    mutex = CreateSemaphore(NULL, 1, 1, NULL);
    slots = CreateSemaphore(NULL, 2 * n, n * 2, NULL);
    sem_ack = CreateSemaphore(NULL, 0, n, NULL);
}

SBuf::~SBuf()
{
    WaitForSingleObject(mutex, INFINITE);
    delete[]len;
    delete[]buf;
    CloseHandle(mutex);
    CloseHandle(slots);
    CloseHandle(sem_ack);
}
/*
    将数据写入发送缓冲区
    buf:应用层数据
    len:数据长度
*/
void SBuf::write(const char* buf, int len)
{
    WaitForSingleObject(slots, INFINITE);
    WaitForSingleObject(mutex, INFINITE);
    this->buf[nextWrite % n] = new char[len];
    this->len[nextWrite % n] = len;
    for (int i = 0; i < len; i++)
        this->buf[nextWrite % n][i] = buf[i];
    nextWrite++;
    ReleaseSemaphore(mutex, 1, NULL);
}

//传输层将应用层数据组装成数据报并发送
void SBuf::send()
{
    WaitForSingleObject(mutex, INFINITE);
    if (nextSeqnum < base + N && nextSeqnum < nextWrite)    //滑动窗口中还有可以发送的数据
    {
        if (base == nextSeqnum)                             //开始发送时，开启定时器
        {
            timer->setTimeOut(rto.rto);
            timer->startTimer();
            flag = true;
        }
        packet p;
        makePkt(&p, buf[nextSeqnum % n], len[nextSeqnum % n], nextSeqnum);
        sendto(s, (char*)&p, len[nextSeqnum % n] + sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
        nextSeqnum++;
        ReleaseSemaphore(sem_ack, 1, NULL);
    }
    ReleaseSemaphore(mutex, 1, NULL);
}

//接受ack数据报
void SBuf::ack()
{
    sockaddr from = addr;
    int fromlen = sizeof(sockaddr);
    packet p;
    if(recvfrom(s, (char*)&p, sizeof(packet) - MAXBUFSIZE, 0, &from, &fromlen) < 0) return;
    WaitForSingleObject(mutex, INFINITE);
    if (check(&p) && isAck(p.flag) && p.ack >= base && p.ack < nextSeqnum)
    {
        if (flag)
        {
            rto.addSampleRTT(timer->getDiff());         //加入一次RTT，估算下一次RTO
            flag = false;
        }
        for (unsigned int i = base; i<= p.ack; i++)
        {
            delete []buf[i % n];                        //删除被确认的数据
            WaitForSingleObject(sem_ack, INFINITE);     //ack信号量递减
        }
        int num = p.ack - base + 1;
        base = p.ack + 1;                               //base = ack + 1
        if (base == nextSeqnum)                         //发送缓冲区为空，停止计时器
            timer->stopTimer();
        else
        {
            timer->setTimeOut(rto.rto);
            timer->startTimer();                        //发送缓冲区不为空，重启定时器
        }
        ReleaseSemaphore(slots, num, NULL);
    }
    ReleaseSemaphore(mutex, 1, NULL);
}

//超时重传
void SBuf::retrans()
{
    WaitForSingleObject(mutex, INFINITE);
    if (timer->isStart && timer->testTimeOut())
    {
        flag = false;
        /*timer->setTimeOut(timer->timeout * 2);*/
        timer->startTimer();
        //packet p;
        //makePkt(&p, buf[base % n], len[base % n], base);
        //sendto(s, (char*)&p, len[base % n] + sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
        for (unsigned int i = base; i < nextSeqnum; i++)        //定时器超时，重传[base, nextSeqnum)中的数据
        {
            packet p;
            makePkt(&p, buf[i % n], len[i % n], i);
            sendto(s, (char*)&p, len[i % n] + sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
        }
    }
    ReleaseSemaphore(mutex, 1, NULL);
}

bool SBuf::isterminated()
{
    bool t;
    WaitForSingleObject(mutex, INFINITE);
    t = istermi;
    ReleaseSemaphore(mutex, 1, NULL);
    return t;
}

bool SBuf::close()
{
    bool flag = true;
    WaitForSingleObject(mutex, INFINITE);
    if(base == nextWrite)
    {
        flag = false;
        packet p;
        makeFinPkt(&p, nextSeqnum);
        istermi = true;
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
    ReleaseSemaphore(mutex, 1, NULL);
    return flag;
}
