#pragma once
#include<Windows.h>

//���ն˽��յ������������
class ConnectionBuf
{
public:
	ConnectionBuf(int);		//��ʼ�������������
	~ConnectionBuf();		//�����������
	void insert(SOCKET);	//��������һ������
	void accept(SOCKET*);	//Ӧ�ò����һ������
private:
	int backlog;		//���Խ��ܵ����������
	int front;			//����
	int rear;			//��β
	SOCKET *s;			//���ӻ���
	HANDLE mutex;		//�����ź���
	HANDLE slots;		//����ʣ�������������ź���
	HANDLE items;		//���������������ź���
};
