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
#include "HookTest.h"

#include "..\WinCore\Function.h"
#include "..\WinCore\Hook.h"

#include <iostream>

using namespace std;
using namespace tcpie::wincore;

class test_class
{
public:
	__declspec(noinline) int add(int a, float b, int* ret_buffer)
	{
		int ret = a + (int)b;

		*ret_buffer = ret;

		return ret;
	}

	__declspec(noinline) int add_end(int a, float b, int* ret_buffer)
	{
		return 0;
	}
};

__declspec(noinline) int __cdecl add(int a, float b, int* ret_buffer)
{
	int ret = a + (int)b;

	*ret_buffer = ret;

	return ret;
}

__declspec(noinline) void add_end() { }

__declspec(noinline) int __stdcall add_stdcall(int a, float b, int* ret_buffer)
{
	int ret = a + (int)b;

	*ret_buffer = ret;

	return ret;
}

__declspec(noinline) void add_stdcall_end() { }

__declspec(noinline) DetourRet __stdcall add_change_arg(DetourArgs* args)
{
	args->Arguments->at(0) = (void*)4;

	return DETOUR_ARGCHANGED;
}

__declspec(noinline) DetourRet __stdcall add_change_ret(DetourArgs* args)
{
	args->CustomReturnValue = 500;

	return DETOUR_RETCHANGED;
}

__declspec(noinline) DetourRet __stdcall add_block_fn(DetourArgs* args)
{
	args->CustomReturnValue = 600;

	return DETOUR_FNBLOCKED;
}

class detour_class : public tcpie::wincore::IDetourClass
{
public:
	__declspec(noinline) virtual DetourRet DetourCallback(DetourArgs* Arguments) override
	{
		cout << "\tClass detour working? 1" << endl;

		return DETOUR_NOCHANGE;
	}
};

typedef int (test_class::*test_memfnptr)(int, float, int*);

bool hook_test()
{
	cout << "Hook test:" << endl;

	for (int i = CDECL_CALLCONV; i < 3; i ++)
	{
		void* fn = NULL;
		void* fn_end = NULL;
		test_class instance;

		if (i == CDECL_CALLCONV)
		{
			cout << "\t--- CDECL" << endl;
			fn = (void*)&add;
			fn_end = (void*)&add_end;
		}
		else if (i == STDCALL_CALLCONV)
		{
			cout << "\t--- STDCALL" << endl;
			fn = (void*)&add_stdcall;
			fn_end = (void*)&add_stdcall_end;
		}
		else if (i == THISCALL_CALLCONV)
		{
			cout << "\t--- THISCALL" << endl;
			test_memfnptr fn_ptr = &test_class::add;
			test_memfnptr fn_ptr_end = &test_class::add_end;

			__asm
			{
				push eax;
				mov eax, fn_ptr;
				mov fn, eax;
				pop eax;
			}

			__asm
			{
				push eax;
				mov eax, fn_ptr_end;
				mov fn_end, eax;
				pop eax;
			}
		}

#ifdef _DEBUG
		vector<BYTE>* signature = new std::vector<BYTE>((BYTE*)fn, (BYTE*)fn_end);
#else
		vector<BYTE>* signature = new std::vector<BYTE>((BYTE*)fn, (BYTE*)((DWORD)fn + 0x150));
#endif
		vector<char>* mask = new std::vector<char>(signature->size(), 'x');


		Function* target = Function::FindFunction(signature,
			mask,
			(CallingConvention)i,	// Calling convention
			3,						// Number of arguments
			DWORD_SIZE);			// Return type

		cout << "\tFindFunction() succeeded? " << (bool)(target->GetAddress() == fn) << endl;

		Hook* cdecl_hook = Hook::CreateHook(target, new wstring(L"add"));

		cout << "\tCreateHook() succeeded? " << (bool)(cdecl_hook != NULL) << endl;

		Detour* cdecl_change_arg_detour = cdecl_hook->RegisterDetour(add_change_arg, DETOUR_PRE);

		cout << "\tRegisterDetour(fn) succeeded? " << (bool)(cdecl_change_arg_detour != NULL) << endl;

		Detour* cdecl_class_detour = cdecl_hook->RegisterDetour(new detour_class(), DETOUR_PRE);

		cout << "\tRegisterDetour(class) succeeded? " << (bool)(cdecl_class_detour != NULL) << endl;

		Detour* cdecl_change_ret_detour = cdecl_hook->RegisterDetour(add_change_ret, DETOUR_POST);

		Detour* cdecl_blockfn = cdecl_hook->RegisterDetour(add_block_fn, DETOUR_PRE);

		cdecl_blockfn->Disable();
		cdecl_hook->Enable();
		cdecl_change_ret_detour->Disable();

		int ret_buffer = 0;
		int ret = 0;

		switch (i)
		{
		case CDECL_CALLCONV:
			ret = add(2, 5, &ret_buffer);
			break;

		case STDCALL_CALLCONV:
			ret = add_stdcall(2, 5, &ret_buffer);
			break;

		case THISCALL_CALLCONV:
			ret = instance.add(2, 5, &ret_buffer);
			break;
		}

		cdecl_class_detour->Disable();

		cout << "\tChange arg succeeded? " << (bool)(ret == 9) << endl;
		cdecl_change_ret_detour->Enable();
		cdecl_change_arg_detour->Disable();

		switch (i)
		{
		case CDECL_CALLCONV:
			ret = add(2, 5, &ret_buffer);
			break;

		case STDCALL_CALLCONV:
			ret = add_stdcall(2, 5, &ret_buffer);
			break;

		case THISCALL_CALLCONV:
			ret = instance.add(2, 5, &ret_buffer);
			break;
		}

		cout << "\tChange ret succeeded? " << (bool)(ret == 500) << endl;
		cdecl_change_ret_detour->Disable();

		ret_buffer = 0;
		cdecl_blockfn->Enable();

		switch (i)
		{
		case CDECL_CALLCONV:
			ret = add(2, 5, &ret_buffer);
			break;

		case STDCALL_CALLCONV:
			ret = add_stdcall(2, 5, &ret_buffer);
			break;

		case THISCALL_CALLCONV:
			ret = instance.add(2, 5, &ret_buffer);
			break;
		}

		cout << "\tBlock function succeeded? " << (bool)(ret_buffer == 0 && ret == 600) << endl; 
	}

	cout << "Hook test done. " << endl;

	return true;
}