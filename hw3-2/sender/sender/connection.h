#pragma once
#include<Windows.h>

//接收端接收的连接请求队列
class ConnectionBuf
{
public:
	ConnectionBuf(int);		//初始化连接请求队列
	~ConnectionBuf();		//析构请求队列
	void insert(SOCKET);	//传输层插入一个连接
	void accept(SOCKET*);	//应用层接受一个连接
private:
	int backlog;		//可以接受的最大连接数
	int front;			//队首
	int rear;			//队尾
	SOCKET *s;			//连接缓存
	HANDLE mutex;		//互斥信号量
	HANDLE slots;		//缓存剩余连接数计数信号量
	HANDLE items;		//缓存连接数计数信号量
};
