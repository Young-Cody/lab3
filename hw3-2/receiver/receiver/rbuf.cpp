#include "rbuf.h"

/*
	n:���ջ�������С
	cs:����socket
	s:������socket
	addr:���ӶԷ���ַ
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

	mutex = CreateSemaphore(NULL, 1, 1, NULL);
	slots = CreateSemaphore(NULL, n, n, NULL);
	items = CreateSemaphore(NULL, 0, n, NULL);
}

RBuf::~RBuf()
{
	WaitForSingleObject(mutex, INFINITE);
	for (int i = 0; i < n; i++)
		if (buf[i]) delete []buf[i];
	delete[]buf;
	CloseHandle(mutex);
	CloseHandle(slots);
	CloseHandle(items);
}

/*
	buf:Ӧ�ò���յ����ݱ�
	len:���ݱ�����
*/
void RBuf::read(char* buf, int len)
{
	WaitForSingleObject(items, INFINITE);
	WaitForSingleObject(mutex, INFINITE);
	front++;
	for (int i = 0; i < len; i++)
		buf[i] = this->buf[front % n][i];
	delete []this->buf[front % n];
	this->buf[front % n] = NULL;
	ReleaseSemaphore(mutex, 1, NULL);
	ReleaseSemaphore(slots, 1, NULL);
}

bool RBuf::recv()
{
	bool flag = true;
	packet p;
	sockaddr from = addr;
	int fromlen = sizeof(sockaddr) ;
	if(recvfrom(s, (char*)&p, sizeof(packet), 0, &from, &fromlen) < 0) return true;
	WaitForSingleObject(mutex, INFINITE);
	if (check(&p) && p.seq >= expected && !buf[(rear + p.seq - expected + 1) % n] && !isSyn(p.flag))	//���ݱ��������ȷ
	{
		packet ackPkt;
		
		if (isFin(p.flag) && p.seq == expected)	//fin���ģ����ջ������е����ݱ��������ack���ر����ӡ�
		{
			if (front == rear)
			{
				flag = false;
				sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
			}
		}
		else
		{
			WaitForSingleObject(slots, INFINITE);
			int idx = rear + p.seq - expected + 1;	//���յ����ݱ��ڻ����е�λ��
			buf[idx % n] = new char[p.dataLen];
			for (int i = 0; i < p.dataLen; i++)		//�������ʧ������ݱ���������
				buf[idx % n][i] = p.data[i];
			while (buf[(rear + 1)% n])				//�õ���ǰ������յ����һ�����ݱ������
			{
				ReleaseSemaphore(items, 1, NULL);	//�����ź�������
				rear++;
				expected++;
			}
			makeAckPkt(&ackPkt, expected - 1);
			sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));	//�����ۼ�ȷ��ack
		}
	}
	else	//���ݱ�����ʹ�����ظ�����
	{
		packet ackPkt;
		makeAckPkt(&ackPkt, expected - 1);
		sendto(s, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &addr, sizeof(sockaddr));
	}
	ReleaseSemaphore(mutex, 1, NULL);
	return flag;
}

void RBuf::getSocket(SOCKET *s)
{
	*s = this->cs;
}
