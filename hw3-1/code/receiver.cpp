#include"rdt.h"
#include"Timer.h"
#include"receiver.h"
#include"ftp.h"

SOCKET sockRcvr;
sockaddr_in addrRcvr;

int initiate()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int err;
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0)
	{
	    cerr<<"fail to start up";
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		cerr<<"fail to start up with version 2.2";
		return -1;
	}
	sockRcvr = socket(AF_INET, SOCK_DGRAM, 0);

	cout<<"请输入接收端的IP地址:";
	string IP;
	cin>>IP;
	unsigned short port;
	cout<<"请输入接收端的端口号:";
	cin>>port;

	addrRcvr.sin_port = htons(port);
	addrRcvr.sin_family = AF_INET;
	addrRcvr.sin_addr.S_un.S_addr = inet_addr(IP.c_str());

    if(rdt::bind(sockRcvr, (sockaddr*)&addrRcvr) == -1) return -1;
	return 0;
}

int recvFile(const char *dir)
{
    char path[256];
    strcpy_s(path, dir);
    strcat_s(path, "\\");
    ftp *f = new ftp;
    if(rdt::recv_deliver(sockRcvr, (char*)f, sizeof(ftp)) == -1) return -1;
    strcat_s(path, f->name);

	cout<<"开始接收文件"<<f->name<<'\n';

    FILE* file;
	char buf[MAXBUFSIZE];
    fopen_s(&file, path, "wb");
    if(!file) return -1;
	long long num = f->size / MAXBUFSIZE;
    while(num--)
	{
		if(rdt::recv_deliver(sockRcvr, buf, MAXBUFSIZE) < 0) return -1;
		fwrite(buf, 1, MAXBUFSIZE, file);
	}
	int rem;
	if((rem = f->size % MAXBUFSIZE) > 0)
	{
		if(rdt::recv_deliver(sockRcvr, buf, rem) == -1) return -1;
		fwrite(buf, 1, rem, file);
	}
	cout<<"接收文件成功\n";
	fclose(file);
	return 0;
}

void recvFiles()
{
    string dir;
    cout<<"请输入文件保存的位置:";
    cin>>dir;
    while(recvFile(dir.c_str()) == 0);
}

int main()
{
    if(initiate() == -1)
        cerr<<"接收端初始化失败\n";
    if(rdt::accept(sockRcvr) == -1)
        cerr<<"接受连接失败\n";
    recvFiles();
}