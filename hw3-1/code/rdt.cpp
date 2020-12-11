#include"rdt.h"
#include"Timer.h"
#include<Windows.h>
#pragma comment(lib, "ws2_32")
map<SOCKET, sockaddr> connection;           //所有连接
map<SOCKET, sockaddr> sock2addr;            //socket映射到地址
map<SOCKET, unsigned char> curSeq;          //发送端当前序列号
map<SOCKET, unsigned char> expectedSeq;     //接收端当前期望的序列号
map<SOCKET, RTO> curRTO;                    //RTO估计

//将socket和地址绑定
int rdt::bind(SOCKET s, const sockaddr *addr)
{
	if(addr)
	{
		sock2addr[s] = *addr;
		return bind(s, addr, sizeof(sockaddr));
	}
	return -1;
}

//建立连接
int rdt::connect(SOCKET s, const sockaddr *name)
{
	if(sock2addr.find(s) == sock2addr.end() || !name) return -1; //连接建立失败

    int timeout = 100;
    //将recvfrom设置为非阻塞
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == -1) {
        cerr<<"setsockopt failed";
        return -1; //连接建立失败
    }

    curSeq[s] = 0;
    RTO r;
    r.EstimatedRTT = 1000;
    r.DevRTT = 0;
    r.rto = 1000;
    curRTO[s] = r;

	const sockaddr * saddr = &sock2addr[s];
	SOCKADDR_IN *saddr_in = (SOCKADDR_IN*)saddr;
	packet *p = new packet;
	memset(p, 0, sizeof(packet));
	p->saf = setSyn(p->saf);
	p->saf = setSeqNum(p->saf, curSeq[s]);
	p->dataLen = 0;
	p->checkSum = ~getCheckSum(p);
    int len = sizeof(packet) - MAXBUFSIZE;

	packet t;
	int fromlen = sizeof(sockaddr);
    sockaddr recvaddr = *name;

    Timer connection_timer;  //设置超时时间为75秒
    connection_timer.setTimeOut(connTimeOut);
    connection_timer.startTimer();
	while (1)
	{
        if(connection_timer.testTimeOut()) return -1;
        if(sendto(s, (char*)p, len, 0, name, sizeof(sockaddr)) == -1) continue;     //发送SYN报文 
		if(recvfrom(s, (char*)&t, len, 0, &recvaddr, &fromlen) < 0) continue;       //收到报文
		if(check(&t) && isAck(t.saf) && (getAckNum(t.saf) == curSeq[s]))            //收到的数据报检验和正确并且按序，连接建立成功
		{
			connection[s] = *name;
            curSeq[s] = !curSeq[s];
			return 0;
		}
	}
    return -1;  //连接建立失败
}

//应用层发送数据
int rdt::send(SOCKET s, const char *buf, int len)
{
    if(connection.find(s) == connection.end() || sock2addr.find(s) == sock2addr.end()) 
        return -1;  //连接不存在
    packet pkt;
    makePkt(s, &pkt, buf, len);

    packet ackPkt;
    int fromlen = sizeof(sockaddr);
    sockaddr recvaddr = connection[s];
    int flag = true;    //记录是否进行了重传，如没有进行重传，则计算RTO

    Timer timer;
    timer.setTimeOut(curRTO[s].rto);
    timer.startTimer();
    //将应用层数据封装到报文中发送
    if(sendto(s, (const char *)&pkt, sizeof(pkt) - MAXBUFSIZE + len, 0, &connection[s],sizeof(sockaddr)) == -1) return -1;

    while(1)
    {
        if(timer.testTimeOut())     //超时重传
        {
            flag = false;
            if (sendto(s, (const char*)&pkt, sizeof(pkt) - MAXBUFSIZE + len, 0, &connection[s], sizeof(sockaddr)) == -1) return -1;
            timer.setTimeOut(timer.timeout * 2);
            timer.startTimer();
        }

        if(recvfrom(s, (char*)&ackPkt, sizeof(pkt) - MAXBUFSIZE, 0, &recvaddr, &fromlen) <= 0) continue;
        if(check(&ackPkt) && isAck(ackPkt.saf) && (getAckNum(ackPkt.saf) == curSeq[s]))     //ack报文段检验和正确，并且按序
        {
            curSeq[s] = !curSeq[s];
            timer.stopTimer();
            if(flag) curRTO[s].addSampleRTT(timer.getDiff());   //计算下一次RTO
            return 0;
        }
    }
}

