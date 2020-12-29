#include"sender.h"
#include"rdt.h"

SOCKET sockSender;
SOCKADDR_IN addrSender;
double trans,tt;
double avg[4];
double a[4];

//发送端初始化
int initiate()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int err;
	memset(avg,0,sizeof(avg));
	memset(a,0,sizeof(a));
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
	string IP;
	unsigned short port;
	//cout<<"请输入发送端IP地址:";
	//string IP;
	//cin>>IP;
	//unsigned short port;
	//cout<<"请输入发送端端口号:";
	//cin>>port;
	IP = "127.0.0.1";
	port = 3000;
	addrSender.sin_addr.S_un.S_addr = inet_addr(IP.c_str());
	addrSender.sin_family = AF_INET;
	addrSender.sin_port = htons(port);
	sockSender = socket(AF_INET, SOCK_DGRAM, 0);
	if(rdt::bind(sockSender, (sockaddr*)&addrSender) == -1) return -1;

	//cout<<"请输入接收端的IP地址:";
	//cin>>IP;
	//cout<<"请输入接收端的端口号:";
	//cin>>port;
	IP = "127.0.0.1";
	port = 2000;
	sockaddr_in addrRcvr;
	addrRcvr.sin_port = htons(port);
	addrRcvr.sin_family = AF_INET;
	addrRcvr.sin_addr.S_un.S_addr = inet_addr(IP.c_str());

	cout << "请输入发送窗口大小:";
	int N;
	cin >> N;
	if(rdt::connect(sockSender, (const sockaddr*)&addrRcvr, N) == -1){
		cerr<<"建立连接失败\n";
		return -1;
	}
	return 0;
}

//打开文件
FILE* openFile(const char *path, ftp *f)
{
	char ext[16];
	_splitpath_s(path, NULL, 0, NULL, 0, f->name, 48, ext, 16);
	strcat_s(f->name, ext);
	FILE *file;
	fopen_s(&file, path, "rb");
	if(file)
	{
		fseek(file, 0, SEEK_END);
		f->size = ftell(file);
	}
	return file;
}

//发送单个文件
void sendFile(const char* filename)
{
	ftp f;
	memset(&f, 0, sizeof(ftp));
	FILE* file = openFile(filename, &f);
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

	fseek(file,0,SEEK_SET);
	long long num = f.size / MAXBUFSIZE;
	Timer t;
	double time = 0;
	while (num--)
	{
		fread(buf, 1, MAXBUFSIZE, file);
		t.startTimer();
		if (rdt::send(sockSender, buf, MAXBUFSIZE) < 0)
		{
			cerr << "传输文件失败\n";
			fclose(file);
			return;
		}
		time += t.getDiff();
		t.startTimer();
	}
	int rem;
	if ((rem = f.size % MAXBUFSIZE) > 0)
	{
		fread(buf, 1, rem, file);
		t.startTimer();
		if (rdt::send(sockSender, buf, rem) == -1)
		{
			cerr << "传输文件失败\n";
			fclose(file);
			return;
		}
		time += t.getDiff();
	}
	cout<<"传输文件成功\n";
	trans = time;
	tt = f.size * 8 / time;
	fclose(file);
}

//发送多个文件
void sendFiles()
{
	int n;
	//cout<<"请输入要传输的文件数:";
	//cin>>n;
	n = 1;
	rdt::send(sockSender, (char*)&n, 1);
	const char* test[5] = {
		"..\\test\\a.txt",
		"..\\test\\2.jpg",
		"..\\test\\1.jpg",
		"..\\test\\helloworld.txt",
		"..\\test\\3.jpg"
	};
	for (int i = 0; i < n; i++)
	{
		//cout << "请输入要发送的文件:";
		//string filename;
		//cin >> filename;
		//sendFile(filename.c_str());
		sendFile(test[i]);
		cout << "传输时间:" << trans << "ms  平均吞吐率:" << tt << "kbps\n";
		avg[i] += tt;
		a[i] += trans;
	}	
}

int main()
{
	if(initiate() == -1)
	{
		cerr<<"发送端初始化失败\n";
		exit(1);
	}
		sendFiles();
	// double sumt=0;
	// double suma=0;
	// for(int i = 0; i < 4; i++)
	// {
	// 	sumt+=a[i];
	// 	suma+=avg[i];
	// }
	// cout<<"平均传输时间"<<sumt<<"ms\n";
	// cout<<"平均吞吐率"<<suma / 4<<"kbps\n";
	Sleep(INFINITE);
}