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
#include "NotifyHook.h"
#include "Process.h"
#include "MemoryRegion.h"
#include "Thread.h"

#include "assembly.h"

#include <algorithm>

namespace tcpie { namespace wincore {

	DWORD jmp_offs_notify(DWORD from, DWORD to, DWORD jmp_instr_size)
	{
		DWORD ret = to - from - jmp_instr_size;

		return ret;
	}

	DWORD jmp_offs_notify(void* from, void* to, DWORD jmp_instr_size)
	{
		return jmp_offs_notify((DWORD)from, (DWORD)to, jmp_instr_size);
	}

	std::map<std::wstring, NotifyHook*>* NotifyHook::hooks = new std::map<std::wstring, NotifyHook*>();

	NotifyHook::~NotifyHook()
	{
		delete this->pre_detours;
		delete this->post_detours;
		delete this->name;
		delete this->unhooked_function;
	}

	void NotifyHook::SetEnabled(bool enabled)
	{
		if (enabled)
		{
			this->Enable();
		}
		else
		{
			this->Disable();
		}
	}

	const void* NotifyHook::GetFunctionAddress() const
	{
		return this->function;
	}

	const void* NotifyHook::GetUnhookedFunctionAddress() const
	{
		return this->unhooked_function;
	}

	void NotifyHook::Enable()
	{
		this->enabled = true;

		Process::GetCurrentProcess()->WriteMemory(this->patch_code, const_cast<void*>(this->function));
	}

	void NotifyHook::Disable()
	{
		this->enabled = false;

		Process::GetCurrentProcess()->WriteMemory(this->old_code, const_cast<void*>(this->function));
	}

	bool NotifyHook::IsEnabled() const
	{
		return this->enabled;
	}

	const std::wstring* NotifyHook::GetName() const
	{
		return this->name;
	}

	NotifyDetour* NotifyHook::RegisterDetour(NotifyDetourCallback Callback)
	{
		NotifyDetour* ret = NULL;

		ret = new NotifyDetour(this, Callback);
		this->pre_detours->push_back(ret);

		return ret;
	}

	NotifyDetour* NotifyHook::RegisterDetour(const INotifyDetourClass* Callback)
	{
		NotifyDetour* ret = NULL;

		ret = new NotifyDetour(this, Callback);
		this->pre_detours->push_back(ret);

		return ret;
	}

	NotifyHook* NotifyHook::GetHookByName(const std::wstring* Name)
	{
		return NotifyHook::hooks->at(*Name);
	}

	NotifyHook* NotifyHook::CreateHook(const void* TargetFunctionAddress, std::wstring* Name, bool DoSafetyChecks)
	{
		return NULL;
	}
} }
