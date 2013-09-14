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

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <vector>
#include <string>
#include <iostream>

namespace tcpie { namespace wincore {

class Module;
class Thread;
class MemoryRegion;

class __declspec(dllexport) Process
{
private:
	DWORD id;
	HANDLE handle;
	PROCESSENTRY32W process_info;
	std::wstring* name;
	Module* main_module;

	std::vector<Module*>* module_cache;

	static Process* current_process;

public:
	Process(PROCESSENTRY32W ProcessInfo);
	~Process();

	const DWORD GetId();

	std::wstring* GetName();
	Module* GetMainModule();
	
	std::vector<Module*>* GetModules();
	std::vector<Thread*>* GetThreads();

	MemoryRegion* WriteMemory(MemoryRegion* MemoryToWrite, void* Destination = NULL);
	MemoryRegion* WriteMemory(void* Start, DWORD Size, void* Destination = NULL);
	
	bool FreeMemory(MemoryRegion* Region);

	std::vector<BYTE>* ReadMemory(MemoryRegion* Region);

	Module* FindModuleByName(std::wstring* Name);
	Thread* GetOldestThread();
	
	static DWORD GetCurrentProcessId();
	static Process* GetCurrentProcess();
	static Process* FindProcessById(DWORD Id);
	static std::vector<Process*>* GetSystemProcesses();
};

} }

#endif
