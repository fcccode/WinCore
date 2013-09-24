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
#include "Process.h"
#include "Module.h"
#include "Thread.h"
#include "MemoryRegion.h"

#include <iostream>

#define PROCESS_TINKER (PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_WRITE | PROCESS_VM_READ)

namespace tcpie { namespace wincore {

Process* Process::current_process = NULL;

Process::Process(PROCESSENTRY32 ProcessInfo)
{
	this->id = ProcessInfo.th32ProcessID;
	this->handle = OpenProcess(PROCESS_TINKER, false, this->id);
	this->process_info = ProcessInfo;
	
	this->name = new std::wstring(ProcessInfo.szExeFile);

	this->module_cache = this->FindModules();
	this->main_module = NULL;

	for (size_t i = 0; i < this->module_cache->size(); i++)
	{
		if (this->module_cache->at(i)->GetId() == 1)
		{
			this->main_module = this->module_cache->at(i);

			break;
		}
	}
}

Process::~Process()
{
	CloseHandle(this->handle);

	delete this->name;

	if (this->main_module != NULL)
	{
		delete this->main_module;
	}

	for (size_t i = 0; i < this->module_cache->size(); i++)
	{
		delete this->module_cache->at(i);
	}

	delete this->module_cache;
}

DWORD Process::GetId() const { return this->id; }

const std::wstring* Process::GetName() const { return this->name; }

const Module* Process::GetMainModule() const { return this->main_module; }

std::vector<Module*>* Process::FindModules() const
{
	std::vector<Module*>* ret = new std::vector<Module*>();

	// We get a snapshot of the process
    HANDLE procSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, this->id);

    // A module-entry in the process
    MODULEENTRY32 mod32;
    mod32.dwSize = sizeof(MODULEENTRY32);

    BOOL retVal = Module32First(procSnapshot, &mod32);

    while (retVal)
    {
		ret->push_back(new Module(mod32));

        // We iterate through all modules
        retVal = Module32Next(procSnapshot, &mod32);
    }

    CloseHandle(procSnapshot);

	return ret;
}

std::vector<Thread*>* Process::FindThreads() const
{
	std::vector<Thread*>* ret = new std::vector<Thread*>();

	THREADENTRY32 te;
    HANDLE thSnapshot;
    int returnValue, procFound = 0;

    thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if(thSnapshot == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    te.dwSize = sizeof(THREADENTRY32);

    returnValue = Thread32First(thSnapshot, &te);

    while(returnValue)
    {
		if (te.th32OwnerProcessID == this->id)
		{
			ret->push_back(new Thread(te));
		}
		
        returnValue =  Thread32Next(thSnapshot,&te);
        te.dwSize = sizeof(THREADENTRY32);
    }

    CloseHandle(thSnapshot);
    
	return ret;
}

MemoryRegion* Process::WriteMemory(const MemoryRegion* MemoryToWrite, void* Destination /* = NULL */) const
{
	return this->WriteMemory(MemoryToWrite->GetStartAddress(), MemoryToWrite->GetSize(), Destination);
}

MemoryRegion* Process::WriteMemory(void* Start, DWORD Size, void* Destination /* = NULL */) const
{
	void* address = Destination;

	if (Destination == NULL)
	{
		address			  = VirtualAllocEx(this->handle, Destination, Size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	}

	SIZE_T num_written = 0;
	
	bool retval               = WriteProcessMemory(this->handle, address, Start, Size, &num_written) != 0;

	return new MemoryRegion(address, (DWORD)num_written);
}

bool Process::FreeMemory(MemoryRegion* Region) const
{
	bool ret = VirtualFreeEx(this->handle, Region->GetStartAddress(), 0, MEM_RELEASE) != 0; 
	
	if (ret)
	{
		delete Region;
	}

	return ret;
}

Module* Process::FindModuleByName(const std::wstring* Name)
{
	for (size_t j = 0; j < 2; j++)
	{
		for (size_t i = 0; i < this->module_cache->size(); i++)
		{
			if (this->module_cache->at(i)->GetName()->find(*Name) != std::wstring::npos)
			{
				return this->module_cache->at(i);
			}
		}

		if (j == 0)
		{
			this->module_cache = this->FindModules();
		}
	}

	return NULL;
}

const Thread* Process::GetOldestThread() const
{
	return Thread::FindOldest(this->FindThreads());
}

Process* Process::FindProcessById(DWORD Id)
{
	std::vector<Process*>* procs = Process::GetSystemProcesses();

	for (size_t i = 0; i < procs->size(); i++)
	{
		if (procs->at(i)->GetId() == Id)
		{
			return procs->at(i);
		}
	}

	return NULL;
}

std::vector<Process*>* Process::GetSystemProcesses()
{
	std::vector<Process*>* ret = new std::vector<Process*>();

	PROCESSENTRY32 pe;
    HANDLE thSnapshot;
    int returnValue, procFound = 0;

    thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if(thSnapshot == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    pe.dwSize = sizeof(PROCESSENTRY32);

    returnValue = Process32First(thSnapshot, &pe);
	
    while(returnValue)
    {
		ret->push_back(new Process(pe));
		
        returnValue =  Process32Next(thSnapshot,&pe);
        pe.dwSize = sizeof(PROCESSENTRY32);
    }

    CloseHandle(thSnapshot);

	return ret;
}

std::vector<BYTE>* Process::ReadMemory(const MemoryRegion* Region) const
{
	BYTE* ret = new BYTE[Region->GetSize()];
	SIZE_T num_bytes_read = 0;

	ReadProcessMemory(this->handle, Region->GetStartAddress(), ret, Region->GetSize(), &num_bytes_read);

	std::vector<BYTE>* retvector = new std::vector<BYTE>(ret, ret + Region->GetSize());

	/*
	for (int i = 0; i < Region->GetSize(); i++)
	{
		retvector->push_back(ret[i]);
	}
	*/
	delete [] ret;

	return retvector;
}

DWORD Process::GetCurrentProcessId()
{
	return ::GetCurrentProcessId();
}

Process* Process::GetCurrentProcess()
{
	if (Process::current_process != NULL)
	{
		return Process::current_process;
	}

	Process::current_process = Process::FindProcessById(GetCurrentProcessId());

	return Process::current_process;
}

} }
