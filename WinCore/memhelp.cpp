/*
Copyright (c) 2013 Stijn "tcpie" Hinterding (contact: contact at tcpie dot eu)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "stdafx.h"
#include "memhelp.h"

namespace tcpie { namespace wincore {

bool memhelp::IsAddrInRegion(void* Start, void* End, void* Addr)
{
	return (Addr >= Start && Addr <= End);
}

bool memhelp::IsAddrInArea(void* Start, DWORD Size, void* Addr)
{
	return memhelp::IsAddrInRegion(Start, (void*)((DWORD)Start + Size), Addr);
}

int memhelp::PlaceCode(BYTE* Code, size_t CodeLength, void** CodeAddress)
{
	void* code_address = (*CodeAddress == NULL) ? malloc(CodeLength) : *CodeAddress; 

	if (code_address == NULL)
	{
		return 1;
	}

	DWORD old_protect = 0;

	if (!VirtualProtect(code_address, CodeLength, PAGE_EXECUTE_READWRITE, &old_protect))
	{
		if (*CodeAddress != NULL)
		{
			free(code_address);
		}

		return 2;
	}

	memcpy(code_address, Code, CodeLength);

	*CodeAddress = code_address;

	return 0;
}

void* memhelp::FindDwordLocation(DWORD Value, DWORD* StartAddress, void* EndAddress, size_t StepSize)
{
	DWORD* current_address = StartAddress;

	for (unsigned int i = (unsigned int)StartAddress; i < (unsigned int)EndAddress; i++)
	{
		DWORD temp_value = 0;
		temp_value = *current_address;

		if (temp_value == Value)
		{
			return current_address;
		}

		current_address = (DWORD*)((DWORD)current_address + StepSize);
	}

	return NULL;
}

void* memhelp::FindDwordLocationRemotely(HANDLE ProcessHandle, DWORD value, DWORD* StartAddress, void* EndAddress, size_t StepSize)
{
	if (ProcessHandle == GetCurrentProcess())
	{
		return memhelp::FindDwordLocation(value, StartAddress, EndAddress, StepSize);
	}

	DWORD* current_address = StartAddress;

	for (unsigned int i = (unsigned int)StartAddress; i < (unsigned int)EndAddress; i++)
	{
		DWORD temp_value = 0;

		if (!ReadProcessMemory(ProcessHandle, current_address, &temp_value, sizeof(DWORD), NULL))
		{
			int error = GetLastError();
			return NULL;
		}

		if (temp_value == value)
		{
			return current_address;
		}

		current_address = (DWORD*)((DWORD)current_address + StepSize);
	}

	return NULL;
}

} }
