#pragma once
//#include <Windows.h>
#include <cwchar>
#include <string>

namespace Unity
{
	namespace Obfuscators
	{
		std::string ROT_String(const char* pString, int iValue);
	}
}