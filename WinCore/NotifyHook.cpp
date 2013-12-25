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
#include "Function.h"

#include "assembly.h"

#include <algorithm>

#include <iostream>

#define PTRADD(ptr, diff) (void*)((DWORD)ptr + (DWORD)diff)

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

	void NotifyDetour::CallDetour(const void* FunctionAddress) const
	{
		if (!this->enabled)
		{
			return;
		}

		if (this->detour_class != NULL)
		{
			const_cast<INotifyDetourClass*>(this->detour_class)->NotifyDetourCallback(const_cast<NotifyDetour*>(this));
		}

		if (this->detour_function != NULL)
		{
			this->detour_function(const_cast<NotifyDetour*>(this));
		}
	}

	DWORD NotifyHook::detour()
	{
		for (size_t i = 0; i < this->pre_detours->size(); i++)
		{
			this->pre_detours->at(i)->CallDetour(this->function);
		}

		for (size_t i = 0; i < this->post_detours->size(); i++)
		{
			this->post_detours->at(i)->CallDetour(this->function);
		}

		return 0;
	}

	void NotifyHook::global_detour(NotifyHook* hook)
	{
		hook->detour();
	}

	NotifyHook::NotifyHook(const void* TargetFunction, std::wstring* Name, void* UnhookedFunction, MemoryRegion* OldCode, MemoryRegion* PatchCode)
	{
		this->function = TargetFunction;
		this->name = Name;
		this->unhooked_function = UnhookedFunction;
		this->old_code = OldCode;
		this->patch_code = PatchCode;
		this->pre_detours = new std::vector<NotifyDetour*>();
		this->post_detours = new std::vector<NotifyDetour*>();
		this->enabled = false;
	}

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

	NotifyHook* NotifyHook::CreateHook(void* TargetFunctionAddress, std::wstring* Name, bool DoSafetyChecks)
	{		
		Function* TargetFunction = new Function(TargetFunctionAddress, CDECL_CALLCONV, 0, NONE);
		PatchInfo* patchinfo = TargetFunction->FindPatchInfo();

		if (DoSafetyChecks)
		{
			if (*(BYTE*)((DWORD)TargetFunction->GetAddress() - 1) != 0x90 &&		// NOP 
				*(BYTE*)((DWORD)TargetFunction->GetAddress() - 1) != 0xCC &&		// Filling
				*(BYTE*)((DWORD)TargetFunction->GetAddress() - 1) != 0xC3)		// RET
			{
				// Input is most likely faulty, we are probably not at a real functions start address, but somewhere *inside* a function!!
				return NULL;
			}
		}		

		// Hook layout:
		//  (>> : code created in this function)
		// Original caller
		//		|
		//		| Original call
		//		|
		//		V
		// >> Patched-in jump instruction
		//		|
		//		| Patched in JMP to pre-code
		//		|
		//		V
		// >> Pre-code: save the stack, set global detour return address
		//		|
		//		| JMP to global detour
		//		|
		//		V
		// Global detour: call all registered detours
		//		|
		//		| Global detour returns to address set in pre-code
		//		|
		//		V
		// >> Post-code: cleans up the stack, jumps to the unhooked function
		//		|
		//		| JMP
		//		|
		//		V
		// >> Unhooked function: part of the original function that was overwritten by the patch
		//		|
		//		| JMP to original function body
		//		|
		//		V
		// Original function body: we're done :-)

		DWORD wildcard = 0x1337;

			//-- Small preparations
		MemoryRegion* original_code = Process::GetCurrentProcess()->WriteMemory(TargetFunctionAddress, patchinfo->GetNumOpcodes());

		MemoryRegion* unhooked_fn_region = Process::GetCurrentProcess()->WriteMemory(TargetFunction->GetAddress(), patchinfo->GetNumOpcodes() + ASM_SIZE(jmp_far));

		MemoryRegion* patch = Process::GetCurrentProcess()->WriteMemory(ASM(jmp_far), ASM_SIZE(jmp_far));

		NotifyHook* ret = new NotifyHook(TargetFunctionAddress, Name, unhooked_fn_region->GetStartAddress(), original_code, patch);


			//-- We work from end to beginning. First, we setup the unhooked function portion
		MemoryRegion* unhooked_fn_jmp_region = Process::GetCurrentProcess()->WriteMemory(ASM(jmp_far), ASM_SIZE(jmp_far), PTRADD(unhooked_fn_region->GetStartAddress(), patchinfo->GetNumOpcodes()));

		DWORD unhooked_fn_jmp_offset = jmp_offs_notify(unhooked_fn_jmp_region->GetStartAddress(), PTRADD(TargetFunctionAddress, patchinfo->GetNumOpcodes()), ASM_SIZE(jmp_far));

		// First occurence == jmp offset to original function body
		unhooked_fn_jmp_region->ReplaceFirstOccurence(wildcard, unhooked_fn_jmp_offset);
		
		// Fix any jumps
		if (patchinfo->GetJumpInstructionAddress() != NULL)
		{
			DWORD original_jmp_offset = *(DWORD*)((BYTE*)patchinfo->GetJumpInstructionAddress() + patchinfo->GetJumpInstructionLength() - sizeof(void*));
			DWORD original_jmp_target = original_jmp_offset + (DWORD)patchinfo->GetJumpInstructionAddress() + patchinfo->GetJumpInstructionLength();

			DWORD jmp_instruction_offset_from_fn_start = (DWORD)patchinfo->GetJumpInstructionAddress() - (DWORD)TargetFunction->GetAddress();
			DWORD new_offset = original_jmp_target - ((DWORD)unhooked_fn_region->GetStartAddress() + jmp_instruction_offset_from_fn_start) - patchinfo->GetJumpInstructionLength();
			*(DWORD*)((DWORD)unhooked_fn_region->GetStartAddress() + jmp_instruction_offset_from_fn_start + patchinfo->GetJumpInstructionLength() - sizeof(void*)) = new_offset;
		}


			//-- Now we setup the post code
		MemoryRegion* post_code_region = Process::GetCurrentProcess()->WriteMemory(ASM(pop_ecx_jmp), ASM_SIZE(pop_ecx_jmp));

		// First occurence == jmp offset to unhooked function
		void* post_code_jmp_instr_addr = (DWORD*)post_code_region->ReplaceFirstOccurence(wildcard, NULL);

		DWORD post_code_jmp_offset = jmp_offs_notify((void*)((DWORD)post_code_jmp_instr_addr - 1), unhooked_fn_region->GetStartAddress(), ASM_SIZE(jmp_far));

		(*(DWORD*)post_code_jmp_instr_addr) = post_code_jmp_offset;


			//-- Now we setup the pre code
		MemoryRegion* pre_code_region = Process::GetCurrentProcess()->WriteMemory(ASM(push_ecx_and_two_args), ASM_SIZE(push_ecx_and_two_args));

		// First occurence == first parameter for detour: relevant hook
		pre_code_region->ReplaceFirstOccurence(wildcard, (DWORD)ret);

		// Second occurence == address of post code
		pre_code_region->ReplaceFirstOccurence(wildcard, (DWORD)post_code_region->GetStartAddress());

		// Third occurence == jmp offset to detour function
		pre_code_region->ReplaceFirstOccurence(wildcard, (DWORD)&NotifyHook::global_detour);

			//-- Now we setup the patch	
		DWORD patch_jmp_offset = jmp_offs_notify(TargetFunctionAddress, pre_code_region->GetStartAddress(), ASM_SIZE(jmp_far));

		// First occurence == jmp offset to pre code
		patch->ReplaceFirstOccurence(wildcard, patch_jmp_offset);		

		return ret;
	}
} }
