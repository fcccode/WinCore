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

/// @file Module
/// @author tcpie
/// @brief Contains code relevant to the Module class.

#ifndef _MODULE_H_
#define _MODULE_H_

#include <string>
#include <vector>

namespace tcpie { namespace wincore {

class MemoryRegion;
class Thread;
class Process;

/// @brief Describes a Windows module.
///
/// A module can be a loaded Dynamic Link Module (DLL), or an application's main module (example.exe)
class __declspec(dllexport) Module
{
private:
	MODULEENTRY32W module_info;
	MemoryRegion* memory_region;
	Process* owning_process;
	std::wstring* path;
	std::wstring* name;

public:
	/// @brief Constructs a Module.
	/// @param ModuleInfo		The module's info.
	Module(MODULEENTRY32W ModuleInfo);

	/// @brief Destructs a Module.
	~Module();

	/// @brief Gets the process that owns this module.
	/// @return The process this module belongs to.
	const Process* GetOwningProcess();
	
	/// @brief Gets the module's name.
	/// @return The module's name.
	const std::wstring* GetName() const { return this->name; }

	/// @brief Gets the module's ID.
	/// @return The module's ID.
	DWORD GetId() const { return this->module_info.th32ModuleID; }

	/// @brief Gets the module's memory region.
	/// @return The module's memory region.
	const MemoryRegion* GetMemoryRegion() const { return this->memory_region; }

	/// @brief Finds all threads started in this module.
	/// @return A vector containing the threads started in this module.
	///
	/// The caller is responsible for deleting the vector and the contained Threads.
	std::vector<Thread*>* FindThreadsStartedHere() const;
};

} }

#endif
