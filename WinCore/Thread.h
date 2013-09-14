/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _THREAD_H_
#define _THREAD_H_

#include <vector>
#include <TlHelp32.h>

namespace tcpie { namespace wincore {

class Process;

class __declspec(dllexport) Thread
{
private:
	DWORD id;
	HANDLE handle;

	THREADENTRY32 thread_info;
	void* start_address;
	Process* owner;
	DWORD owner_id;

public:
	Thread(THREADENTRY32 ThreadInfo);
	~Thread();

	DWORD GetId() { return this->id; }

	void* GetStartAddress();
	
	void PushToStack(std::vector<void*>* Args);

	void Suspend();
	void Resume();
	CONTEXT GetContext();
	bool SetContext(CONTEXT value);

	Process* GetOwningProcess();

	static DWORD GetCurrentThreadId();
	static Thread* GetCurrentThread();
	static Thread* FindOldest(std::vector<Thread*>* Threads);
	static Thread* FindThreadById(DWORD ThreadId);
	static std::vector<Thread*>* GetSystemThreads();
	static Thread* Create(Process* HostProcess, void* StartAddress, void* Parameter);
};

} }

#endif
