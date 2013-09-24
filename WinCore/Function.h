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
	PatchInfo(PatchInfo& other)
	{
		this->num_opcodes = other.GetNumOpcodes();
		this->jump_instruction_address = other.GetJumpInstructionAddress();
		this->jump_instruction_length = other.GetJumpInstructionLength();
	}

	PatchInfo(int NumOpcodes, void* JumpInstructionAddress, int JumpInstructionLength)
	{
		this->num_opcodes = NumOpcodes;
		this->jump_instruction_address = JumpInstructionAddress;
		this->jump_instruction_length = JumpInstructionLength; 
	}

	int GetNumOpcodes() const { return this->num_opcodes; }
	void* GetJumpInstructionAddress() const { return this->jump_instruction_address; }
	int GetJumpInstructionLength() const { return this->jump_instruction_length; }
};

class __declspec(dllexport) Function
{
private:
	const Process* process;
	void* address;
	int argcount;
	ReturnType return_type;
	CallingConvention calling_convention;
	PatchInfo* patchinfo;

	bool callInDifferentProcess(const Process* TargetProcess, const Thread* CallingThread, const std::vector<void*>* Args, void* instance, bool cleanup, void* asm_fn, DWORD* ret_val) const;
	bool callInDifferentThread(const Thread* CallingThread, const std::vector<void*>* Args, void* instance, bool cleanup, DWORD* ret_val) const;

protected:
	MemoryRegion* CreateFunctionStackCleaner(size_t NumArgsToClean) const;

public:
	static Function* FindFunction(const std::vector<BYTE>* Signature, const std::vector<char>* SignatureMask, CallingConvention CallConv, int ArgCount, ReturnType RetType);
	static Function* FindFunction(const std::vector<BYTE>* Signature, const std::vector<char>* SignatureMask, const Process* process, CallingConvention CallConv, int ArgCount, ReturnType RetType);

	Function(Function& other);
	Function(void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType);
	Function(const Process* process, void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType);

	~Function();

	const Process* GetProcess() const { return this->process; }
	ReturnType GetReturnType() const { return this->return_type; }
	void* GetAddress() const { return this->address; }
	int GetArgCount() const { return this->argcount; }
	CallingConvention GetCallingConvention() const { return this->calling_convention; }

	PatchInfo* FindPatchInfo();

	Function* CreateWrapperWithForcedReturnAddress(void* Address) const;
	Function* CreateStdcallWrapper(int NumArgsToClean = -1) const;

	// Calls the function.
	//		If the function is within the current process,
	//		the current thread is used. If not, all threads
	//		of the target process are gathered and tried
	//		one-by-one to execute the function. If execution
	//		succeeds, Call() returns.
	//
	//		The return value is true on success and false on
	//		function call time out.
	bool Call(const std::vector<void*>* Args, DWORD* ReturnValue = NULL, void* Instance = NULL) const;

	// Calls the function using the specified thread.
	//		If the specified thread is NULL, the
	//		current thread will be used.
	//
	//		Note that the call may time out if the
	//		specified thread is currently sleeping!
	bool Call(const std::vector<void*>* Args, const Thread* CallingThread, DWORD* ReturnValue = NULL, void* Instance = NULL) const;
};

} }

#endif
