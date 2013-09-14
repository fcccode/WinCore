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

	DWORD GetId() const { return this->id; }

	void* GetStartAddress();
	
	void PushToStack(const std::vector<void*>* Args);

	void Suspend() const;
	void Resume() const;

	CONTEXT GetContext() const;
	bool SetContext(CONTEXT value) const;

	const Process* GetOwningProcess();

	static DWORD GetCurrentThreadId();
	static Thread* GetCurrentThread();
	static Thread* FindOldest(const std::vector<Thread*>* Threads);
	static Thread* FindThreadById(DWORD ThreadId);
	static std::vector<Thread*>* GetSystemThreads();
	static Thread* Create(const Process* HostProcess, void* StartAddress, void* Parameter);
};

} }

#endif
