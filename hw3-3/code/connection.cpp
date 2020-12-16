#include "connection.h"

/*
	backlog:队列中的最大连接数
*/
ConnectionBuf::ConnectionBuf(int backlog)
{
	this->backlog = backlog;
	front = rear = 0;
	s = new SOCKET[backlog];
	mutex = CreateSemaphore(NULL, 1, 1, NULL);
	slots = CreateSemaphore(NULL, backlog, backlog, NULL);
	items = CreateSemaphore(NULL, 0, backlog, NULL);
}

ConnectionBuf::~ConnectionBuf()
{
	delete[]s;
	CloseHandle(mutex);
	CloseHandle(items);
	CloseHandle(slots);
}

/*
	s:插入的连接socket
*/
void ConnectionBuf::insert(SOCKET s)
{
	WaitForSingleObject(slots, INFINITE);
	WaitForSingleObject(mutex, INFINITE);
	this->s[++rear % backlog] = s;
	ReleaseSemaphore(mutex, 1, NULL);
	ReleaseSemaphore(items, 1, NULL);
}

/*
	s:应用层接收的连接socket
*/
void ConnectionBuf::accept(SOCKET *s)
{
	WaitForSingleObject(items, INFINITE);
	WaitForSingleObject(mutex, INFINITE);
	*s = this->s[++front % backlog];
	ReleaseSemaphore(mutex, 1, NULL);
	ReleaseSemaphore(slots, 1, NULL);
}
