#ifndef RBUF_H
#define RBUF_H
#include<iostream>
#include<Windows.h>
#include"packet.h"
using namespace std;

//���ջ�����
class RBuf
{
public:
    RBuf(int, SOCKET, SOCKET, sockaddr);    //��ʼ�����ջ�����
    ~RBuf();
    void read(char*, int);                  //Ӧ�ò�����ܻ�����������
    bool recv();                            //rdt�������ݣ����뻺����
    void getSocket(SOCKET*);                //��������socket
private:
    char** buf;                             //������
    int n;                                  //��������С
    unsigned int front;                     //���������ף�ָ��Ӧ�ò���һ��Ҫ�������ݱ�
    unsigned int rear;                      //��������β��ָ������յ������һ�����ݱ���֮����ʧ�����ݱ�
    unsigned int expected;                  //���ջ����ڴ����յ���һ���ݱ������к�
    SOCKET cs;                              //����socket
    SOCKET s;                               //������socket
    sockaddr addr;                          //�Է��ĵ�ַ
    HANDLE mutex;                           //�����ź���
    HANDLE slots;                           //������ʣ��λ�ü����ź���
    HANDLE items;                           //������������յ������ݱ���Ŀ�����ź���
};

#endif
