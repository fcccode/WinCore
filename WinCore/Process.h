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

/// @file Process
/// @author tcpie
/// @brief Contains code relevant to the Process class.

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <vector>
#include <string>
#include <iostream>

namespace tcpie { namespace wincore {

class Module;
class Thread;
class MemoryRegion;

/// @brief Describes a Windows process.
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
	/// @brief Constructs a Process.
	/// @param ProcessInfo		The process's info.
	Process(PROCESSENTRY32W ProcessInfo);

	/// @brief Destructs a Process.
	~Process();

	/// @brief Gets the process id.
	/// @return The process id (pid).
	DWORD GetId() const;

	/// @brief Gets the process name.
	/// @return The process name.
	const std::wstring* GetName() const;

	/// @brief Gets the process's main module.
	/// @return The process's main module (often "process.exe")
	const Module* GetMainModule() const;
	
	/// @brief Finds all modules in this process.
	/// @return A vector containing all modules in this process.
	///
	/// Note: like all other Find* functions, this function should be regarded as slow!
	///
	/// The caller is responsible for deleting the vector and the contained Modules.
	std::vector<Module*>* FindModules() const;

	/// @brief Finds all threads in this process.
	/// @return A vector containing all threads in this process.
	///
	/// Note: like all other Find* functions, this function should be regarded as slow!
	///
	/// The caller is responsible for deleting the vector and the contained Threads.
	std::vector<Thread*>* FindThreads() const;

	/// @brief Writes data to the process's memory.
	/// @param MemoryToWrite		The data to write.
	/// @param Destination			Where to place the data inside the process. If Destination is NULL, space will be allocated using VirtualAllocEx().
	/// @return						A MemoryRegion describing the region where the data was placed.
	MemoryRegion* WriteMemory(const MemoryRegion* MemoryToWrite, void* Destination = NULL) const;

	/// @brief Writes data to the process's memory.
	/// @param Start				Where the data starts
	/// @param Size					The size of the buffer to write.
	/// @param Destination			Where to place the data inside the process. If Destination is NULL, space will be allocated using VirtualAllocEx().
	/// @return						A MemoryRegion describing the region where the data was placed.
	MemoryRegion* WriteMemory(void* Start, DWORD Size, void* Destination = NULL) const;
	
	/// @brief Frees a Written MemoryRegion.
	/// @param Region				The region to free.
	/// @return						A value indicating whether the freeing was successful.
	///
	/// This function should only be called for a Region which was returned from WriteMemory, where Destination was set to NULL!
	///
	/// If the function succeeds, Region is deleted. If the function fails, Region is not deleted.
	bool FreeMemory(MemoryRegion* Region) const;

	/// @brief Reads memory from this process.
	/// @param Region				The memory region to read from.
	/// @return						A vector containing the read data. If reading failed, NULL will be returned.
	///
	/// The caller is responsible for deleting the returned vector.
	std::vector<BYTE>* ReadMemory(const MemoryRegion* Region) const;

	/// @brief Finds a module by name.
	/// @param Name					The name to search for.
	/// @return						The found Module. If no Module was found, NULL will be returned.
	///
	/// Note: like all other Find* functions, this function should be regarded as slow!
	Module* FindModuleByName(const std::wstring* Name);

	/// @brief Finds the oldest thread inside this process.
	/// @return						The oldest thread.
	///
	/// Note: like all other Find* functions, this function should be regarded as slow!
	const Thread* FindOldestThread() const;
	
	/// @brief Gets the ID of the current process.
	/// @return The ID of the current process.
	static DWORD GetCurrentProcessId();

	/// @brief Gets the current process.
	/// @return The current process.
	static Process* GetCurrentProcess();

	/// @brief Finds a process by its ID.
	/// @return The found process. If no process was found, NULL will be returned.
	///
	/// Note: like all other Find* functions, this function should be regarded as slow!
	static Process* FindProcessById(DWORD Id);

	/// @brief Gets the currently active processes in the system.
	/// @return A vector containing the currently active processes in the system.
	///
	/// Note: this function should be regarded as slow!
	static std::vector<Process*>* GetSystemProcesses();
};

} }

#endif
