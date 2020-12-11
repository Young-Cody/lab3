#ifndef SENDER_H
#define SENDER_H

#include"ftp.h"
#include<iostream>
using namespace std;

int initiate();							//发送端初始化
void sendFile();						//发送单个文件
void sendFiles();						//发送多个文件
FILE* openFile(const char *, ftp *);	//打开文件

#endif
