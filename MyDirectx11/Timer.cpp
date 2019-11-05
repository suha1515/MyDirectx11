#include "Timer.h"

using namespace std::chrono;

Timer::Timer()
{
	last = steady_clock::now();
}

//Mark 함수는 이함수를 호출하고 다음 호출할때까지 사이의 시간을 반환한다.
float Timer::Mark()
{
	const auto old = last;
	last = steady_clock::now();
	const duration<float> frameTime = last - old;
	return frameTime.count();
}

//Peek함수는 Mark 함수 호출이후 Mark 함수의 리셋없이 이후의 시간경과를 반환한다.
float Timer::Peek() const
{
	return duration<float>(steady_clock::now() - last).count();
}
