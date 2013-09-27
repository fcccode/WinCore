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

/// @file		Hook.h
/// @author		tcpie
/// @brief		Contains code relevant to the Hook class.

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

/// @brief Your callback function needs to be a DetourCallback.
typedef DetourRet (__stdcall *DetourCallback) (DetourArgs*);

/// @brief Contains the types of detours supported.
enum DetourType
{
	DETOUR_PRE = 0,		///< The detour callback is called before the original function is called.
	DETOUR_POST,		///< The detour callback is called after the original function is called.
};

/// @brief Contains the types of returns a detour callback can make.
///
/// The hooking engine will look at this return value and process the changes accordingly.
/// If your changes are not in accordance with the value you returned, your changes will be ignored!
/// Example: if you change the return value, but return DETOUR_NOCHANGE, the return value will be unchanged.
enum DetourRet
{
	DETOUR_NOCHANGE = 0,													///< The detour callback did not change anything.
	DETOUR_ARGCHANGED = 1,													///< The detour callback only changed the argument(s).
	DETOUR_RETCHANGED = 1 << 1,												///< The detour callback only changed the return value.
	DETOUR_ARGRETCHANGED = (int)DETOUR_ARGCHANGED | (int)DETOUR_RETCHANGED,	///< The detour callback changed the return value, as well as the argument(s).
	DETOUR_FNBLOCKED = (1 << 2) | (int)DETOUR_ARGRETCHANGED,				///< The call to the original function should be blocked. It is assumed the detour callback changed the return value and the argument(s).
};

/// @brief Any detour classes need to inherit from IDetourClass.
///
/// If one wishes to register a detour class, instead of a function, one should inherit from IDetourClass.
class __declspec(dllexport) IDetourClass
{
public:
	/// @brief Default virtual destructor, allowing you to specify your own.
	virtual ~IDetourClass() { }

	// NOTE: While Arguments is not marked const, you should NOT delete it!
	/// @brief Callback that is called when the hooked function is executed.
	/// @param Arguments		The arguments to the function.
	/// @return					You should return the appropriate DetourRet value.
	///
	/// Any changes made to Arguments will be propagated, provided the right value is returned. Note that
	/// Arguments can be altered, but should not be deleted!
	virtual DetourRet DetourCallback(DetourArgs* Arguments) = 0;
};

/// @brief Describes a detour (aka callback) to a function.
class __declspec(dllexport) Detour
{
	friend class Hook;

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

	DetourRet CallDetour(DetourArgs* Arguments) const;

public:
	/// @brief Default destructor.
	~Detour() { }

	///  @brief Gets the detour type.
	/// @return The detour type.
	DetourType GetDetourType() const { return this->detour_type; }

	/// @brief Gets the hook this detour belongs to.
	/// @return The hook this detour belongs to.
	const Hook* GetHook() const { return this->hook; }

	/// @brief Gets the detour callback class, if any.
	/// @return The detour callback class. If no instance was specified, NULL is returned.
	const IDetourClass* GetDetourClass() const { return this->detour_class; }

	/// @brief Gets the detour callback function.
	/// @return The detour callback function. If no function was specified, NULL is returned.
	const DetourCallback GetDetourCallback() const { return this->detour_function; }
};

/// @brief The arguments passed to detour callback functions / classes.
class __declspec(dllexport) DetourArgs
{
	friend class Hook;
private:
	void* instance;
	DetourRet previous_changes;
	const Detour* detour;

	DetourArgs(const Detour* TheDetour, void* Instance, const std::vector<void*>* Arguments, DWORD CustomReturnValue, DetourRet PreviousChanges)
	{
		this->instance = Instance;
		this->Arguments = new std::vector<void*>(*Arguments);
		this->previous_changes = PreviousChanges;
		this->CustomReturnValue = CustomReturnValue;
		this->detour = TheDetour;
	}

public:
	/// @brief Gets the instance of the hooked function.
	/// @return The instance of the hooked function. If none was provided or if the function is not a member function, NULL is returned.
	///
	/// "The instance of the function" means the instance of the class the hooked member function belongs to.
	void* GetInstance() const { return this->instance; }

