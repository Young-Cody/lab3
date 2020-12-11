#ifndef RECV_H
#define RECV_H

int initiate();					//接收端初始化
int recvFile(const char *);		//接收单个文件
void recvFiles();				//接收多个文件
void destroy();					//接收端关闭

#endif