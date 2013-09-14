#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include "MemSearchable.h"

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

class __declspec(dllexport) Function : public MemSearchable
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
	Function(Function& other);
	Function(void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType, std::vector<unsigned char*>* Signature, std::wstring* SignatureMask);
	Function(Process* process, void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType, std::vector<unsigned char*>* Signature, std::wstring* SignatureMask);

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
