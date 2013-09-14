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
#include "Module.h"
#include "MemoryRegion.h"
#include "Thread.h"
#include "Process.h"

namespace tcpie { namespace wincore {

Module::Module(MODULEENTRY32W ModuleInfo)
{
	this->module_info = ModuleInfo;
	this->memory_region = new MemoryRegion((void*)this->module_info.modBaseAddr, this->module_info.modBaseSize);
	this->owning_process = NULL;
	this->path = new std::wstring(this->module_info.szExePath);
	this->name = new std::wstring(this->module_info.szModule);
}

std::vector<Thread*>* Module::FindThreadsStartedHere()
{
	std::vector<Thread*>* threads = this->owning_process->GetThreads();
	std::vector<Thread*>* ret = new std::vector<Thread*>();

	for (size_t i = 0; i < threads->size(); i++)
	{
		if (this->memory_region->ContainsAddress(threads->at(i)->GetStartAddress()))
		{
			ret->push_back(threads->at(i));
		}
	}

	delete threads;

	return ret;
}

Process* Module::GetOwningProcess()
{
	if (this->owning_process == NULL)
	{
		this->owning_process = Process::FindProcessById(this->module_info.th32ProcessID);
	}
		
	return this->owning_process; 
}

} }