	/// @brief Gets the type of detour.
	/// @return The detour type.
	DetourType GetDetourType() const { return this->detour->GetDetourType(); }

	/// @brief Gets any previous changes made to the DetourArgs.
	/// @return Any previous changes made to the DetourArgs.
	///
	/// This function is provided so one knows what effect code of somebody else has on the hooked function.
	DetourRet GetPreviousChanges() const { return this->previous_changes; }

	/// @brief Gets the relevant Detour.
	/// @return The relevant detour.
	const Detour* GetDetour() const { return this->detour; }

	/// @brief The custom return value.
	///
	/// Alter this value and return the appropriate DetourRet value to adjust the hooked function's return value.
	DWORD CustomReturnValue;
	
	/// @brief The arguments.
	///
	/// Alter these values and return the appropriate DetourRet value to adjust the arguments that will be passed to the hooked function.
	std::vector<void*>* Arguments;
};

/// @brief Provides hooking functionality.
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
	/// @brief Changes the enabled state of the hook.
	/// @param enabled If set to true, the hook is enabled. If set to false, the hook is disabled.
	void SetEnabled(bool enabled);

	/// @brief Enables the hook.
	///
	/// Enables the hook by patching in a JMP instruction at the start of the target function.
	void Enable();

	/// @brief Disables the hook.
	///
	/// Disables the hook by patching back the original code to the start of the target function.
	void Disable();

	/// @brief Gets whether the hooks is enabled.
	/// @return A value indicating whether the hook is enabled.
	bool IsEnabled() const;

	/// @brief Gets the name of the hook.
	/// @return The name of the hook.
	const std::wstring* GetName() const;

	/// @brief Gets the target function.
	/// @return The target function.
	const Function* GetFunction() const;

	/// @brief Gets the unhooked version of the function.
	/// @return The unhooked version of the function.
	///
	/// This function is provided so the unhooked version of the function can still be called. This is needed in some cases to avoid infinite loops.
	const Function* GetUnhookedFunction() const;

	/// @brief Registers a detour callback function for this hook.
	/// @param Callback		The callback function to call when the hooked function is called.
	/// @param Type			The detour type, indicating if the callback function should be called before or after the call to the unhooked function.
	/// @return				The registered detour.
	///
	/// You should not call the unhooked function yourself in your callback, the hook will take care of this.
	Detour* RegisterDetour(DetourCallback Callback, DetourType Type);

	/// @brief Registers a detour callback class for this hook.
	/// @param Callback		The callback class to call when the hooked function is called.
	/// @param Type			The detour type, indicating if the callback class should be called before or after the call to the unhooked function.
	/// @return				The registered detour.
	///
	/// You should not call the unhooked function yourself in your callback, the hook will take care of this.
	Detour* RegisterDetour(const IDetourClass* Callback, DetourType Type);

	/// @brief Gets a hook by its name.
	/// @param Name			The name to search by.
	/// @return				The found hook. If no hook was found, this function returns NULL.
	static Hook* GetHookByName(const std::wstring* Name);

	/// @brief Creates a hook.
	/// @param TargetFunction		The function to hook.
	/// @param Name					The name to give to the hook.
	/// @param DefaultReturnValue	The default value to return when the call to the function should be blocked.
	/// @param DoSafetyChecks		Whether or not the function address should be checked for correctness.
	/// @return						The created hook.
	///
	/// This function creates a hook by analyzing the target function and writing a few trampoline functions to memory. As such, this function
	/// should be regarded as *slow*!
	///
	/// The default return value provided is the return value used if a callback wants to block the call to the function, and does not provide
	/// a return value itself. Make sure this value is somewhat correct, a wrong value may cause crashes!
	///
	/// The safety checks performed by the function are pretty basic, and consist of a simple check, to see if the address of the TargetFunction
	/// appears correct. This is done by checking if the start address does not seem to fall within a function.
	static Hook* CreateHook(const Function* TargetFunction, std::wstring* Name, DWORD DefaultReturnValue = 0, bool DoSafetyChecks = true);	
};

} }

#endif
