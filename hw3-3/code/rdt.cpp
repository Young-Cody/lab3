#include"rdt.h"

map<SOCKET, sockaddr> sock2addr;        //socket所绑定的地址
map<SOCKET, SBuf*> sendBuf;             //所有连接的发送缓冲区
map<SOCKET, RBuf*> recvBuf;             //所有连接的接收缓冲区
map<SOCKET, ConnectionBuf*> consocks;   //接受的连接

/*
    s:要进行地址绑定的socket
    addr:绑定的地址

    return:绑定是否成功
*/
int rdt::bind(SOCKET s, const sockaddr *addr)
{
    sock2addr[s] = *addr;
    return bind(s, addr, sizeof(sockaddr));
}

/*
    s:发送缓冲区
*/
DWORD WINAPI sendThread(LPVOID s)
{
    SBuf* sb = (SBuf*)s;
    while (!sb->isterminated())
    {
        sb->send();
        sb->ack();
        sb->retrans();
    }
    return 0;
}


/*
    建立连接
    s:发送连接请求的socket
    name:对方的地址
    windowSize:滑动窗口大小
*/
int rdt::connect(SOCKET s, const sockaddr *name, int windowSize)
{
	if(!name || sock2addr.find(s) == sock2addr.end()) return -1; //连接建立失败

    int timeout = 10;
    //将recvfrom设置为非阻塞
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == -1) {
        cerr<<"setsockopt failed";
        return -1; //连接建立失败
    }

    packet p;
    makeSynPkt(&p, windowSize);
    int len = sizeof(packet) - MAXBUFSIZE;

	packet t;

	int fromlen = sizeof(sockaddr);
    sockaddr recvaddr = *name;

    Timer connection_timer;  
    connection_timer.setTimeOut(75000);     //设置连接建立超时时间为75秒
    connection_timer.startTimer();
	while (1)
	{
        if(connection_timer.testTimeOut()) return -1;   //连接建立超时
        if(sendto(s, (char*)&p, len, 0, name, sizeof(sockaddr)) < 0) continue;
		if(recvfrom(s, (char*)&t, len, 0, &recvaddr, &fromlen) < 0) continue;
		if(check(&t) && isAck(t.flag) && t.ack == 0) //连接建立成功
		{
            SBuf* sb = new SBuf(windowSize, s, &recvaddr);              //分配发送缓冲区
            sendBuf[s] = sb;
            CreateThread(NULL, 0, sendThread, (LPVOID)sb, 0, NULL);     //创建发送缓冲区工作线程
            return 0;
		}
	}
    return -1;  //连接建立失败
}

/*
    应用层发送数据,将数据写入发送缓冲
    s:标识一条连接的socket
    buf:要发送的应用层数据
    len:发送数据的长度
*/
int rdt::send(SOCKET s, const char *buf, int len)
{
    if(sock2addr.find(s) == sock2addr.end() || sendBuf.find(s) == sendBuf.end())
        return -1;  //连接不存在
    sendBuf[s]->write(buf, len);    //将应用层数据写入发送缓冲区
    return 0;
}

//监听连接请求
DWORD WINAPI listenThread(LPVOID s)
{
    packet p;
    int len = sizeof(sockaddr);
    sockaddr from;
    SOCKET sock = (SOCKET)s;
    while (1)
    {
        if (recvfrom(sock, (char*)&p, sizeof(packet) - MAXBUFSIZE, 0, &from, &len) == -1) continue;
        if (check(&p) && isSyn(p.flag))     //收到SYN数据报
        {   
            packet ackPkt;
            makeAckPkt(&ackPkt, 0);
            sendto(sock, (char*)&ackPkt, sizeof(packet) - MAXBUFSIZE, 0, &from, sizeof(sockaddr));

            SOCKET consock = socket(AF_INET, SOCK_DGRAM, 0);
            RBuf* rb = new RBuf(p.N * 2, consock, sock, from);
            recvBuf[consock] = rb;

            consocks[sock]->insert(consock);
        }
    }
}

/*
    开始监听
    s:接收连接的socket
    backlog:可以接受的最大连接数
*/
int rdt::listen(SOCKET s, int backlog)
{
    if (sock2addr.find(s) == sock2addr.end()) return -1;    //未绑定socket到本地地址
    int timeout = 10;

    //将recvfrom设置为非阻塞
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1) {
        cerr << "setsockopt failed";
        return -1;
    }
    ConnectionBuf* cs = new ConnectionBuf(backlog);
    consocks[s] = cs;
    CreateThread(NULL, 0, listenThread, (LPVOID)s, 0, NULL);
    return 0;
}

//接收数据，将数据放入接收缓冲区
DWORD WINAPI recvThread(LPVOID p)
{
    RBuf* rb = (RBuf*)p;
    while (rb->recv());
    SOCKET s = 0;
    rb->getSocket(&s);
    delete rb;
    recvBuf.erase(s);
    return 0;
}

//应用层接收连接
SOCKET rdt::accept(SOCKET s)
{
    if(sock2addr.find(s) == sock2addr.end() ||consocks.find(s) == consocks.end()) return -1;   //未开始监听或未绑定socket到本地地址
    SOCKET consock = 0;
    consocks[s]->accept(&consock);  //从连接缓冲区中得到一条连接
    CreateThread(NULL, 0, recvThread, (LPVOID)recvBuf[consock], 0, NULL);   //创建接收缓冲区工作线程
    return consock;
}

//关闭连接
int rdt::close(SOCKET s)
{
    if(sock2addr.find(s) == sock2addr.end() || sendBuf.find(s) == sendBuf.end()) return -1;
    while(sendBuf[s]->close());
    Sleep(100);
    delete sendBuf[s];
    sendBuf.erase(s);
    closesocket(s);
    return 0;
}

//接受数据并返回给应用层
int rdt::recv(SOCKET s, char *buf, int len)
{
    if(recvBuf.find(s) == recvBuf.end()) return -1;
    RBuf *rb = recvBuf.find(s)->second;
    rb->read(buf, len);
    return 0;
}
