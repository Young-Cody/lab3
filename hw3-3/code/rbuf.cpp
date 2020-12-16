#include "rbuf.h"

/*
	n:接收缓冲区大小
	cs:连接socket
	s:服务器socket
	addr:连接对方地址
*/
RBuf::RBuf(int n, SOCKET cs, SOCKET s,sockaddr addr)
{
	this->n = n;
	this->s = s;
	this->cs = cs;
	this->addr = addr;
	buf = new char* [n];

	memset(buf, 0, sizeof(char*) * n);
	front = rear = 1;
	expected = 1;
}

RBuf::~RBuf()
{
	for (int i = 0; i < n; i++)
		if (buf[i]) delete []buf[i];
	delete[]buf;
}

/*
	buf:应用层接收的数据报
	len:数据报长度
*/
void RBuf::read(char* buf, int len)
{
	while(front >= rear);
	front++;
	for (int i = 0; i < len; i++)
		buf[i] = this->buf[front % n][i];
	delete []this->buf[front % n];
	this->buf[front % n] = NULL;
}

bool RBuf::recv()
{
	if((rear - front) >= n) return true;
	bool flag = true;
	packet p;
	sockaddr from = addr;
	int fromlen = sizeof(sockaddr) ;
	if(recvfrom(s, (char*)&p, sizeof(packet), 0, &from, &fromlen) < 0) return true;
	if (check(&p) && p.seq >= expected && !buf[(rear + p.seq - expected + 1) % n] && !isSyn(p.flag))	//数据报检验和正确
	{
		packet ackPkt;
		
		if (isFin(p.flag) && p.seq == expected)	//fin报文，接收缓冲区中的数据被读完后发送ack，关闭连接。
		{
			if (front == rear)
			{
				flag = false;
				sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
			}
		}
		else
		{
			int idx = rear + p.seq - expected + 1;	//接收的数据报在缓存中的位置
			buf[idx % n] = new char[p.dataLen];
			for (int i = 0; i < p.dataLen; i++)		//将按序或失序的数据报缓存起来
				buf[idx % n][i] = p.data[i];
			while (buf[(rear + 1) % n])				//得到当前按序接收的最后一个数据报的序号
			{
				rear++;
				expected++;
			}
			makeAckPkt(&ackPkt, expected - 1);
			sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));	//发送累计确认ack
		}
	}
	else	//数据报检验和错误或重复接收
	{
		packet ackPkt;
		makeAckPkt(&ackPkt, expected - 1);
		sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
	}
	return flag;
}

void RBuf::getSocket(SOCKET *s)
{
	*s = this->cs;
}
