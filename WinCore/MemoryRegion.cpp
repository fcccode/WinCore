/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "stdafx.h"
#include "MemoryRegion.h"

namespace tcpie { namespace wincore {

MemoryRegion::MemoryRegion(void* StartAddress, void* EndAddress)
{
	this->start_address = StartAddress;
	this->end_address = EndAddress;
	this->size = (DWORD)((DWORD)EndAddress - (DWORD)StartAddress);
}

MemoryRegion::MemoryRegion(void* StartAddress, DWORD Size)
{
	this->start_address = StartAddress;
	this->end_address = (void*)((DWORD)StartAddress + Size);
	this->size = Size;
}

void* MemoryRegion::ReplaceFirstOccurence(DWORD Occurence, DWORD Replacement)
{
	for (BYTE* i = (BYTE*)this->GetStartAddress(); i < (BYTE*)this->GetEndAddress(); i++)
	{
		DWORD cmp = *(DWORD*)i;

		if (cmp == Occurence)
		{
			*(DWORD*)i = Replacement;

			return i;
		}
	}

	return NULL;
}

bool MemoryRegion::ContainsAddress(void* Address)
{
	return (Address >= this->start_address && Address <= this->end_address);
}

bool MemoryRegion::OverlapsWith(MemoryRegion* Region)
{
	return (this->ContainsAddress(Region->GetStartAddress()) || this->ContainsAddress(Region->GetEndAddress()));
}
} }
