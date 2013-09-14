// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "..\WinCore\Function.h"
#include "..\WinCore\Thread.h"
#include "..\WinCore\Process.h"
#include "..\WinCore\MemoryRegion.h"
#include "..\WinCore\Hook.h"

#include <vector>
#include <iostream>
#include <string>

#pragma comment(lib, "..\\Debug\\WinCore.lib")

using namespace tcpie::wincore;

int __stdcall add(int a, int b)
{
	//MessageBoxA(NULL, "ADD", "MSG", MB_OK);

	//std::cout << "fn Add(): a: " << a << " b: " << b << " ret = " << a + b << std::endl;

	return a + b;
}

int __cdecl add_cdelc(int a, int b)
{
	return a + b;
}

DetourRet __stdcall add_detour(DetourArgs* args)
{
	std::cout << "add called with " << (int)args->Arguments->at(0) << " ; " << (int)args->Arguments->at(1) << std::endl;

	args->Arguments->at(0) = (void*)1;
	args->CustomReturnValue = 1337;
	return DETOUR_ARGRETCHANGED;
}

int loop()
{
	while(true)
	{
		//std::cout << "hey!" << std::endl;
		// add_cdelc(3, 10);
		
		Sleep(0);
		Sleep(1);
		Sleep(10);
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	Thread* t = Thread::Create(Process::GetCurrentProcess(), loop, NULL);

	Function* fn = new Function((void*)add, STDCALL_CALLCONV, 2, DWORD_SIZE, NULL, NULL);
	Function* fn_c = new Function((void*)add_cdelc, CDECL_CALLCONV, 2, DWORD_SIZE, NULL, NULL);
	Function* fn_c_wrapped_to_stdcall = fn_c->CreateStdcallWrapper();

	Hook* h = Hook::CreateHook(fn_c, new std::wstring(L"fn_c"), 0);
	Hook* h2 = Hook::CreateHook(fn, new std::wstring(L"fn_add"), 0);

	Detour* add_detour_dt = h2->RegisterDetour(add_detour, DETOUR_PRE);
	h2->Enable();
	//h->Enable();

	std::vector<void*>* args = new std::vector<void*>();
	args->push_back((void*)2);
	args->push_back((void*)3);

	Thread* current_thread = Thread::GetCurrentThread();

	for (int i = 0; i < 100; i++)
	{
		DWORD ret1 = 0;
		DWORD ret2 = 0;
		DWORD ret3 = 0;

		fn->Call(args, current_thread, &ret1);
		fn_c->Call(args, current_thread, &ret2);		
		// fn_c_wrapped_to_stdcall->Call(args, current_thread, &ret3);

		std::cout << "#" << i << " ret 1, 2, 3: " << ret1 << " " << ret2 << " " << ret3 << std::endl;
		h2->SetEnabled(!h2->IsEnabled());
	}

	std::vector<Process*>* procs = Process::GetSystemProcesses();
	Process* target = NULL;
	
	for (size_t i = 0; i < procs->size(); i++)
	{
		if (*procs->at(i)->GetPath() == std::wstring(L"notepad++.exe"))
		{
			target = procs->at(i);

			break;
		}
	}

	if (target != NULL)
	{
		MemoryRegion* mem = target->WriteMemory(add, (DWORD)add_cdelc - (DWORD)add);
		std::cout << "addr of remote add(): 0x" << std::hex << (DWORD)mem->GetStartAddress() << std::dec << std::endl;

		Function* fn = new Function(target, mem->GetStartAddress(), STDCALL_CALLCONV, 2, DWORD_SIZE, NULL, NULL);

		DWORD ret3 = 0;
		
		fn->Call(args, &ret3);

		std::cout << "ret3: " << ret3 << std::endl;
	}
	
	MessageBox(NULL, TEXT("Stop"), TEXT(""), MB_OK);

	return 0;
}

