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
