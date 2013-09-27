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

/// @file		Function.h
/// @author		tcpie
/// @brief      Contains code relevant for the Function class.

#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <vector>
#include <string>

/// @brief		My encapsulating namespace
namespace tcpie { 

/// @brief		Namespace for the WinCore project
namespace wincore {

class MemoryRegion;
class Process;
class Thread;

/// @brief		Contains the types of sizes a function can return
///
/// @public
enum ReturnType
{
	DWORD_SIZE = 0,		///< The function returns a value the size of a DWORD
	PTR_SIZE,			///< The function returns a value the size of a pointer (void*)
	BYTE_SIZE,			///< The function returns a value the size of a byte
	NONE,				///< The function does not return a value (void)
};

/// @brief		Contains the available calling conventions
///
/// @public
enum CallingConvention
{
	CDECL_CALLCONV = 0,	///< The function uses the cdecl calling convention (push right-to-left, caller cleans up stack)
	THISCALL_CALLCONV,	///< The function uses the thiscall calling conventions (push right-to-left, callee cleans up stack, instance in ECX)
	STDCALL_CALLCONV,	///< The function uses the stdcall calling convention (push right-to-left, callee cleans up stack)
};

/// @brief		Holds the relevant info for patching a JMP instruction at the start of a function
class __declspec(dllexport) PatchInfo
{
private:
	int num_opcodes;
	void* jump_instruction_address;
	int jump_instruction_length;

public:
	/// @brief		Copies an instance of patch info to a new instance
	///	@param		other						The other instance
	PatchInfo(PatchInfo& other)
	{
		this->num_opcodes = other.GetNumOpcodes();
		this->jump_instruction_address = other.GetJumpInstructionAddress();
		this->jump_instruction_length = other.GetJumpInstructionLength();
	}

	/// @brief		Construct an instance of PatchInfo
	/// @param		NumOpcodes					The number of opcodes needed before the minimum number for a jump (5) is reached.
	/// @param		JumpInstructionAddress		If there is a JMP at the function start, this hold it's address. Otherwise this should be NULL.
	/// @param		JumpInstructionLength		The length (in bytes) of the JMP instruction (regular JMP is 5, short JMP is smaller)
	PatchInfo(int NumOpcodes, void* JumpInstructionAddress, int JumpInstructionLength)
	{
		this->num_opcodes = NumOpcodes;
		this->jump_instruction_address = JumpInstructionAddress;
		this->jump_instruction_length = JumpInstructionLength; 
	}

	///	@brief		Gets the number of opcodes at the function start
	///
	///				The number of opcodes represents the number needed to reach the minimum number for a jump (5), while not
	///				cleaving any existing instructions.
	int GetNumOpcodes() const { return this->num_opcodes; }

	/// @brief		Gets the address of a jump instruction (if any)
	void* GetJumpInstructionAddress() const { return this->jump_instruction_address; }

	/// @brief		Gets the jump instruction size
	int GetJumpInstructionLength() const { return this->jump_instruction_length; }
};

/// @brief			Provides methods to deal with functions
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

	MemoryRegion* CreateFunctionStackCleaner(size_t NumArgsToClean) const;

public:
	/// @brief		Finds a function using the specified parameters
	/// @param		Signature			The function's signature
	/// @param		SignatureMask		The functions's signature mask. Should have the same length as Signature. If any bytes need to be ignored, denote them with '?'
	/// @param		CallingConv			The function's calling convention
	/// @param		ArgCount			The numer of argument the function has. Use -1 for variable number of arguments.
	/// @param		RetType				The return type of the function
	///
	///				This method assumes the function is in the current process. Internally this function uses MemoryRegion::FindAddress
	///
	/// @return		An instance of a Function on success, NULL on failure.
	static Function* FindFunction(const std::vector<BYTE>* Signature, const std::vector<char>* SignatureMask, CallingConvention CallConv, int ArgCount, ReturnType RetType);

	/// @brief		Finds a function using the specified parameters
	/// @param		Signature			The function's signature
	/// @param		SignatureMask		The functions's signature mask. Should have the same length as Signature. If any bytes need to be ignored, denote them with '?'
	/// @param		process				The process the function is in
	/// @param		CallingConv			The function's calling convention
	/// @param		ArgCount			The number of argument the function has. Use -1 for variable number of arguments.
	/// @param		RetType				The return type of the function
	///
	///				Internally this function uses MemoryRegion::FindAddress
	///
	/// @return		An instance of a Function on success, NULL on failure.
	static Function* FindFunction(const std::vector<BYTE>* Signature, const std::vector<char>* SignatureMask, const Process* process, CallingConvention CallConv, int ArgCount, ReturnType RetType);

