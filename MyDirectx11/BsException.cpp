#include "BsException.h"
#include <sstream>

BsException::BsException(int line, const char* file) noexcept
	:line(line), file(file)
{
}
const char* BsException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* BsException::GetType() const noexcept
{
	return "BsException";
}

int BsException::GetLine() const noexcept
{
	return line;
}

const std::string& BsException::GetFile() const noexcept
{
	return file;
}

std::string BsException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl << "[Line] " << line;
	return oss.str();
}
