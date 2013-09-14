/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _MEMORYREGION_H_
#define _MEMORYREGION_H_

namespace tcpie { namespace wincore {

class __declspec(dllexport) MemoryRegion
{
private:
	void* start_address;
	void* end_address;
	DWORD size;

public:
	MemoryRegion(void* StartAddress, DWORD Size);
	MemoryRegion(void* StartAddress, void* EndAddress);

	~MemoryRegion() { }

	void* GetStartAddress() { return this->start_address; }
	void* GetEndAddress() { return this->end_address; }
	DWORD GetSize() { return this->size; }

	void* ReplaceFirstOccurence(DWORD Occurence, DWORD Replacement);

	bool ContainsAddress(void* Address);
	bool OverlapsWith(MemoryRegion* Region);
};

} }

#endif
