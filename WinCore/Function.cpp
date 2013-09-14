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
#include "Function.h"
#include "MemSearchable.h"
#include "Thread.h"
#include "Process.h"
#include "assembly.h"
#include "MemoryRegion.h"

#include "ADE32.h"

#define FN_MAGIC_NUMBER 0xAB098235

namespace tcpie { namespace wincore {

Function::Function(Function& other) : MemSearchable(other.signature, other.signature_mask)
{
	this->process = other.GetProcess();
	this->address = other.GetAddress();
	this->argcount = other.GetArgCount();
	this->return_type = other.GetReturnType();
	this->calling_convention = other.GetCallingConvention();
	this->patchinfo = other.patchinfo;
	this->signature = other.signature;
	this->signature_mask = other.signature_mask;
}

Function::Function(void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType, std::vector<unsigned char*>* Signature, std::wstring* SignatureMask)
	: MemSearchable(Signature, SignatureMask)
{
	this->process = Process::GetCurrentProcess();
	this->address = Address;
	this->argcount = ArgCount;
	this->return_type = RetType;
	this->calling_convention = CallConv;
	this->patchinfo = NULL;
}

Function::Function(Process* process, void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType, std::vector<unsigned char*>* Signature, std::wstring* SignatureMask)
	: MemSearchable(Signature, SignatureMask)
{
	this->process = process;
	this->address = Address;
	this->argcount = ArgCount;
	this->return_type = RetType;
	this->calling_convention = CallConv;
	this->patchinfo = NULL;
}

Function::~Function()
{
	if (this->patchinfo != NULL)
	{
		delete this->patchinfo;
	}
}

PatchInfo* Function::FindPatchInfo()
{
	if (this->patchinfo != NULL)
	{
		return this->patchinfo;
	}

	BYTE* addr = (BYTE*)this->address;
	int JmpInstructionLength = -1;
	void* JmpInstructionAddress = NULL;

	int tmpLen = 0;

	while(tmpLen < 5) 
	{
		int i = 0;

		// We have to be careful, if we run into jumps or calls!
		switch (addr[0])
		{
			// JMP SHORT	: length: 2 << UNSUPPORTED
		case 0xEB:
			// JMPF			: length: 5 + 2 = 7 << UNSUPPORTED
		case 0xEA:
			return NULL;

			// CALL			: length: 5
		case 0xE8:
			// JMP			: length: 5
		case 0xE9:
			i = 5;
			JmpInstructionLength = 5;
			JmpInstructionAddress = &addr[0];

			break;

			// Misc
		case 0x0F:
			switch (addr[1])
			{
				// JGE, JNE, etc etc
			case 0x80:	// 2 + 4
			case 0x81:	// 2 + 4
			case 0x82:	// 2 + 4
			case 0x83:	// 2 + 4
			case 0x84:	// 2 + 4
			case 0x85:	// 2 + 4
			case 0x86:	// 2 + 4
			case 0x87:	// 2 + 4
			case 0x88:	// 2 + 4
			case 0x89:	// 2 + 4
			case 0x8A:	// 2 + 4
			case 0x8B:	// 2 + 4
			case 0x8C:	// 2 + 4
			case 0x8D:	// 2 + 4
			case 0x8E:	// 2 + 4
			case 0x8F:	// 2 + 4
				i = 6;
				JmpInstructionLength = 6;
				JmpInstructionAddress = &addr[0];
				break;

			default:
				i = tcpie::thirdparty::z0mbie::oplen(addr);
				break;
			}

		default:
			i = tcpie::thirdparty::z0mbie::oplen(addr);
			break;
		}

		if(i == 0 || i == -1)
			return NULL;

		tmpLen += i;
		addr += i;
	}

	PatchInfo* info = new PatchInfo(tmpLen, JmpInstructionAddress, JmpInstructionLength);
	this->patchinfo = info;

	return info;
}

bool Function::callInDifferentThread(Thread* CallingThread, std::vector<void*>* Args, void* instance, bool cleanup, DWORD* return_value)
{
	CallingThread->Suspend();

	Sleep(10);

	CONTEXT context = CallingThread->GetContext();
	DWORD old_eip = context.Eip;
	DWORD ret_val = FN_MAGIC_NUMBER;

	std::vector<void*>* args = new std::vector<void*>();
	
	args->push_back((DWORD*)this->address);
	args->push_back((DWORD*)Args->size());
	args->push_back((DWORD*)&(*Args)[0]);
	args->push_back((DWORD*)instance);
	args->push_back((DWORD*)cleanup);
	args->push_back((DWORD*)&ret_val);
	args->push_back((DWORD*)old_eip);	

	CallingThread->PushToStack(args);

	context = CallingThread->GetContext();
	context.Eip = (DWORD)asm_call_fn;
	
	DWORD esp_after = context.Esp;

	CallingThread->SetContext(context);
	CallingThread->Resume();

	for (int i = 0; i < 50; i += 1)
	{
		if (ret_val != FN_MAGIC_NUMBER)
		{
			Sleep(5);		// Sleep to make sure asm_call_fn() has returned
			
			*return_value = ret_val;

			delete args;

			return true;
		}

		Sleep(1);
	}
	
	std::cout << "fn call failed." << std::endl;

	// Failure. Set ESP back:
	CallingThread->Suspend();

	context = CallingThread->GetContext();
	
	if (context.Esp == esp_after)
	{
		context.Esp += (args->size() + 1) * sizeof(DWORD*);
		std::cout << "Resetting ESP" << std::endl;
	}

	CallingThread->SetContext(context);

	CallingThread->Resume();

	*return_value = 0;

	delete args;

	return false;
}

bool Function::callInDifferentProcess(Process* TargetProcess, Thread* CallingThread, std::vector<void*>* Args, void* instance, bool cleanup, void* asm_fn, DWORD* return_value)
{
	std::cout << "addr of remote asm fn: 0x" << std::hex << (DWORD)asm_fn << std::dec << " calling thread id: " << std::hex << CallingThread->GetId() << std::dec << std::endl;

	CallingThread->Suspend();

	CONTEXT context = CallingThread->GetContext();
	DWORD old_eip = context.Eip;
	DWORD ret_val = FN_MAGIC_NUMBER;

	MemoryRegion* remote_args = TargetProcess->WriteMemory(&(*Args)[0], Args->size() * sizeof(DWORD*));
	MemoryRegion* remote_retval = TargetProcess->WriteMemory(&ret_val, sizeof(DWORD));

	std::vector<void*>* args = new std::vector<void*>();
	
	args->push_back((DWORD*)this->address);
	args->push_back((DWORD*)Args->size());
	args->push_back((DWORD*)remote_args->GetStartAddress());
	args->push_back((DWORD*)instance);
	args->push_back((DWORD*)cleanup);
	args->push_back((DWORD*)remote_retval->GetStartAddress());
	args->push_back((DWORD*)old_eip);	

	CallingThread->PushToStack(args);

	context = CallingThread->GetContext();
	context.Eip = (DWORD)asm_fn;
	
	DWORD esp_after = context.Esp;

	CallingThread->SetContext(context);

	MessageBoxA(NULL, "wait", "", MB_OK);
	CallingThread->Resume();

	for (int i = 0; i < 500; i += 15)
	{
		std::vector<BYTE>* ret_signal = TargetProcess->ReadMemory(remote_retval);
		DWORD new_ret = *(DWORD*)&(*ret_signal)[0];
		
		if (new_ret != FN_MAGIC_NUMBER)
		{
			Sleep(5);		// Sleep to make sure asm_call_fn() has returned
			*return_value = new_ret;

			delete args;

			return true;; 
		}

		CallingThread->Suspend();
		context = CallingThread->GetContext();
		CallingThread->Resume();

		std::cout << "eip: 0x" << std::hex << context.Eip << std::dec << std::endl;
		Sleep(15);
	}	
	
	std::cout << "fn call timed out." << std::endl;
	
	MessageBoxA(NULL, "stop", "", MB_OK);

	// Failure. Set ESP back:
	CallingThread->Suspend();

	context = CallingThread->GetContext();
	
	if (context.Esp == esp_after)
	{
		context.Esp += (args->size() + 1) * sizeof(DWORD*);
		std::cout << "Resetting ESP" << std::endl;
	}

	CallingThread->SetContext(context);

	CallingThread->Resume();	

	*return_value = 0;

	delete args;

	return false;
}

MemoryRegion* Function::CreateFunctionStackCleaner(size_t NumArgsToClean)
{
	DWORD wildcard = 0x1337;

	if (NumArgsToClean == this->argcount || this->argcount == -1)
	{
		MemoryRegion* region = Process::GetCurrentProcess()->WriteMemory(ASM(remove_stack_args_and_return), ASM_SIZE(remove_stack_args_and_return));

		if (region == NULL)
		{
			return NULL;
		}

		region->ReplaceFirstOccurence(wildcard, this->argcount * 4);

		return region;
	}
	else
	{
		MemoryRegion* region = Process::GetCurrentProcess()->WriteMemory(ASM(remove_stack_args_save_retaddr), ASM_SIZE(remove_stack_args_save_retaddr));

		if (region == NULL)
		{
			return NULL;
		}

		region->ReplaceFirstOccurence(wildcard, this->argcount * 4);
		region->ReplaceFirstOccurence(wildcard, (this->argcount - NumArgsToClean) * 4);

		return region;
	}

	return NULL;
}

Function* Function::SetFirstReturnAddress(void* Address)
{
	MemoryRegion* region = Process::GetCurrentProcess()->WriteMemory(ASM(change_ret_addr), ASM_SIZE(change_ret_addr));

	if (region == NULL)
	{
		return NULL;
	}

	DWORD wildcard = 0x1337;

	region->ReplaceFirstOccurence(wildcard, this->argcount);
	region->ReplaceFirstOccurence(wildcard, this->argcount * 4);
	region->ReplaceFirstOccurence(wildcard, (DWORD)Address);
	region->ReplaceFirstOccurence(wildcard, (DWORD)this->address);

	void* start_addr = region->GetStartAddress();

	delete region;

	return new Function(start_addr, this->calling_convention, this->argcount, this->return_type, this->signature, this->signature_mask);
}

Function* Function::CreateStdcallWrapper(int NumArgsToClean)
{
	if (this->calling_convention == STDCALL_CALLCONV || (this->calling_convention == CDECL_CALLCONV && this->argcount == 0))
	{
		return this;
	}

	if (this->calling_convention == CDECL_CALLCONV)
	{
		if (NumArgsToClean < 0)
		{
			NumArgsToClean = this->argcount;
		}

		MemoryRegion* region = this->CreateFunctionStackCleaner(NumArgsToClean);

		if (region == NULL)
		{
			return NULL;
		}

		Function* ret = this->SetFirstReturnAddress(region->GetStartAddress());
		ret->calling_convention = STDCALL_CALLCONV;

		delete region;

		return ret;
	}
	else if (this->calling_convention == THISCALL_CALLCONV)
	{
		DWORD wildcard = 0x1337;

		MemoryRegion* pre_fn = Process::GetCurrentProcess()->WriteMemory(ASM(first_arg_into_ecx), ASM_SIZE(first_arg_into_ecx));
		pre_fn->ReplaceFirstOccurence(wildcard, (DWORD)this->address);

		void* new_addr = pre_fn->GetStartAddress();

		delete pre_fn;

		return new Function(this->process, this->address, STDCALL_CALLCONV, this->argcount, this->return_type, this->signature, this->signature_mask);
	}

	return NULL;
}

bool Function::Call(std::vector<void*>* Args, DWORD* ReturnValue /* = NULL */, void* Instance /* = NULL */)
{
	if (this->process->GetId() == Process::GetCurrentProcessId())
	{
		return this->Call(Args, Thread::GetCurrentThread(), ReturnValue, Instance);
	}

	std::vector<Thread*>* threads = this->process->GetThreads();
	bool success = false;

	for (size_t i = 0; i < threads->size(); i++)
	{
		success = this->Call(Args, threads->at(i), ReturnValue, Instance);

		if (success)
		{
			break;
		}
	}

	for (size_t i = 0; i < threads->size(); i++)
	{
		delete threads->at(i);
	}

	threads->clear();

	delete threads;

	return success;
}

bool Function::Call(std::vector<void*>* Args, Thread* CallingThread, DWORD* ReturnValue, void* Instance)
{
	switch (this->calling_convention)
	{
	case STDCALL_CALLCONV:

		if (this->process->GetId() == Process::GetCurrentProcessId())
		{
			if (CallingThread->GetId() == Thread::GetCurrentThreadId())
			{
				*ReturnValue = asm_call_fn(this->address, Args->size(), (DWORD*)&(*Args)[0], NULL, false);

				return true;
			}

			return this->callInDifferentThread(CallingThread, Args, NULL, false, ReturnValue);
		}
		else
		{
			MemoryRegion* mem = this->process->WriteMemory(asm_call_fn, ASM_SIZE(call_fn));

			bool success = this->callInDifferentProcess(this->process, CallingThread, Args, NULL, false, mem->GetStartAddress(), ReturnValue);

			this->process->FreeMemory(mem);

			return success;
		}

		break;

	case CDECL_CALLCONV:

		if (this->process->GetId() == Process::GetCurrentProcessId())
		{
			if (CallingThread->GetId() == Thread::GetCurrentThreadId())
			{
				*ReturnValue = asm_call_fn(this->address, Args->size(), (DWORD*)&(*Args)[0], NULL, true);

				return true;
			}

			return this->callInDifferentThread(CallingThread, Args, NULL, true, ReturnValue);
		}
		else
		{
			MemoryRegion* mem = this->process->WriteMemory(asm_call_fn, ASM_SIZE(call_fn));

			bool success = this->callInDifferentProcess(this->process, CallingThread, Args, NULL, true, mem->GetStartAddress(), ReturnValue);

			this->process->FreeMemory(mem);

			return success;
		}

		break;

	case THISCALL_CALLCONV:

		if (this->process->GetId() == Process::GetCurrentProcessId())
		{
			if (CallingThread->GetId() == Thread::GetCurrentThreadId())
			{
				*ReturnValue = asm_call_fn(this->address, Args->size(), (DWORD*)&(*Args)[0], Instance, false);

				return true;
			}

			return this->callInDifferentThread(CallingThread, Args, Instance, false, ReturnValue);
		}
		else
		{
			MemoryRegion* mem = this->process->WriteMemory(asm_call_fn, ASM_SIZE(call_fn));

			bool success = this->callInDifferentProcess(this->process, CallingThread, Args, Instance, false, mem->GetStartAddress(), ReturnValue);

			this->process->FreeMemory(mem);

			return success;
		}

		break;
	}

	return 0;
}

} }
