#pragma once

#include<chrono>
/* ������ GetTickCount�� �ּ� ���е��� 10~16ms ����
 ���ݴ� ���е��� ���� timeGetTime�� ��� ��Ƽ�̵�� Ÿ�̸ӿ� ������ ������ 5ms ���� ���е��� ������
 ���� ���е��� �ʿ��ϴٸ� ������ QueryPerformanceQuery/QueryPerformanceCounter ���µ�
 �� �Լ��� ����� OS�� �����ϴ� ������� C++�� ǥ�� �޼��尡 �ƴϴ� �� OS�� �ٲ�� �ش� ����� ��� �� �� ���ٴ¶�

 �̷������� c++11���� ä�õ� chrono�� ����ϸ�ȴ� ���е��� nanosec ���� �����ϸ� �ð����������� ���굵 �����ϴ�
 */
class Timer
{
public:
	Timer();
	float Mark();
	float Peek() const;
private:
	std::chrono::steady_clock::time_point last;
	//Steady_Clock�� ���� �Ǵ� ���Ҹ��ϴ� �� �ð������������ʴ� ������ clock Ŭ�����̴�.
	//os �ð��� ������ �������� �ʴ´ٴ¶�
};

