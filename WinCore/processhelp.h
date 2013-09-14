/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PROCESSHELP_H_
#define _PROCESSHELP_H_

#include "WinCore.h"

#include <string>
#include <vector>

namespace tcpie { namespace wincore {

struct smallthreadinfo
{
public:
	DWORD ID;
	DWORD StartAddress;
};

class __declspec(dllexport) processhelp
{
public:
	// GetProcessIdFromName function
	//
	// Returns the PID of the first process with name ProcessName
	//
	// Returns:
	// NULL:			failure
	// anything else:	success
	static unsigned long GetProcessIdFromName(std::wstring ProcessName);

	// GetRunningProcesses function
	//
	// Returns a vector containing the currently active processes
	static std::vector<PROCESSENTRY32>* GetRunningProcesses();

	// GetRunningThreads function
	//
	// Returns a vector containing the currently active threads in the system
	static std::vector<THREADENTRY32>* GetRunningThreads();

	// GetProcessModules function
	//
	// Returns a vector containing the modules of the specified process
	static std::vector<MODULEENTRY32>* GetProcessModules(DWORD ProcessId);

	// GetOldestThreadIdFromProcess
	//
	// Returns the ID of the oldest (aka "main") thread from a certain process.
	//
	// Returns 0 on failure.
	static unsigned long GetOldestThreadIdFromProcess(DWORD ProcessId);

	// GetThreadStartAddress function
	//
	// Gets the start address of the specified thread.
	//
	// NOTE: this function uses undocumented Windows functions and thus is liable to break!
	static void* GetThreadStartAddress(HANDLE ThreadHandle);

	// FindModuleThreads function
	//
	// Returns a vector containing information on threads started *in* the specified module
	static std::vector<smallthreadinfo>* FindModuleThreads(DWORD ProcessId, MODULEENTRY32 Module);
};

} }

#endif
