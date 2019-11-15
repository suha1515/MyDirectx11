#pragma once
#include <queue>
#include <bitset>
#include <optional>

class Keyboard
{
	friend class Window;
public:
	class Event				//Ű���� Ű�� �������� �߻��ϴ� �̺�Ʈ Ŭ����
	{
	public:
		enum class Type
		{
			Press,			//��������
			Release,		//������������
			Invalid			//��ȿ�������� �̺�Ʈ
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
	//Window�� ���Ǵ� �Լ��� Ŭ���̾�Ʈ�� ����Ҽ�����
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
	std::bitset<nKeys> keystates;			//bitSet�� ��Ʈ�ʵ带 <T> T��ŭ ����Ҽ��ִ�
											//256u�� 256������ Ű�� ������ �ִ�. �׸��� �ε����� ���� �ش� ��Ʈ�ʵ忡 �����Ͽ� true,false ��������.
	std::queue<Event> keybuffer;			//Ű���� �Է��� �����ϴ� ť
	std::queue<char> charbuffer;			//���ڿ� Ű���带 �����ϴ� ť


};

template<typename T>
inline void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept
{
	//����ũ�⺸�� Ŀ���� ��ҵ��� pop
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}