//应用层接收连接
int rdt::accept(SOCKET s)
{
    if(sock2addr.find(s) == sock2addr.end()) return -1;
    packet pkt;
    sockaddr addr;
    int addrlen = sizeof(sockaddr);
    while (1)
    {
        if(recvfrom(s, (char*)&pkt, sizeof(packet) - MAXBUFSIZE, 0, &addr, &addrlen) > 0)
        {
            if(check(&pkt) && isSyn(pkt.saf))   //检验和正确并且是SYN报文段
            {   
                int seq = getSeqNum(pkt.saf);
                packet ackPkt;
                connection[s] = addr;   //建立连接
                expectedSeq[s] = !seq;  //期待序列号取反
                makeAckPkt(s, &ackPkt, seq);
                sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr_in));   //发送对syn报文段的ack
                return 0;
            }
        }
    }
}

//关闭连接
int rdt::close(SOCKET s)
{
    if(connection.find(s) == connection.end() || sock2addr.find(s) == sock2addr.end()) return -1;
    packet p;
	memset(&p, 0, sizeof(packet));
	p.saf = setFin(p.saf);
	p.saf = setSeqNum(p.saf, curSeq[s]);
	p.dataLen = 0;
	p.checkSum = ~getCheckSum(&p);
	int len = sizeof(packet) - MAXBUFSIZE;
    packet t;
	int fromlen = sizeof(sockaddr);
    sockaddr recvaddr = connection[s];

	while (1)
	{
        if(sendto(s, (char*)&p, len, 0, &recvaddr, sizeof(sockaddr)) == -1) continue;   //发送FIN报文端
		if(recvfrom(s, (char*)&t, len, 0, &recvaddr, &fromlen) < 0) continue;
		if(check(&t) && isAck(t.saf) && getAckNum(t.saf) == curSeq[s]) //数据报检验和正确并且按序，连接关闭成功
		{
			connection.erase(s);
			return 0;
		}
	}
    return -1;  //连接关闭失败
}

//接受数据并返回给应用层
int rdt::recv_deliver(SOCKET s, char *buf, int len)
{
    if(connection.find(s) == connection.end() || sock2addr.find(s) == sock2addr.end()) return -1;
    packet pkt;
    int datalen;
    int fromlen = sizeof(sockaddr);
    packet ackPkt;
    while (1)
    {
        if(recvfrom(s, (char*)&pkt, sizeof(packet) - MAXBUFSIZE + len, 0, &connection[s], &fromlen) <= 0) continue;
        if(check(&pkt) && (getSeqNum(pkt.saf) == expectedSeq[s])) //数据没有差错并且没有重复接收，发送ack，向应用层传递数据
        {
            datalen = pkt.dataLen;
            makeAckPkt(s, &ackPkt,getSeqNum(pkt.saf));
            sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &connection[s],sizeof(sockaddr));
            if(isFin(pkt.saf))
            {
                sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &connection[s], sizeof(sockaddr));
                connection.erase(s);
                expectedSeq.erase(s);
                return -1;
            }
            expectedSeq[s] = !expectedSeq[s];
            for (int i = 0; i < len; i++)
                buf[i] = pkt.data[i];
            return len;
        }
        else    //数据发生错误或重复接收，发送ack
        {
            makeAckPkt(s, &ackPkt,!expectedSeq[s]);
            sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &connection[s],sizeof(sockaddr));
        }
    }
}

//封装数据包
void rdt::makePkt(SOCKET s, packet *p, const char *buf, int len)
{
    memset(p, 0, sizeof(packet));
    p->saf = setSeqNum(p->saf, curSeq[s]);
    p->dataLen = len;
    for(int i = 0; i < len; i++)
        p->data[i] = buf[i];
    p->checkSum = ~getCheckSum(p);
}

//封装ack数据包
void rdt::makeAckPkt(SOCKET s, packet *ackPkt, unsigned char ackNum)
{
    memset(ackPkt, 0, sizeof(packet));
    ackPkt->saf = setAck(ackPkt->saf);
    ackPkt->saf = setAckNum(ackPkt->saf, ackNum);
    ackPkt->checkSum = ~getCheckSum(ackPkt);
}

//增加一次RTT，估算下一次RTO
void RTO::addSampleRTT(double sampleRTT)
{
    EstimatedRTT = 0.875 * EstimatedRTT + 0.125 * sampleRTT;
    DevRTT = 0.75 * DevRTT + 0.25 * abs(sampleRTT - EstimatedRTT);
    rto = EstimatedRTT + 4 * DevRTT;
}

//计算检验和
unsigned short getCheckSum(packet *p)
{
    unsigned int ans = 0;
    unsigned short res;
    ans += (unsigned short)p->saf << 8;
    ans += p->checkSum;
    ans += p->dataLen;
    for(int i = 0; i < p->dataLen; i++)
        if(i%2) ans += p->data[i];
        else ans += (unsigned short)p->data[i] << 8;
    ans = (ans >> 16) + (ans & 0xffff);
    res = (ans >> 16) + (ans & 0xffff);
    return res;
}
