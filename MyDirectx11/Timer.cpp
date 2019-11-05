#include "Timer.h"

using namespace std::chrono;

Timer::Timer()
{
	last = steady_clock::now();
}

//Mark �Լ��� ���Լ��� ȣ���ϰ� ���� ȣ���Ҷ����� ������ �ð��� ��ȯ�Ѵ�.
float Timer::Mark()
{
	const auto old = last;
	last = steady_clock::now();
	const duration<float> frameTime = last - old;
	return frameTime.count();
}

//Peek�Լ��� Mark �Լ� ȣ������ Mark �Լ��� ���¾��� ������ �ð������ ��ȯ�Ѵ�.
float Timer::Peek() const
{
	return duration<float>(steady_clock::now() - last).count();
}
