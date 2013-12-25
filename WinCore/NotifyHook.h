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

/// @file		NotifyHook.h
/// @author		tcpie
/// @brief		Contains code relevant to the Hook class.

#ifndef _NOTIFYHOOK_H_
#define _NOTIFYHOOK_H_

#include <windows.h>
#include <string>
#include <vector>
#include <map>

namespace tcpie { namespace wincore {

class MemoryRegion;

/// @brief Your callback function needs to be a NotifyDetourCallback.
typedef void (__stdcall *NotifyDetourCallback) (void* HookedFunctionAddress);

/// @brief Any detour classes need to inherit from INotifyDetourClass.
///
/// If one wishes to register a detour class, instead of a function, one should inherit from INotifyDetourClass.
class __declspec(dllexport) INotifyDetourClass
{
public:
	/// @brief Default virtual destructor, allowing you to specify your own.
	virtual ~INotifyDetourClass() { }

	// NOTE: While Arguments is not marked const, you should NOT delete it!
	/// @brief Callback that is called when the hooked function is executed.
	/// @param HookedFunctionAddress		The address of the hooked function.
	/// @return					void
	///
	virtual void NotifyDetourCallback(void* HookedFunctionAddress) = 0;
};

/// @brief Describes a detour (aka callback) to a function.
class __declspec(dllexport) NotifyDetour
{
	friend class NotifyHook;

private:
	const NotifyHook* hook;
	NotifyDetourCallback detour_function;
	const INotifyDetourClass* detour_class;
	bool enabled;

	NotifyDetour(const NotifyHook* FnHook, NotifyDetourCallback DetourFunction)
	{
		this->enabled = true;
		this->hook = FnHook;
		this->detour_function = DetourFunction;
		this->detour_class = NULL;
	}

	NotifyDetour(const NotifyHook* FnHook, const INotifyDetourClass* DetourClass)
	{
		this->enabled = true;
		this->hook = FnHook;
		this->detour_class = DetourClass;
		this->detour_function = NULL;
	}

	void CallDetour(const void* FunctionAddress) const;

public:
	/// @brief Default destructor.
	~NotifyDetour() { }

	bool IsEnabled() { return this->enabled; }

	void Enable() { this->enabled = true; }

	void Disable() { this->enabled = false; }

	void SetEnabled(bool value) { this->enabled = value; }

	/// @brief Gets the hook this detour belongs to.
	/// @return The hook this detour belongs to.
	const NotifyHook* GetHook() const { return this->hook; }

	/// @brief Gets the detour callback class, if any.
	/// @return The detour callback class. If no instance was specified, NULL is returned.
	const INotifyDetourClass* GetDetourClass() const { return this->detour_class; }

	/// @brief Gets the detour callback function.
	/// @return The detour callback function. If no function was specified, NULL is returned.
	const NotifyDetourCallback GetDetourCallback() const { return this->detour_function; }
};

/// @brief Provides hooking functionality.
class __declspec(dllexport) NotifyHook
{
private:
	std::wstring* name;
	const void* function;
	void* unhooked_function;

	MemoryRegion* old_code;
	MemoryRegion* patch_code;

	DWORD default_returnvalue;
	bool enabled;
	std::vector<NotifyDetour*>* pre_detours;
	std::vector<NotifyDetour*>* post_detours;

	static std::map<std::wstring, NotifyHook*>* hooks;

	NotifyHook(const void* TargetFunction, std::wstring* Name, void* UnhookedFunction, MemoryRegion* OldCode, MemoryRegion* PatchCode);
	~NotifyHook();

	DWORD detour();
	static void __stdcall global_detour(NotifyHook* hook);

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

	/// @brief Gets the address of the target function.
	/// @return The address of the target function.
	const void* GetFunctionAddress() const;

	/// @brief Gets the address of the unhooked version of the function.
	/// @return The address of the unhooked version of the function.
	const void* GetUnhookedFunctionAddress() const;

	/// @brief Registers a detour callback function for this hook.
	/// @param Callback		The callback function to call when the hooked function is called.
	/// @return				The registered detour.
	///
	/// You should not call the unhooked function yourself in your callback, the hook will take care of this.
	NotifyDetour* RegisterDetour(NotifyDetourCallback Callback);

	/// @brief Registers a detour callback class for this hook.
	/// @param Callback		The callback class to call when the hooked function is called.
	/// @param Type			The detour type, indicating if the callback class should be called before or after the call to the unhooked function.
	/// @return				The registered detour.
	///
	/// You should not call the unhooked function yourself in your callback, the hook will take care of this.
	NotifyDetour* RegisterDetour(const INotifyDetourClass* Callback);

	/// @brief Gets a hook by its name.
	/// @param Name			The name to search by.
	/// @return				The found hook. If no hook was found, this function returns NULL.
	static NotifyHook* GetHookByName(const std::wstring* Name);

	/// @brief Creates a hook.
	/// @param TargetFunctionAddress		The function to hook.
	/// @param Name							The name to give to the hook.
	/// @param DoSafetyChecks				Whether or not the function address should be checked for correctness.
	/// @return								The created hook.
	///
	/// This function creates a hook by analyzing the target function and writing a few trampoline functions to memory. As such, this function
	/// should be regarded as *slow*!
	///
	/// The default return value provided is the return value used if a callback wants to block the call to the function, and does not provide
	/// a return value itself. Make sure this value is somewhat correct, a wrong value may cause crashes!
	///
	/// The safety checks performed by the function are pretty basic, and consist of a simple check, to see if the address of the TargetFunction
	/// appears correct. This is done by checking if the start address does not seem to fall within a function.
	static NotifyHook* CreateHook(void* TargetFunctionAddress, std::wstring* Name, bool DoSafetyChecks = true);
};

} }

#endif
