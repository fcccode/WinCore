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

#ifndef _MEMORYREGION_H_
#define _MEMORYREGION_H_

#include <vector>

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

	void* GetStartAddress() const { return this->start_address; }
	void* GetEndAddress() const { return this->end_address; }
	DWORD GetSize() const { return this->size; }

	void* ReplaceFirstOccurence(DWORD Occurence, DWORD Replacement) const;

	void* FindAddress(const std::vector<BYTE>* Signature, const std::vector<char>* SignatureMask, size_t StepSize = 1) const;

	bool ContainsAddress(void* Address) const;
	bool OverlapsWith(const MemoryRegion* Region) const;
};

} }

#endif
