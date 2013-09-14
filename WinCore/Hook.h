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
	virtual DetourRet DetourCallback(DetourArgs* Arguments) = 0;
};

class __declspec(dllexport) Detour
{
private:
	Hook* hook;
	DetourCallback detour_function;
	IDetourClass* detour_class;
	DetourType detour_type;

	Detour(Hook* FnHook, DetourCallback DetourFunction, DetourType Type)
	{
		this->hook = FnHook;
		this->detour_function = DetourFunction;
		this->detour_class = NULL;
		this->detour_type = Type;
	}

	Detour(Hook* FnHook, IDetourClass* DetourClass, DetourType Type)
	{
		this->hook = FnHook;
		this->detour_class = DetourClass;
		this->detour_function = NULL;
		this->detour_type = Type;
	}

protected:
	DetourRet CallDetour(DetourArgs* Arguments);

public:
	DetourType GetDetourType() { return this->detour_type; }
	Hook* GetHook() { return this->hook; }
	IDetourClass* GetDetourClass() { return this->detour_class; }
	DetourCallback GetDetourCallback() { return this->detour_function; }

	friend class Hook;
};

class __declspec(dllexport) DetourArgs
{
	friend class Hook;
private:
	void* instance;
	DetourRet previous_changes;
	Detour* detour;

protected:
	DetourArgs(Detour* TheDetour, void* Instance, std::vector<void*>* Arguments, DWORD CustomReturnValue, DetourRet PreviousChanges)
	{
		this->instance = Instance;
		this->Arguments = Arguments;
		this->previous_changes = PreviousChanges;
		this->CustomReturnValue = CustomReturnValue;
		this->detour = TheDetour;
	}

public:
	void* GetInstance() { return this->instance; }
	DetourType GetDetourType() { return this->detour->GetDetourType(); }
	DetourRet GetPreviousChanges() { return this->previous_changes; }
	Detour* GetDetour() { return this->detour; }

	DWORD CustomReturnValue;
	
	std::vector<void*>* Arguments;
};

class __declspec(dllexport) Hook
{
private:
	std::wstring* name;
	Function* function;
	Function* unhooked_function;

	MemoryRegion* old_code;
	MemoryRegion* patch_code;

	DWORD default_returnvalue;
	bool enabled;
	std::vector<Detour*>* pre_detours;
	std::vector<Detour*>* post_detours;

	static std::map<std::wstring, Hook*>* hooks;

	Hook(Function* TargetFunction, std::wstring* Name, Function* UnhookedFunction, DWORD DefaultReturnValue, MemoryRegion* OldCode, MemoryRegion* PatchCode);
	~Hook();

	DWORD detour(void* instance, std::vector<void*>* args);
	static DWORD __cdecl global_detour(Hook* hook, ...);

public:
	
	void SetEnabled(bool enabled);
	void Enable();
	void Disable();

	bool IsEnabled();
	const std::wstring* GetName();

	Function* GetFunction();
	Function* GetUnhookedFunction();

	Detour* RegisterDetour(DetourCallback Callback, DetourType Type);
	Detour* RegisterDetour(IDetourClass* Callback, DetourType Type);

	static Hook* GetHookByName(std::wstring* Name);
	static Hook* CreateHook(Function* TargetFunction, std::wstring* Name, DWORD DefaultReturnValue, bool DoSafetyChecks = true);	
};

} }

#endif
