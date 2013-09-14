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
#include "MemSearchable.h"
#include "MemoryRegion.h"

namespace tcpie { namespace wincore {

MemSearchable::MemSearchable(std::vector<unsigned char*>* Signature, std::wstring* SignatureMask)
{
	this->signature = Signature;
	this->signature_mask = SignatureMask;
}

void* MemSearchable::FindAddress(MemoryRegion* Region, size_t StepSize)
{
	if (this->signature == NULL || this->signature_mask == NULL)
	{
		return NULL;
	}

	unsigned int counter = 0;

	for (BYTE* i = (BYTE*)Region->GetStartAddress(); i <= Region->GetEndAddress(); i += StepSize)
	{
		if (*i == (BYTE)this->signature->at(counter) || this->signature_mask->at(counter) == '?')
		{
			counter++;
		}
		else if (*i != (BYTE)this->signature->at(counter))
		{
			counter = 0;
		}

		if (counter >= this->signature->size() - 1)
		{
			return (void*)i;
		}
	}

	return NULL;
}

} }
