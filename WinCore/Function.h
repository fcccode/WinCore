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

#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <vector>
#include <string>

namespace tcpie { namespace wincore {

class MemoryRegion;
class Process;
class Thread;

enum ReturnType
{
	DWORD_SIZE = 0,
	PTR_SIZE,
	BYTE_SIZE,
	NONE,
};

enum CallingConvention
{
	CDECL_CALLCONV = 0,
	THISCALL_CALLCONV,
	STDCALL_CALLCONV,
};

class __declspec(dllexport) PatchInfo
{
private:
	int num_opcodes;
	void* jump_instruction_address;
	int jump_instruction_length;

public:
	PatchInfo(int NumOpcodes, void* JumpInstructionAddress, int JumpInstructionLength)
	{
		this->num_opcodes = NumOpcodes;
		this->jump_instruction_address = JumpInstructionAddress;
		this->jump_instruction_length = JumpInstructionLength; 
	}

	int GetNumOpcodes() { return this->num_opcodes; }
	void* GetJumpInstructionAddress() { return this->jump_instruction_address; }
	int GetJumpInstructionLength() { return this->jump_instruction_length; }
};

class __declspec(dllexport) Function
{
private:
	Process* process;
	void* address;
	int argcount;
	ReturnType return_type;
	CallingConvention calling_convention;
	PatchInfo* patchinfo;

	bool callInDifferentProcess(Process* TargetProcess, Thread* CallingThread, std::vector<void*>* Args, void* instance, bool cleanup, void* asm_fn, DWORD* ret_val);
	bool callInDifferentThread(Thread* CallingThread, std::vector<void*>* Args, void* instance, bool cleanup, DWORD* ret_val);

protected:
	MemoryRegion* CreateFunctionStackCleaner(size_t NumArgsToClean);

public:
	static Function* FindFunction(const std::vector<BYTE>* Signature, const std::vector<char>* SignatureMask, CallingConvention CallConv, int ArgCount, ReturnType RetType);
	static Function* FindFunction(const std::vector<BYTE>* Signature, const std::vector<char>* SignatureMask, Process* process, CallingConvention CallConv, int ArgCount, ReturnType RetType);

	Function(Function& other);
	Function(void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType);
	Function(Process* process, void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType);

	~Function();

	Process* GetProcess() { return this->process; }
	ReturnType GetReturnType() { return this->return_type; }
	void* GetAddress() { return this->address; }
	int GetArgCount() { return this->argcount; }
	CallingConvention GetCallingConvention() { return this->calling_convention; }

	PatchInfo* FindPatchInfo();

	Function* SetFirstReturnAddress(void* Address);
	Function* CreateStdcallWrapper(int NumArgsToClean = -1);

	// Calls the function.
	//		If the function is within the current process,
	//		the current thread is used. If not, all threads
	//		of the target process are gathered and tried
	//		one-by-one to execute the function. If execution
	//		succeeds, Call() returns.
	//
	//		The return value is true on success and false on
	//		function call time out.
	bool Call(std::vector<void*>* Args, DWORD* ReturnValue = NULL, void* Instance = NULL);

	// Calls the function using the specified thread.
	//		Note that the call may time out if the
	//		specified thread is currently sleeping!
	bool Call(std::vector<void*>* Args, Thread* CallingThread, DWORD* ReturnValue = NULL, void* Instance = NULL);
};

} }

#endif
