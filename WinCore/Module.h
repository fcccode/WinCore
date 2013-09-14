/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
