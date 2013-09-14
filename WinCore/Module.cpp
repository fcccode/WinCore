/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
