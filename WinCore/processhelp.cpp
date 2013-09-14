/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "stdafx.h"
#include "processhelp.h"
#include "memhelp.h"

#define QUERY_WIN32_START_ADDR 9

namespace tcpie { namespace wincore {

typedef int (WINAPI *NTQUERYINFORMATIONTHREAD)(HANDLE, LONG, PVOID, ULONG, PULONG);

unsigned long processhelp::GetProcessIdFromName(std::wstring ProcessName)
{
	PROCESSENTRY32 pe;
    HANDLE thSnapshot;
    int returnValue, procFound = 0;

    thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if(thSnapshot == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    pe.dwSize = sizeof(PROCESSENTRY32);

    returnValue = Process32First(thSnapshot, &pe);

    while(returnValue)
    {
        DWORD tempDWORD = 0;
        if(tempDWORD = StrStrI(pe.szExeFile, ProcessName.c_str()) && std::wstring(pe.szExeFile).length() == ProcessName.length())
        {           
            if (tempDWORD == 1)
            {
                CloseHandle(thSnapshot);
                return pe.th32ProcessID;
            }
        }

        returnValue =  Process32Next(thSnapshot,&pe);
        pe.dwSize = sizeof(PROCESSENTRY32);
    }

    CloseHandle(thSnapshot);
    return NULL;
}

std::vector<PROCESSENTRY32>* processhelp::GetRunningProcesses()
{
	std::vector<PROCESSENTRY32>* ret = new std::vector<PROCESSENTRY32>();

	PROCESSENTRY32 pe;
    HANDLE thSnapshot;
    int returnValue, procFound = 0;

    thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if(thSnapshot == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    pe.dwSize = sizeof(PROCESSENTRY32);

    returnValue = Process32First(thSnapshot, &pe);

    while(returnValue)
    {
		ret->push_back(pe);
		
        returnValue =  Process32Next(thSnapshot,&pe);
        pe.dwSize = sizeof(PROCESSENTRY32);
    }

    CloseHandle(thSnapshot);
    
	return ret;
}

std::vector<THREADENTRY32>* processhelp::GetRunningThreads()
{
	std::vector<THREADENTRY32>* ret = new std::vector<THREADENTRY32>();

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
		ret->push_back(te);
		
        returnValue =  Thread32Next(thSnapshot,&te);
        te.dwSize = sizeof(THREADENTRY32);
    }

    CloseHandle(thSnapshot);
    
	return ret;
}

std::vector<MODULEENTRY32>* processhelp::GetProcessModules(DWORD ProcessId)
{
	std::vector<MODULEENTRY32>* ret = new std::vector<MODULEENTRY32>();

	// We get a snapshot of the process
    HANDLE procSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessId);

    // A module-entry in the process
    MODULEENTRY32 mod32;
    mod32.dwSize = sizeof(MODULEENTRY32);

    BOOL retVal = Module32First(procSnapshot, &mod32);

    while (retVal)
    {
		ret->push_back(mod32);

        // We iterate through all modules
        retVal = Module32Next(procSnapshot, &mod32);
    }

    CloseHandle(procSnapshot);

	return ret;
}

std::vector<smallthreadinfo>* FindModuleThreads(DWORD ProcessId, MODULEENTRY32 Module)
{
	std::vector<smallthreadinfo>* ret = new std::vector<smallthreadinfo>();

    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(THREADENTRY32);
    threadEntry.cntUsage = 0;

    HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, ProcessId);

    if (snapShot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    BOOL retVal = Thread32First(snapShot, &threadEntry);

    while (retVal)
	{
        if (threadEntry.th32OwnerProcessID == ProcessId)
        {
            HANDLE threadHandle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, threadEntry.th32ThreadID);

            void* startAddr = processhelp::GetThreadStartAddress(threadHandle);

            if (memhelp::IsAddrInArea(Module.modBaseAddr, Module.modBaseSize, startAddr))
            {
				smallthreadinfo temp_ret;
				temp_ret.ID = threadEntry.th32ThreadID;
				temp_ret.StartAddress = (DWORD)startAddr;

				ret->push_back(temp_ret);
            }

            CloseHandle(threadHandle);
        }

        retVal = Thread32Next(snapShot, &threadEntry);
    }

    CloseHandle(snapShot);

    return ret;
}

unsigned long processhelp::GetOldestThreadIdFromProcess(DWORD ProcessId)
{
	std::vector<THREADENTRY32>* threads = processhelp::GetRunningThreads();

	THREADENTRY32 oldest_thread;
	FILETIME oldest_time;
	oldest_thread.dwFlags = 0x1337;

	for (unsigned int i = 0; i < threads->size(); i++)
	{
		if (threads->at(i).th32OwnerProcessID != ProcessId)
		{
			continue;
		}

		FILETIME creation_time;
		FILETIME exit_time;
		FILETIME kernel_time;
		FILETIME user_time;

		bool success = false;
		success = GetThreadTimes((HANDLE)threads->at(i).th32ThreadID, &creation_time, &exit_time, &kernel_time, &user_time) != 0;

		if (oldest_thread.dwFlags == 0x1337 || CompareFileTime(&oldest_time, &creation_time) == 1)
		{
			oldest_time = creation_time;
			oldest_thread = threads->at(i);
		}
	}

	if (oldest_thread.dwFlags == 0x1337)
	{
		return 0;
	}

	return oldest_thread.th32ThreadID;
}

void* processhelp::GetThreadStartAddress(HANDLE ThreadHandle)
{
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

} }
