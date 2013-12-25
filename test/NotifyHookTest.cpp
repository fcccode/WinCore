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
#include "NotifyHookTest.h"

#include "..\WinCore\NotifyHook.h"

#include <iostream>

using namespace std;
using namespace tcpie::wincore;

__declspec(noinline) int printLOL(int val)
{
	return val - 2;
}

__declspec(noinline) void __stdcall notify_callback(NotifyDetour* Detour)
{
	cout << "\tNotify callback called? 1    (fn 0x" << hex << Detour->GetHook()->GetFunctionAddress() << dec << ")" << endl;
}

class notify_callback_class : public tcpie::wincore::INotifyDetourClass
{
public:
	notify_callback_class()
	{

	}

	__declspec(noinline) virtual void NotifyDetourCallback(NotifyDetour* Detour) override
	{
		cout << "\tNotify class callback called? 1    (fn 0x" << hex << Detour->GetHook()->GetFunctionAddress() << dec << ")" << endl;
	}
};

bool notifyhook_test()
{
	cout << "NotifyHook test:" << endl;

	NotifyHook* n_hook = NotifyHook::CreateHook(&printLOL, new wstring(L"printLOL"));

	cout << "\tCreateHook succeeded? " << (bool)(n_hook != NULL) << endl;

	NotifyDetour* n_detour1 = n_hook->RegisterDetour(notify_callback);

	cout << "\tRegisterDetour(fn) succeeded? " << (bool)(n_detour1 != NULL) << endl;

	NotifyDetour* n_detour2 = n_hook->RegisterDetour(new notify_callback_class());

	cout << "\tRegisterDetour(class) succeeded? " << (bool)(n_detour2 != NULL) << endl;

	n_hook->Enable();

	int ret = printLOL(10);

	return true;
}