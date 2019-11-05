#pragma once
#include <queue>
//마우스 기능
/*
	마우스도 키보드 이벤트와 다르지않다 마우스 클래스안에 Event 클래스를 가지고 있으며
	해당 이벤트는 마우스 이벤트의 종류를 가지고있다.

	Event 클래스는 마우스의 x,y좌표 및 오른쪽,왼쪽 마우스 버튼이 눌렸는지에 대한 정보를 가지고있으며

	마우스 클래스는 메시지 발생시 해당 정보를 buffer 큐에 담고 메시지 처리이후 
	버퍼안의 내용을 읽고 처리한다.
*/
class Mouse
{
	friend class Window;
public:
	class Event
	{
	public:
		enum class Type
		{
			LPress,
			LRelease,
			RPress,
			RRelease,
			WheelUp,
			WheelDown,
			Move,
			Enter,
			Leave,
			Invalid
		};
	private:
		Type type;
		bool leftIsPressed;
		bool rightIsPressed;
		int x;
		int y;
		int delta;
	public:
		Event() noexcept : type ( Type::Invalid),leftIsPressed(false),rightIsPressed(false),x(0),y(0),delta(0){}
		Event(Type type, const Mouse& parent) noexcept : type(type),leftIsPressed(parent.leftIsPressed),rightIsPressed(parent.rightIsPressed),
			x(parent.x),y(parent.y),delta(parent.wheelDeltaCarry){}
		bool IsValid() const noexcept
		{
			return type != Type::Invalid;
		}
		Type GetType() const noexcept
		{
			return type;
		}
		std::pair<int, int> GetPos() const noexcept
		{
			return { x,y };
		}
		int GetPosX() const noexcept
		{
			return x;
		}
		int GetPosY() const noexcept
		{
			return y;
		}
		int GetDelta() const noexcept
		{
			return delta;
		}
		bool LeftIsPressed() const noexcept
		{
			return leftIsPressed;
		}
		bool RightIsPressed() const noexcept
		{
			return rightIsPressed;
		}
	};
public:
	Mouse() = default;
	Mouse(const Mouse&) = delete;
	Mouse& operator=(const Mouse&) = delete;
	std::pair<int, int> GetPos() const noexcept;
	int GetPosX() const noexcept;
	int GetPosY() const noexcept;
	bool IsInWindow() const noexcept;
	bool LeftIsPressed() const noexcept;
	bool RightIsPressed() const noexcept;
	Mouse::Event Read() noexcept;
	bool IsEmpty() const noexcept
	{
		return buffer.empty();
	}
	void Flush() noexcept;
private:
	void OnMouseMove(int x, int y) noexcept;
	void OnMouseLeave() noexcept;
	void OnMouseEnfer() noexcept;
	void OnLeftPressed(int x, int y) noexcept;
	void OnLeftReleased(int x, int y) noexcept;
	void OnRightPressed(int x, int y) noexcept;
	void OnRightReleased(int x, int y) noexcept;
	void OnWheelUp(int x, int y) noexcept;
	void OnWheelDown(int x, int y) noexcept;
	void TrimBuffer() noexcept;
	void OnWheelDelta(int x, int y, int delta);
private:
	static constexpr unsigned int bufferSize = 16u;
	int x=0;
	int y=0;
	bool leftIsPressed = false;
	bool rightIsPressed = false;
	bool isInWindow = false;
	int wheelDeltaCarry = 0;
	std::queue<Event> buffer;

};

