#include"sender.h"
#include"rdt.h"
#include"Timer.h"

SOCKET sockSender;
SOCKADDR_IN addrSender;

//发送端初始化
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

	cout<<"请输入发送端IP地址:";
	string IP;
	cin>>IP;
	unsigned short port;
	cout<<"请输入发送端端口号:";
	cin>>port;
	addrSender.sin_addr.S_un.S_addr = inet_addr(IP.c_str());;
	addrSender.sin_family = AF_INET;
	addrSender.sin_port = htons(port);
	sockSender = socket(AF_INET, SOCK_DGRAM, 0);
	if(rdt::bind(sockSender, (sockaddr*)&addrSender) == -1) return -1;

	cout<<"请输入接收端的IP地址:";
	cin>>IP;
	cout<<"请输入接收端的端口号:";
	cin>>port;
	sockaddr_in addrRcvr;
	addrRcvr.sin_port = htons(port);
	addrRcvr.sin_family = AF_INET;
	addrRcvr.sin_addr.S_un.S_addr = inet_addr(IP.c_str());

	if(rdt::connect(sockSender, (const sockaddr*)&addrRcvr) == -1){
		cerr<<"建立连接失败\n";
		return -1;
	}
	return 0;
}

//打开文件
FILE* openFile(const char *path, ftp *f)
{
	char ext[16];
	_splitpath(path, NULL, NULL, f->name, ext);
	strcat(f->name, ext);
	FILE *file;
	file = fopen(path, "rb");
	if(file)
	{
		fseek(file, 0, SEEK_END);
		f->size = ftell(file);
	}
	return file;
}

//发送单个文件
void sendFile()
{
	cout<<"请输入要发送的文件";
	string filename;
	cin >> filename;
	ftp f;
	memset(&f, 0, sizeof(ftp));
	FILE* file = openFile(filename.c_str(), &f);
	if(!file) 
	{
		cerr<<"打开文件失败\n";
		return;
	}
	if (rdt::send(sockSender, (const char*)&f, sizeof(ftp)) < 0)
	{
		cerr << "传输文件失败\n";
		return;
	}
	cout<<"正在传输文件"<<filename<<"······\n";
	char buf[MAXBUFSIZE];
	Timer timer;
	timer.startTimer();
	fseek(file,0,SEEK_SET);
	long long num = f.size / MAXBUFSIZE;
	while (num--)
	{
		fread(buf, 1, MAXBUFSIZE, file);
		if (rdt::send(sockSender, buf, MAXBUFSIZE) < 0)
		{
			cerr << "传输文件失败\n";
			fclose(file);
			return;
		}
	}
	int rem;
	if ((rem = f.size % MAXBUFSIZE) > 0)
	{
		fread(buf, 1, rem, file);
		if (rdt::send(sockSender, buf, rem) == -1)
		{
			cerr << "传输文件失败\n";
			fclose(file);
			return;
		}
	}
	double t = timer.getDiff();
	cout<<"传输文件成功\n";
	cout << "传输时间:" << t << "ms  平均吞吐率:" << f.size * 1000 * 8 / (1024 * t) << "kbps\n";
	fclose(file);
}

//发送多个文件
void sendFiles()
{
	int n;
	cout<<"请输入要传输的文件数:";
	cin>>n;
	while (n--)
		sendFile();
}

int main()
{
	if(initiate() == -1)
	{
		cerr<<"发送端初始化失败\n";
		exit(1);
	}
	sendFiles();
	rdt::close(sockSender);
}