	/// @brief		Copy constructor
	Function(Function& other);

	/// @brief		Constructs a Function
	/// @param		Address				The function's address
	/// @param		CallConv			The function's calling convention
	/// @param		ArgCount			The number of argument the function has. Use -1 for variable number of arguments.
	/// @param		RetType				The return type of the function
	///
	///				This method assumes the function is in the current process.
	Function(void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType);

	/// @brief		Constructs a Function
	/// @param		process				The process the function is in
	/// @param		Address				The function's address
	/// @param		CallConv			The function's calling convention
	/// @param		ArgCount			The number of argument the function has. Use -1 for variable number of arguments.
	/// @param		RetType				The return type of the function
	Function(const Process* process, void* Address, CallingConvention CallConv, int ArgCount, ReturnType RetType);

	/// @brief		Default destructor
	~Function();

	/// @brief		Gets the function's process
	const Process* GetProcess() const { return this->process; }

	/// @brief		Gets the function's return type
	ReturnType GetReturnType() const { return this->return_type; }

	/// @brief		Gets the function's address
	void* GetAddress() const { return this->address; }

	/// @brief		Gets the function's argument count
	int GetArgCount() const { return this->argcount; }

	/// @brief		Gets the function's calling convention
	CallingConvention GetCallingConvention() const { return this->calling_convention; }

	/// @brief		Find the patch info for this instance
	/// @return		Returns a pointer to a PatchInfo instance on success. On failure the function returns NULL.
	///
	///	As all the other functions that start with Find*, this function can be very slow. Internally
	///	this function disassembles the function, to get the required info. Additional calls to this
	///	function are fast, however, since the function will return the stored info.
	PatchInfo* FindPatchInfo();

	/// @brief		This method creates a wrapper for the current function, forcing the function to return to the specified address.
	/// @param		Address			The address the function should return to.
	/// @return		Upon success, a new instance of a Function, representing the wrapper.
	///
	///	This method returns a brand new Function. If this returned function is called, it return to the specified address,
	///	instead of the address it was called from. The function the wrapper was created for remains unaltered.
	Function* CreateWrapperWithForcedReturnAddress(void* Address) const;

	/// @brief		Creates a wrapper for the current function, which can be called as if the function had the stdcall calling convention.
	/// @param		NumArgsToClean		The number of arguments the wrapper should clean. Set this to -1 so that the wrapper will clean
	///									the number of arguments the function has.
	///
	///	This method returns a brand new Function. This new function can be called as if it has the stdcall calling convention.
	///	thus, if the wrapped function is of the cdecl calling convention, the caller does not have to clean the stack when
	///	calling the wrapper. If the wrapped function is of the thiscall calling convention, instead of passing the instance
	///	through ECX, the caller must provide it as the argument to the wrapper.
	Function* CreateStdcallWrapper(int NumArgsToClean = -1) const;

	///	@brief		Calls the function
	///	@param		Args				The arguments to the function. Elements are pushed in the supplied order, the first element is pushed first.
	/// @param		ReturnValue			A pointer to a DWORD where the function's return value will be stored
	/// @param		Instance			The instance to use when calling the function
	/// @return		True on success, false on function call time-out.
	///
	///	This method will call the Function. If the function is within the current process,
	///	the current thread is used. If not, all threads of the target process are gathered
	///	and tried one-by-one to execute the function. If execution succeeds, Call() returns.
	///
	/// !!A warning on return values!! in some cases (eg: float values), the function does not
	/// return the actual return value, but a pointer to the value.
	bool Call(const std::vector<void*>* Args, DWORD* ReturnValue = NULL, void* Instance = NULL) const;

	///	@brief		Calls the function
	///	@param		Args				The arguments to the function. Elements are pushed in the supplied order, the first element is pushed first.
	/// @param		CallingThread		The thread to use for calling the function.
	/// @param		ReturnValue			A pointer to a DWORD where the function's return value will be stored
	/// @param		Instance			The instance to use when calling the function
	/// @return		True on success, false on function call time-out.
	///
	///	This method will call the Function. If the specified thread is NULL, the current thread
	///	will be used. Note that the call may time out if the specified thread is currently sleeping!
	///
	/// !!A warning on return values!! in some cases (eg: float values), the function does not
	/// return the actual return value, but a pointer to the value.
	bool Call(const std::vector<void*>* Args, const Thread* CallingThread, DWORD* ReturnValue = NULL, void* Instance = NULL) const;
};

} }

#endif
