#include"Timer.h"

//��ʱ����ʼ��
Timer::Timer()
{
    timeout = 0;
    QueryPerformanceFrequency(&f);
    isStart = false;
}

//��ʱ����ʼ��ʱ
void Timer::startTimer()
{
    isStart = true;
    setTimer();
}

//�õ���ǰclick��
void Timer::setTimer()
{
    QueryPerformanceCounter(&timer_start);
}

//��ʱ��ֹͣ��ʱ
void Timer::stopTimer()
{
    isStart = false;
}

//�õ�ʱ���
double Timer::getDiff()
{
    QueryPerformanceCounter(&timer_end);
    return ((timer_end.QuadPart - timer_start.QuadPart) * 1000.0 / f.QuadPart);
}

//����Ƿ�ʱ
bool Timer::testTimeOut()
{
    return getDiff() > timeout;
}

//���ó�ʱʱ��
void Timer::setTimeOut(double time)
{
    timeout = time;
}