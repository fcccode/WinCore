/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
	std::wstring* path;
	Module* main_module;

	std::vector<Module*>* module_cache;

	static Process* current_process;

public:
	Process(PROCESSENTRY32W ProcessInfo);
	~Process();

	const DWORD GetId();

	std::wstring* GetPath();
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
