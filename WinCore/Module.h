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

#ifndef _MODULE_H_
#define _MODULE_H_

#include <string>
#include <vector>

namespace tcpie { namespace wincore {

class MemoryRegion;
class Thread;
class Process;

class __declspec(dllexport) Module
{
private:
	MODULEENTRY32W module_info;
	MemoryRegion* memory_region;
	Process* owning_process;
	std::wstring* path;
	std::wstring* name;

public:
	Module(MODULEENTRY32W ModuleInfo);

	Process* GetOwningProcess();
	
	std::wstring* GetName() { return this->name; }
	DWORD GetId() { return this->module_info.th32ModuleID; }
	MemoryRegion* GetMemoryRegion() { return this->memory_region; }

	void* GetProcAddress(std::wstring ProcName);
	std::vector<Thread*>* FindThreadsStartedHere();
};

} }

#endif
