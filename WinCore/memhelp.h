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

#ifndef _MEMHELP_H_
#define _MEMHELP_H_

#include "WinCore.h"

namespace tcpie { namespace wincore {

class __declspec(dllexport) memhelp
{
public:
	static bool IsAddrInRegion(void* Start, void* End, void* Addr);

	static bool IsAddrInArea(void* Start, DWORD Size, void* Addr);

	// PlaceCode function
	//
	// Places the data in Code at address *CodeAddress. If *CodeAddress is NULL,
	// the needed memory is allocated using malloc and the page protection
	// value is set to PAGE_EXECUTE_READWRITE using VirtualProtect
	//
	// returns:
	// 0:	success
	// 1:	malloc failed
	// 2:	VirtualProtect failed
	static int PlaceCode(BYTE* Code, size_t CodeLength, void** CodeAddress);

	static void* FindDwordLocation(DWORD Value, DWORD* StartAddress, void* EndAddress, size_t StepSize = sizeof(void*));

	static void* FindDwordLocationRemotely(HANDLE ProcessHandle, DWORD value, DWORD* StartAddress, void* EndAddress, size_t StepSize = sizeof(void*));
};

} }


#endif
