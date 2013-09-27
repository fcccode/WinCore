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

/// @file		MemoryRegion.h
/// @author		tcpie
/// @brief		Contains code relevant to the MemoryRegion class.

#ifndef _MEMORYREGION_H_
#define _MEMORYREGION_H_

#include <vector>

namespace tcpie { namespace wincore {

/// @brief Represents a region of memory.
class __declspec(dllexport) MemoryRegion
{
private:
	void* start_address;
	void* end_address;
	DWORD size;

public:
	/// @brief Constructs a MemoryRegion
	/// @param StartAddress		The start address of the memory region.
	/// @param Size				The size, in bytes, of the memory region.
	MemoryRegion(void* StartAddress, DWORD Size);

	/// @brief Constructs a MemoryRegion.
	/// @param StartAddress		The start address of the memory region.
	/// @param EndAddress		The end address of the memory region.
	MemoryRegion(void* StartAddress, void* EndAddress);

	/// @brief Default destructor.
	~MemoryRegion() { }

	/// @brief Gets the start address.
	/// @return The start address.
	void* GetStartAddress() const { return this->start_address; }

	/// @brief Gets the end address.
	/// @return The end address.
	void* GetEndAddress() const { return this->end_address; }

	/// @brief Gets the size.
	/// @return The size (in bytes).
	DWORD GetSize() const { return this->size; }

	/// @brief Looks for the occurrence of a DWORD and replaces the first instance it finds.
	/// @param Occurence		The value to search for.
	/// @param Replacement		The value to replace with.
	/// @return					The address of the found occurrence. If the search yielded no results, the result value is NULL.
	void* ReplaceFirstOccurence(DWORD Occurence, DWORD Replacement) const;

	/// @brief Searches for the address of a Signature.
	/// @param Signature		The signature to search for.
	/// @param SignatureMask	The signature's mask. Should have the same size as Signature. Any bytes labeled with '?' will be ignored.
	/// @param StepSize			The step size, in bytes, to use when searching. The default is 1.
	/// @return					The found address. If the signature is not found, NULL is returned.
	///
	/// Same behaviour as the often-used sigscan() function.
	void* FindAddress(const std::vector<BYTE>* Signature, const std::vector<char>* SignatureMask, size_t StepSize = 1) const;

	/// @brief Checks if the provided address lies within its borders.
	/// @param Address			The address to check.
	/// @return					A value indicating whether or not the provided Address is within this memory region. (inclusive)
	bool ContainsAddress(void* Address) const;

	/// @brief Checks if the MemoryRegion overlaps with another MemoryRegion.
	/// @param Region			The MemoryRegion to check against.
	/// @return					A value indicating whether the two regions overlap.
	bool OverlapsWith(const MemoryRegion* Region) const;
};

} }

#endif
