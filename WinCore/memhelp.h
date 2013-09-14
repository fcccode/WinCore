/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
