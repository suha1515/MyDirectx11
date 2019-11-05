#include "Keyboard.h"


Keyboard::~Keyboard()
{
}

bool Keyboard::KeyIsPressed(unsigned char keycode) const noexcept
{
	return keystates[keycode];
}

Keyboard::Event Keyboard::ReadKey() noexcept
{
	if (keybuffer.size() > 0u)
	{
		Keyboard::Event e = keybuffer.front();
		keybuffer.pop();
		return e;
	}
	else
		return Keyboard::Event();
}

bool Keyboard::KeyIsEmpty() const noexcept
{
	return keybuffer.empty();
}

char Keyboard::ReadChar() noexcept
{
	if (charbuffer.size() > 0u)
	{
		unsigned char charcode = charbuffer.front();
		charbuffer.pop();
		return charcode;
	}
	else
		return 0;
}

void Keyboard::FlushKey() noexcept
{
	keybuffer = std::queue<Event>();
}
bool Keyboard::CharIsEmpty() const noexcept
{
	return charbuffer.empty();
}

void Keyboard::FlushChar() noexcept
{
	charbuffer = std::queue<char>();
}

void Keyboard::Flush() noexcept
{
	FlushKey();
	FlushChar();
}

void Keyboard::EnableAutorepeat() noexcept
{
	autorepeatEnabled = true;
}

void Keyboard::DisableAutorepeat() noexcept
{
	autorepeatEnabled = false;
}

bool Keyboard::AutorepeatIsEnabled() noexcept
{
	return autorepeatEnabled;
}

//-------list of function which is used by WindowsAPI------------
void Keyboard::OnKeyPressed(unsigned char keycode) noexcept
{
	keystates[keycode] = true;		//set bitfield true where the keycode is pointing
	keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, keycode));	//and push Event with Event type and Keycode
	TrimBuffer(keybuffer);			// call Trim buffer just in case buffer goes oversize
}

void Keyboard::OnKeyReleased(unsigned char keycode) noexcept
{
	keystates[keycode] = false;		//same as OnKeyPressed only diffrent thing is type
	keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, keycode));
	TrimBuffer(keybuffer);
}

void Keyboard::OnChar(char character) noexcept
{
	charbuffer.push(character);			//char buffer cant accept Event so only can push char info
	TrimBuffer(charbuffer);
}

void Keyboard::ClearState() noexcept
{
	keystates.reset();
}
//--------------------------------------------------------------