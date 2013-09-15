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

#ifndef _HOOK_H_
#define _HOOK_H_

#include <string>
#include <vector>
#include <map>

#include "Function.h"

namespace tcpie { namespace wincore {

class Detour;
class Hook;
class DetourArgs;
class IDetourClass;

enum DetourRet;

typedef DetourRet (__stdcall *DetourCallback) (DetourArgs*);

enum DetourType
{
	DETOUR_PRE = 0,
	DETOUR_POST,
};

enum DetourRet
{
	DETOUR_NOCHANGE = 0,
	DETOUR_ARGCHANGED = 1,
	DETOUR_RETCHANGED = 1 << 1,
	DETOUR_ARGRETCHANGED = (int)DETOUR_ARGCHANGED | (int)DETOUR_RETCHANGED,
	DETOUR_FNBLOCKED = (1 << 2) | (int)DETOUR_ARGRETCHANGED,
};

class __declspec(dllexport) IDetourClass
{
public:
	virtual ~IDetourClass() { }

	// NOTE: While Arguments is not marked const, you should NOT delete it!
	virtual DetourRet DetourCallback(DetourArgs* Arguments) = 0;
};

class __declspec(dllexport) Detour
{
private:
	const Hook* hook;
	DetourCallback detour_function;
	const IDetourClass* detour_class;
	DetourType detour_type;

	Detour(const Hook* FnHook, DetourCallback DetourFunction, DetourType Type)
	{
		this->hook = FnHook;
		this->detour_function = DetourFunction;
		this->detour_class = NULL;
		this->detour_type = Type;
	}

	Detour(const Hook* FnHook, const IDetourClass* DetourClass, DetourType Type)
	{
		this->hook = FnHook;
		this->detour_class = DetourClass;
		this->detour_function = NULL;
		this->detour_type = Type;
	}

protected:
	DetourRet CallDetour(DetourArgs* Arguments) const;

public:
	~Detour() { }

	DetourType GetDetourType() const { return this->detour_type; }
	const Hook* GetHook() const { return this->hook; }
	const IDetourClass* GetDetourClass() const { return this->detour_class; }
	const DetourCallback GetDetourCallback() const { return this->detour_function; }

	friend class Hook;
};

class __declspec(dllexport) DetourArgs
{
	friend class Hook;
private:
	void* instance;
	DetourRet previous_changes;
	const Detour* detour;

protected:
	DetourArgs(const Detour* TheDetour, void* Instance, const std::vector<void*>* Arguments, DWORD CustomReturnValue, DetourRet PreviousChanges)
	{
		this->instance = Instance;
		this->Arguments = new std::vector<void*>(*Arguments);
		this->previous_changes = PreviousChanges;
		this->CustomReturnValue = CustomReturnValue;
		this->detour = TheDetour;
	}

public:
	void* GetInstance() const { return this->instance; }
	DetourType GetDetourType() const { return this->detour->GetDetourType(); }
	DetourRet GetPreviousChanges() const { return this->previous_changes; }
	const Detour* GetDetour() const { return this->detour; }

	DWORD CustomReturnValue;
	
	std::vector<void*>* Arguments;
};

class __declspec(dllexport) Hook
{
private:
	std::wstring* name;
	const Function* function;
	Function* unhooked_function;

	MemoryRegion* old_code;
	MemoryRegion* patch_code;

	DWORD default_returnvalue;
	bool enabled;
	std::vector<Detour*>* pre_detours;
	std::vector<Detour*>* post_detours;

	static std::map<std::wstring, Hook*>* hooks;

	Hook(const Function* TargetFunction, std::wstring* Name, Function* UnhookedFunction, DWORD DefaultReturnValue, MemoryRegion* OldCode, MemoryRegion* PatchCode);
	~Hook();

	DWORD detour(void* instance, std::vector<void*>* args);
	static DWORD __cdecl global_detour(Hook* hook, ...);

public:	
	void SetEnabled(bool enabled);
	void Enable();
	void Disable();

	bool IsEnabled() const;
	const std::wstring* GetName() const;

	const Function* GetFunction() const;
	const Function* GetUnhookedFunction() const;

	Detour* RegisterDetour(DetourCallback Callback, DetourType Type);
	Detour* RegisterDetour(const IDetourClass* Callback, DetourType Type);

	static Hook* GetHookByName(const std::wstring* Name);
	static Hook* CreateHook(const Function* TargetFunction, std::wstring* Name, DWORD DefaultReturnValue = 0, bool DoSafetyChecks = true);	
};

} }

#endif
