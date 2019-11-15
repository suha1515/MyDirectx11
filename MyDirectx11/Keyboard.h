#pragma once
#include <queue>
#include <bitset>
#include <optional>

class Keyboard
{
	friend class Window;
public:
	class Event				//키보드 키가 눌렀을때 발생하는 이벤트 클래스
	{
	public:
		enum class Type
		{
			Press,			//눌렀을때
			Release,		//누르지않을때
			Invalid			//유효하지않은 이벤트
		};
	private:
		Type type;
		unsigned char code;
	public:
		Event()noexcept : type(Type::Invalid), code(0u) {}
		Event(Type type, unsigned char code) noexcept : type(type), code(code) {}
		bool IsPress() const noexcept
		{
			return type == Type::Press;
		}
		bool IsRelease() const noexcept
		{
			return type == Type::Release;
		}
		bool IsVaild() const noexcept
		{
			return type != Type::Invalid;
		}
		unsigned char GetCode() const noexcept
		{
			return code;
		}
	};

public:
	Keyboard() = default;
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;
	~Keyboard();
	//key event stuff
	bool KeyIsPressed(unsigned char keycode) const noexcept;
	std::optional<Event> ReadKey() noexcept;
	bool KeyIsEmpty() const noexcept;
	void FlushKey() noexcept;
	//char event stuff
	char ReadChar() noexcept;
	bool CharIsEmpty() const noexcept;
	void FlushChar() noexcept;
	void Flush() noexcept;
	//autorepeat control
	void EnableAutorepeat() noexcept;
	void DisableAutorepeat() noexcept;
	bool AutorepeatIsEnabled() noexcept;
	//Window에 사용되는 함수들 클라이언트는 사용할수없다
	//this functions only can be used by Window not client
private:
	void OnKeyPressed(unsigned char keycode) noexcept;
	void OnKeyReleased(unsigned char keycode) noexcept;
	void OnChar(char character) noexcept;
	void ClearState() noexcept;
	template<typename T>
	static void TrimBuffer(std::queue<T>& buffer) noexcept;
private:
	static constexpr unsigned int nKeys = 256u;
	static constexpr unsigned int bufferSize = 16u;
	bool autorepeatEnabled = false;
	std::bitset<nKeys> keystates;			//bitSet은 비트필드를 <T> T만큼 사용할수있다
											//256u는 256가지의 키를 받을수 있다. 그리고 인덱싱을 통해 해당 비트필드에 접근하여 true,false 지정가능.
	std::queue<Event> keybuffer;			//키보드 입력을 저장하는 큐
	std::queue<char> charbuffer;			//문자열 키보드를 저장하는 큐


};

template<typename T>
inline void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept
{
	//버퍼크기보다 커지면 요소들을 pop
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}
