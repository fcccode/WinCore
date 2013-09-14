/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
