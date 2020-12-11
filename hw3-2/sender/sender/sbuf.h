#ifndef SBUF_H
#define SBUF_H
#include<iostream>
#include<WinSock2.h>
#include<Ws2tcpip.h>
#include<Windows.h>
#include"Timer.h"
#include"packet.h"
using namespace std;

//���ͻ�����
class SBuf
{
public:
    SBuf(int, SOCKET, const sockaddr*);     //��ʼ�����ͻ�����
    ~SBuf();                                //�������ͻ�����
    void write(const char*,int);            //Ӧ�ò����ͻ�����д���ݱ�
    void send();                            //����㷢�����ݱ�
    void ack();                             //����ack���ݱ�
    void retrans();                         //��ʱ�ش�
    bool isterminated();                    //�����Ƿ�ر�
    bool close();                           //�ر�����
private:
    char** buf;                             //���ͻ�����
    int* len;                               //�����ݱ��ĳ���
    int n;                                  //���ͻ������Ĵ�С
    int N;                                  //�������ڵĴ�С
    bool istermi;                           //�����Ƿ�ر�
    unsigned int base;                      //[base, nextSeqnum) �ѷ���δȷ�ϵ����ݱ�,����С�ڻ������ڵĴ�СN
    unsigned int nextSeqnum;                //[nextSeqnum, nextWrite) Ӧ�ò�д�뻺����δ���͵����ݱ�
    unsigned int nextWrite;                 
    bool flag;                                      
    SOCKET s;                               //��ʶ���ӵ�socket
    sockaddr addr;                          //���ӶԷ��ĵ�ַ
    Timer *timer;                           //��ʱ��
    RTO rto;                                //������һ��RTO
    HANDLE mutex;                           //�����ź���
    HANDLE slots;                           //���ͻ�����ʣ���С�����ź���
    HANDLE sem_ack;                         //��ȷ���ı����������ź���
};


#endif

