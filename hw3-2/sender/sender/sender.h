#ifndef SENDER_H
#define SENDER_H

#include"ftp.h"
#include<iostream>
using namespace std;

int initiate();							//���Ͷ˳�ʼ��
void sendFile(const char*);				//���͵����ļ�
void sendFiles();						//���Ͷ���ļ�
FILE* openFile(const char *, ftp *);	//���ļ�

#endif