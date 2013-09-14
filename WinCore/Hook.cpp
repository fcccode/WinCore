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
#include "Hook.h"
#include "Process.h"
#include "MemoryRegion.h"
#include "Thread.h"

#include "assembly.h"

#define PTRADD(ptr, diff) (void*)((DWORD)ptr + (DWORD)diff)

namespace tcpie { namespace wincore {

DetourRet Detour::CallDetour(DetourArgs* Arguments) const
{
	if (this->detour_class != NULL)
	{
		return const_cast<IDetourClass*>(this->detour_class)->DetourCallback(Arguments);
	}

	if (this->detour_function != NULL)
	{
		return this->detour_function(Arguments);
	}

	return DETOUR_NOCHANGE;
}

DWORD jmp_offs(DWORD from, DWORD to, DWORD jmp_instr_size)
{
	DWORD ret = to - from - jmp_instr_size;

	return ret;
}

DWORD jmp_offs(void* from, void* to, DWORD jmp_instr_size)
{
	return jmp_offs((DWORD)from, (DWORD)to, jmp_instr_size);
}

std::map<std::wstring, Hook*>* Hook::hooks = new std::map<std::wstring, Hook*>();

DWORD Hook::detour(void* instance, std::vector<void*>* args)
{
	Thread* t = Thread::GetCurrentThread();

	DetourRet final_ret = DETOUR_NOCHANGE;
	DWORD custom_returnvalue = this->default_returnvalue;
	std::vector<void*>* custom_args = new std::vector<void*>();
	custom_args->assign(args->begin(), args->end());

	for (size_t i = 0; i < this->pre_detours->size(); i++)
	{
		std::vector<void*>* temp_args = new std::vector<void*>();
		temp_args->assign(args->begin(), args->end());
		DWORD temp_custom_ret = this->default_returnvalue;
		DetourArgs* d_args = new DetourArgs(this->pre_detours->at(i), instance, temp_args, temp_custom_ret, final_ret);

		DetourRet temp_ret = this->pre_detours->at(i)->CallDetour(d_args);

		switch (temp_ret)
		{
		case DETOUR_NOCHANGE:
			break;

		case DETOUR_FNBLOCKED:
			final_ret = DETOUR_FNBLOCKED;

		case DETOUR_ARGRETCHANGED:
			if ((int)final_ret < (int)DETOUR_ARGRETCHANGED)
			{
				final_ret = DETOUR_ARGRETCHANGED;
			}

		case DETOUR_RETCHANGED:
			if (final_ret == DETOUR_NOCHANGE)
			{
				final_ret = DETOUR_RETCHANGED;
			}

			custom_returnvalue = d_args->CustomReturnValue;

			if (temp_ret == DETOUR_RETCHANGED || temp_ret == DETOUR_FNBLOCKED)
			{
				break;
			}

		case DETOUR_ARGCHANGED:
			if (final_ret == DETOUR_NOCHANGE)
			{
				final_ret = DETOUR_ARGCHANGED;
			}

			custom_args->clear();

			for (size_t j = 0; j < args->size() && j < temp_args->size() && j < d_args->Arguments->size(); j++)
			{
				custom_args->push_back(d_args->Arguments->at(j));
			}

			break;
		}

		delete d_args;
		delete temp_args;
	}

	DWORD ret = custom_returnvalue;

	if (final_ret != DETOUR_FNBLOCKED)
	{
		this->unhooked_function->Call(custom_args, t, &ret, instance);
	}

	if ((int)final_ret & (int)DETOUR_RETCHANGED)
	{
		ret = custom_returnvalue;
	}

	for (size_t i = 0; i < this->post_detours->size(); i++)
	{
		std::vector<void*>* temp_args = new std::vector<void*>();
		temp_args->assign(custom_args->begin(), custom_args->end());

		DWORD temp_custom_ret = ret;

		DetourArgs* d_args = new DetourArgs(this->post_detours->at(i), instance, temp_args, temp_custom_ret, final_ret);

		DetourRet temp_ret = this->pre_detours->at(i)->CallDetour(d_args);

		if (temp_ret == DETOUR_RETCHANGED || temp_ret == DETOUR_ARGRETCHANGED)
		{
			ret = d_args->CustomReturnValue;
		}

		delete d_args;
		delete temp_args;
	}

	delete t;
	delete custom_args;

	return ret;
}

DWORD Hook::global_detour(Hook* hook, ...)
{
	try
	{
		const std::wstring* hook_name = hook->GetName();
	}
	catch (...)
	{
		// Our hook is deleted! Let's just return...
		return 0;
	}

	void* instance = NULL;
	std::vector<void*> args;

	if (hook->GetFunction()->GetCallingConvention() == THISCALL_CALLCONV)
	{
		instance = *(void**)((DWORD)(&hook) + (DWORD)sizeof(void*));
		DWORD** args_start = (DWORD**)((DWORD)(&hook) + (DWORD)(sizeof(void*) * 2));
		args = std::vector<void*>(args_start, (DWORD**)((DWORD)args_start + (DWORD)hook->GetFunction()->GetArgCount() * sizeof(void*)));
	}
	else
	{
		DWORD** args_start = (DWORD**)((DWORD)(&hook) + (DWORD)(sizeof(void*)));
		args = std::vector<void*>(args_start, (DWORD**)((DWORD)args_start + (DWORD)hook->GetFunction()->GetArgCount() * sizeof(void*)));
	}

	return hook->detour(instance, &args);
}

Hook::Hook(const Function* TargetFunction, std::wstring* Name, Function* UnhookedFunction, DWORD DefaultReturnValue, MemoryRegion* OldCode, MemoryRegion* PatchCode)
{
	this->name = Name;
	this->function = TargetFunction;
	this->pre_detours = new std::vector<Detour*>();
	this->post_detours = new std::vector<Detour*>();
	this->enabled = false;
	this->unhooked_function = UnhookedFunction;
	this->old_code = OldCode;
	this->patch_code = PatchCode;
	this->default_returnvalue = DefaultReturnValue;
}

Hook::~Hook()
{
	this->function->GetProcess()->FreeMemory(this->old_code);
	this->function->GetProcess()->FreeMemory(this->patch_code);

	delete this->pre_detours;
	delete this->post_detours;
	delete this->name;
	delete this->unhooked_function;
}

void Hook::SetEnabled(bool enabled)
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

const Function* Hook::GetFunction() const
{
	return this->function;
}

const Function* Hook::GetUnhookedFunction() const
{
	return this->unhooked_function;
}

void Hook::Enable()
{
	this->enabled = true;

	Process::GetCurrentProcess()->WriteMemory(this->patch_code, this->function->GetAddress());
}

void Hook::Disable()
{
	this->enabled = false;

	Process::GetCurrentProcess()->WriteMemory(this->old_code, this->function->GetAddress());
}

bool Hook::IsEnabled() const
{
	return this->enabled;
}

const std::wstring* Hook::GetName() const
{
	return this->name;
}

Detour* Hook::RegisterDetour(DetourCallback Callback, DetourType Type)
{
	Detour* ret = NULL;

	switch (Type)
	{
	case DETOUR_PRE:
		ret = new Detour(this, Callback, Type);
		this->pre_detours->push_back(ret);

		return ret;

	case DETOUR_POST:
		ret = new Detour(this, Callback, Type);
		this->post_detours->push_back(ret);

		return ret;

	default:
		return NULL;
	}
}

Detour* Hook::RegisterDetour(const IDetourClass* Callback, DetourType Type)
{
	Detour* ret = NULL;

	switch (Type)
	{
	case DETOUR_PRE:
		ret = new Detour(this, Callback, Type);
		this->pre_detours->push_back(ret);

		return ret;

	case DETOUR_POST:
		ret = new Detour(this, Callback, Type);
		this->post_detours->push_back(ret);

		return ret;

	default:
		return NULL;
	}
}

Hook* Hook::GetHookByName(const std::wstring* Name)
{
	return Hook::hooks->at(*Name);
}

Hook* Hook::CreateHook(const Function* TargetFunction, std::wstring* Name, DWORD DefaultReturnValue, bool DoSafetyChecks)
{
	PatchInfo* patchinfo = const_cast<Function*>(TargetFunction)->FindPatchInfo();

	if (DoSafetyChecks)
	{
		if (*(BYTE*)((DWORD)TargetFunction->GetAddress() - 1) != 0x90 && 
			*(BYTE*)((DWORD)TargetFunction->GetAddress() - 1) != 0xCC &&
			(DWORD)TargetFunction->GetAddress() % 4 != 0)
		{
			// Input is most likely faulty, we are probably not at a real functions start address, but somewhere *inside* a function!!
			return NULL;
		}
	}

	BYTE* pre_code = NULL;
	size_t pre_code_length;

	if (TargetFunction->GetCallingConvention() == THISCALL_CALLCONV)
	{
		pre_code = (BYTE*)ASM(push_ecx_and_extra_arg);
		pre_code_length = ASM_SIZE(push_ecx_and_extra_arg);
	}
	else if (TargetFunction->GetCallingConvention() == CDECL_CALLCONV || TargetFunction->GetCallingConvention() == STDCALL_CALLCONV)
	{
		pre_code = (BYTE*)ASM(push_extra_arg);
		pre_code_length = ASM_SIZE(push_extra_arg);
	}
	else
	{
		return NULL; // Unsupported calling convention
	}

	MemoryRegion* pre_code_region = Process::GetCurrentProcess()->WriteMemory(pre_code, pre_code_length);

	DWORD wildcard = 0x1337;

	// Patch in required info
	int num_args_to_clean = 0; 
	int num_fn_args = 0;

	switch (TargetFunction->GetCallingConvention())
	{
	case CDECL_CALLCONV:
		num_args_to_clean = 1;
		num_fn_args = TargetFunction->GetArgCount() + 1;
		break;

	case THISCALL_CALLCONV:
		num_args_to_clean = 1;
		num_fn_args = 1;

	case STDCALL_CALLCONV:
		num_args_to_clean += TargetFunction->GetArgCount() + 1;
		num_fn_args += TargetFunction->GetArgCount() + 1;
		break;

	default:
		num_args_to_clean = TargetFunction->GetArgCount() + 1;
	}

	Function* global_detour_fn = new Function(&Hook::global_detour,
											  CDECL_CALLCONV,
											  num_fn_args,
											  DWORD_SIZE);

	Function* global_detour_fn_wrapped = global_detour_fn->CreateStdcallWrapper(num_args_to_clean);

	MemoryRegion* unhooked_fn_region = Process::GetCurrentProcess()->WriteMemory(TargetFunction->GetAddress(), patchinfo->GetNumOpcodes() + ASM_SIZE(jmp_far));

	DWORD jmp_offset_unhooked_fn_to_fn = jmp_offs((DWORD)unhooked_fn_region->GetStartAddress() + patchinfo->GetNumOpcodes(), (DWORD)TargetFunction->GetAddress() + patchinfo->GetNumOpcodes(), ASM_SIZE(jmp_far));

	MemoryRegion* unhooked_fn_jmp_region = Process::GetCurrentProcess()->WriteMemory(ASM(jmp_far), ASM_SIZE(jmp_far), PTRADD(unhooked_fn_region->GetStartAddress(), patchinfo->GetNumOpcodes()));
	unhooked_fn_jmp_region->ReplaceFirstOccurence(wildcard, jmp_offset_unhooked_fn_to_fn);

	// Fix jmp
	if (patchinfo->GetJumpInstructionAddress() != NULL)
	{
		DWORD original_jmp_offset = *(DWORD*)((BYTE*)patchinfo->GetJumpInstructionAddress() + patchinfo->GetJumpInstructionLength() - sizeof(void*));
		DWORD original_jmp_target = original_jmp_offset + (DWORD)patchinfo->GetJumpInstructionAddress() + patchinfo->GetJumpInstructionLength();

		DWORD jmp_instruction_offset_from_fn_start = (DWORD)patchinfo->GetJumpInstructionAddress() - (DWORD)TargetFunction->GetAddress();
		DWORD new_offset = original_jmp_target - ((DWORD)unhooked_fn_region->GetStartAddress() + jmp_instruction_offset_from_fn_start) - patchinfo->GetJumpInstructionLength();
		*(DWORD*)((DWORD)unhooked_fn_region->GetStartAddress() + jmp_instruction_offset_from_fn_start + patchinfo->GetJumpInstructionLength() - sizeof(void*)) = new_offset;
	}

	DWORD jmp_offset_hooked_fn_to_pre_code = jmp_offs(TargetFunction->GetAddress(), pre_code_region->GetStartAddress(), ASM_SIZE(jmp_far));

	MemoryRegion* old_code = Process::GetCurrentProcess()->WriteMemory(TargetFunction->GetAddress(), patchinfo->GetNumOpcodes());

	// Patch in the hook
	MemoryRegion* patch = Process::GetCurrentProcess()->WriteMemory(ASM(jmp_far), ASM_SIZE(jmp_far));
	void* addr = patch->ReplaceFirstOccurence(wildcard, jmp_offset_hooked_fn_to_pre_code);


	// We provide the user with a function wrapped to stdcall, otherwise the provide function pointer is not of much use...
	void* stdcall_wrapped_function = NULL;

	Function* unhooked_fn = new Function(unhooked_fn_region->GetStartAddress(), 
											TargetFunction->GetCallingConvention(),
											TargetFunction->GetArgCount(),
											TargetFunction->GetReturnType());
	// Function* stdcall_wrapped_unhooked_fn = unhooked_fn->CreateStdcallWrapper();

	Hook* ret = new Hook(TargetFunction, Name, unhooked_fn, DefaultReturnValue, old_code, patch);

	pre_code_region->ReplaceFirstOccurence(wildcard, (DWORD)ret);
	pre_code_region->ReplaceFirstOccurence(wildcard, (DWORD)NULL);
	pre_code_region->ReplaceFirstOccurence(wildcard, (DWORD)global_detour_fn_wrapped->GetAddress());

	delete global_detour_fn;

	return ret;
}

} }
