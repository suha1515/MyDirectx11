#include <string>
#include <sstream>
#include "App.h"

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR	  lpCmdLine,
	int		  nCmdShow)
{
	try
	{
		//여기서 커맨드라인을 입력한다.
		return App{lpCmdLine}.Go();
	}
	catch (const BsException & e)
	{
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception & e)
	{
		MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, "No Details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
		return -1;
	}
}