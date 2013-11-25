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

/// @file Thread.h
/// @author tcpie
/// @brief Contains code relevant to the Thread class.

#ifndef _THREAD_H_
#define _THREAD_H_

#include <windef.h>
#include <vector>
#include <map>
#include <TlHelp32.h>

namespace tcpie { namespace wincore {

class Process;

struct __declspec(dllexport) ThreadInformationBlock
{
public:
	void* SEHFrame;
	void* StackTop;
	void* StackBottom;
	DWORD unknown1;
	DWORD FiberData;
	DWORD DataSlot1;
	void* TIBAddress;
	void* EnvPointer;
	DWORD PID_or_DebugContext;
	DWORD CurrentTID;
	void* ActiveRPCHandle;
	void* ThreadLocalStorage;
	void* ProcessEnvBlock;
	DWORD LastError;
	DWORD NumOwnedCritSections;
	void* CSRClientThreadAddress;
	void* Win32ThreadInformation;
	DWORD Win32ClientInfo;
	void* Wow64FastSysCall;
	DWORD CurrLocale;
	DWORD FPSoftStatusRegister;
	DWORD Reserved;
	DWORD ExceptionCode;
	void* ActivationContextStack;
	DWORD SpareBytes;
	DWORD Reserved2;
	DWORD GdiTEBBatch;
	DWORD GdiRegion;
	DWORD GdiPen;
	DWORD GdiBrush;
	DWORD RealPID;
	DWORD RealTID;
	void* GdiCachedProcessHandle;
	DWORD GdiPID;
	DWORD GdiTID;
	void* GdiThreadLocaleInfo;
	DWORD Reserved3;

};

/// @brief Describes a Windows thread.
class __declspec(dllexport) Thread
{
private:
	DWORD id;
	HANDLE handle;

	THREADENTRY32 thread_info;
	void* start_address;
	Process* owner;
	DWORD owner_id;

	static std::map<DWORD, Thread*>* thread_pool;

public:
	/// @brief Constructs a new Thread
	/// @param ThreadInfo		The thread's info.
	Thread(THREADENTRY32 ThreadInfo);

	/// @brief Destructs a Thread.
	///
	/// Note: this destructs only this instance. The underlying win32 thread is unchanged.
	~Thread();

	/// @brief Gets the thread id.
	/// @return The thread id.
	DWORD GetId() const { return this->id; }

	/// @brief Gets the thread's starting address.
	/// @return The thread's starting address.
	///
	/// Note: The first call to this function may be slow.
	void* GetStartAddress();
	
	/// @brief Pushes the specified arguments to the thread's stack.
	/// @param Args			The arguments to push.
	///
	/// Note: you need to suspend the thread before calling this function! 
	void PushToStack(const std::vector<void*>* Args);

	/// @brief Suspends the thread.
	void Suspend() const;

	/// @brief Resumes the thread.
	///
	/// Note: if the thread was previously suspended for more than 1 time, the thread may still be suspended.
	void Resume() const;

	/// @brief Gets the thread's context.
	/// @return The thread's context.
	///
	/// Note: the returned value only makes sense if the thread is suspended during the call to GetContext.
	CONTEXT GetContext() const;

	/// @brief Sets the thread's context.
	/// @param value		The new context.
	/// @return				A value indicating whether the function succeeded.
	///
	/// Note: you should suspend the thread before calling this function!
	bool SetContext(CONTEXT value) const;

	/// @brief Checks if the thread has terminated.
	/// @return				A value indicating whether the thread has terminated.
	bool HasTerminated() const;

	/// @brief Gets the owning process.
	/// @return The process this thread belongs to.
	const Process* GetOwningProcess();

	/// @brief Gets the ID of the current thread.
	/// @return The ID of the current thread.
	static DWORD GetCurrentThreadId();

	/// @brief Gets the current thread.
	/// @return The current thread.
	///
	/// Note: This function may occasionally be slow. Try to store the current thread in a variable instead of calling this function often.
	static const Thread* GetCurrentThread();

	/// @brief Finds the oldest thread from a collection of threads.
	/// @param Threads			A vector containing the threads to search in.
	/// @return The oldest thread
	///
	/// Note: like all Find* functions, this function should be regarded as slow!
	static const Thread* FindOldest(const std::vector<Thread*>* Threads);

	/// @brief Finds a thread by its ID.
	/// @param ThreadId		The thread's ID.
	/// @return The found thread. If no thread was found, NULL will be returned.
	///
	/// Note: like all Find* functions, this function should be regarded as slow!
	static const Thread* FindThreadById(DWORD ThreadId);

	/// @brief Gets the threads currently active in the system.
	/// @return				A vector containing the threads currently active in the system.
	///
	/// Note: This function should be regarded as slow!
	static std::vector<Thread*>* GetSystemThreads();

	/// @brief Creates a thread.
	/// @param HostProcess		The process to create the thread in.
	/// @param StartAddress		The address the thread should start at.
	/// @param Parameter		The parameter passed to the function the thread starts at.
	/// @return The created thread.
	///
	/// Note: unlike the constructor, this function actually creats a win32 thread!
	/// Performance note: to get the created Thread, the created win32 is searched, meaning
	/// that this function is slow.
	static Thread* Create(const Process* HostProcess, void* StartAddress, void* Parameter);
};

} }

#endif
