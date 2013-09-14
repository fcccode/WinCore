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
#include "Thread.h"
#include "Process.h"
#include "MemoryRegion.h"

#define PROCESS_TINKER (PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_WRITE | PROCESS_VM_READ)
#define THREAD_TINKER (THREAD_GET_CONTEXT    | THREAD_SET_CONTEXT	 | THREAD_SUSPEND_RESUME)

#define QUERY_WIN32_START_ADDR 9

namespace tcpie { namespace wincore {

typedef int (WINAPI *NTQUERYINFORMATIONTHREAD)(HANDLE, LONG, PVOID, ULONG, PULONG);

Thread::Thread(THREADENTRY32 ThreadInfo)
{
	this->thread_info = ThreadInfo;
	this->id = ThreadInfo.th32ThreadID;
	this->handle = OpenThread(THREAD_TINKER, false, this->id);
	this->start_address = NULL;
	this->owner = NULL; 
	this->owner_id = ThreadInfo.th32OwnerProcessID;
}

Thread::~Thread()
{
	CloseHandle(this->handle);
}

Process* Thread::GetOwningProcess()
{
	if (this->owner == NULL)
	{
		this->owner = Process::FindProcessById(this->owner_id);
	}
	
	return this->owner;
}

void Thread::PushToStack(std::vector<void*>* Args)
{
	CONTEXT curr_context = this->GetContext();
	curr_context.ContextFlags = CONTEXT_ALL | CONTEXT_DEBUG_REGISTERS;

	curr_context.Esp -= Args->size() * sizeof(DWORD*);
	
	if (this->GetOwningProcess()->GetId() == Process::GetCurrentProcessId())
	{				
		void** stack = (void**)curr_context.Esp;
		DWORD old_prot = 0;

		bool success = VirtualProtect((void*)stack, Args->size(), PAGE_READWRITE, &old_prot) != 0;
		
		for (size_t i = 0; i < Args->size(); i++) // We push right-to-left
		{
			stack[i] = Args->at(i);
		}			
	}
	else
	{
		MemoryRegion* mem_region = this->GetOwningProcess()->WriteMemory((void*)&(*Args)[0], Args->size() * sizeof(DWORD*), (void*)curr_context.Esp);
	}

	curr_context.Esp -= sizeof(DWORD*);

	this->SetContext(curr_context);	
}

void Thread::Suspend()
{
	SuspendThread(this->handle);
}

void Thread::Resume()
{
	ResumeThread(this->handle);
}

CONTEXT Thread::GetContext()
{
	CONTEXT ret;
	ret.ContextFlags = CONTEXT_ALL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(this->handle, &ret);

	return ret;
}

bool Thread::SetContext(CONTEXT value)
{
	return SetThreadContext(this->handle, &value) != 0;
}

DWORD Thread::GetCurrentThreadId()
{
	HANDLE curr_handle = ::GetCurrentThread();
	DWORD id = GetThreadId(curr_handle);
	CloseHandle(curr_handle);

	return id;
}

Thread* Thread::GetCurrentThread()
{
	HANDLE curr_handle = ::GetCurrentThread();
	DWORD id = GetThreadId(curr_handle);
	CloseHandle(curr_handle);

	return Thread::FindThreadById(id);
}

Thread* Thread::FindOldest(std::vector<Thread*>* Threads)
{
	Thread* oldest_thread = NULL;
	FILETIME oldest_time;

	for (unsigned int i = 0; i < Threads->size(); i++)
	{
		FILETIME creation_time;
		FILETIME exit_time;
		FILETIME kernel_time;
		FILETIME user_time;

		bool success = GetThreadTimes((HANDLE)Threads->at(i)->GetId(), &creation_time, &exit_time, &kernel_time, &user_time) != 0;

		if (oldest_thread == NULL || CompareFileTime(&oldest_time, &creation_time) == 1)
		{
			oldest_time = creation_time;
			oldest_thread = Threads->at(i);
		}
	}

	return oldest_thread;
}

Thread* Thread::FindThreadById(DWORD ThreadId)
{
	std::vector<Thread*>* threads = Thread::GetSystemThreads();
	Thread* ret_thread = NULL;

	for (size_t i = 0; i < threads->size(); i++)
	{
		if (threads->at(i)->GetId() == ThreadId)
		{
			ret_thread = threads->at(i);

			break;
		}
	}

	delete threads;

	return ret_thread;
}

std::vector<Thread*>* Thread::GetSystemThreads()
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
		ret->push_back(new Thread(te));
		
        returnValue =  Thread32Next(thSnapshot,&te);
        te.dwSize = sizeof(THREADENTRY32);
    }

    CloseHandle(thSnapshot);
    
	return ret;
}

void* Thread::GetStartAddress()
{
	if (this->start_address != NULL)
	{		
		return this->start_address;
	}
	
	HANDLE ThreadHandle = OpenThread(THREAD_QUERY_INFORMATION, false, this->id);
	int ntStatus;
    void* startAddr = NULL;
    HANDLE currProc, newThreadHandle;

	// NtQueryInformationThread is not documented, so we have to do it the hard way...
    NTQUERYINFORMATIONTHREAD NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationThread");

    if (NtQueryInformationThread == INVALID_HANDLE_VALUE)
    {
		return 0;
	}

    currProc = GetCurrentProcess();

    if (DuplicateHandle(currProc, ThreadHandle, currProc, &newThreadHandle, THREAD_QUERY_INFORMATION, FALSE, 0))
    {
        ntStatus = NtQueryInformationThread(newThreadHandle, QUERY_WIN32_START_ADDR, &startAddr, sizeof(void*), NULL);

        if (ntStatus != 0)
        {
            CloseHandle(newThreadHandle);

            return 0;
		}

        CloseHandle(newThreadHandle);
    }

    return startAddr;
}

Thread* Thread::Create(Process* HostProcess, void* StartAddress, void* Parameter)
{
	DWORD thread_id = 0;

	if (HostProcess != Process::GetCurrentProcess())
	{
		HANDLE proc_handle = OpenProcess(PROCESS_TINKER, false, HostProcess->GetId());
	
		CreateRemoteThread(proc_handle, NULL, 0, (LPTHREAD_START_ROUTINE)StartAddress, (LPVOID)Parameter, 0, &thread_id);
	}
	else
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartAddress, (LPVOID)Parameter, 0, &thread_id);
	}

	return Thread::FindThreadById(thread_id);
}

} }
