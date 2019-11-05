#pragma once

#include<chrono>
/* 기존의 GetTickCount의 최소 정밀도는 10~16ms 정도
 조금더 정밀도가 높은 timeGetTime의 경우 멀티미디어 타이머에 영향을 받으며 5ms 정도 정밀도를 가진다
 높은 정밀도가 필요하다면 기존에 QueryPerformanceQuery/QueryPerformanceCounter 였는데
 이 함수의 기능은 OS가 제공하는 기능으로 C++의 표준 메서드가 아니다 즉 OS가 바뀌면 해당 기능은 사용 할 수 없다는뜻

 이런점에서 c++11에서 채택된 chrono를 사용하면된다 정밀도는 nanosec 까지 지원하며 시간단위끼리의 연산도 가능하다
 */
class Timer
{
public:
	Timer();
	float Mark();
	float Peek() const;
private:
	std::chrono::steady_clock::time_point last;
	//Steady_Clock은 증가 또는 감소만하는 즉 시간역행을하지않는 개념의 clock 클래스이다.
	//os 시간을 돌려도 역행하지 않는다는뜻
};

