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
