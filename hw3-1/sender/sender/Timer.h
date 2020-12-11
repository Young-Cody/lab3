#ifndef TIMER_H
#define TIMER_H
#include<Windows.h>
#include<iostream>
using namespace std;

//��ʱ��
struct Timer{
    Timer();                    //��ʱ����ʼ��
    void startTimer();          //��ʱ����ʼ��ʱ
    void setTimer();            //�õ���ǰclick��
    void stopTimer();           //��ʱ��ֹͣ��ʱ
    double getDiff();           //�õ�ʱ���
    bool testTimeOut();         //����Ƿ�ʱ
    void setTimeOut(double);    //���ó�ʱʱ��
    bool isStart;               //��ʱ���Ƿ�ʼ
    double timeout;             //��ʱʱ�䣬��λms
    LARGE_INTEGER timer_start, timer_end, f;
};

#endif // !TIMER_H