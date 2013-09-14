#ifndef _MEMSEARCHABLE_H_
#define _MEMSEARCHABLE_H_

#include <vector>

namespace tcpie { namespace wincore {

class MemoryRegion;

class __declspec(dllexport) MemSearchable
{
protected:
	std::vector<unsigned char*>* signature;
	std::wstring* signature_mask;

public:
	MemSearchable(std::vector<unsigned char*>* Signature, std::wstring* SignatureMask);

	void* FindAddress(MemoryRegion* Region, size_t StepSize = sizeof(void*));
};

} }

#endif
