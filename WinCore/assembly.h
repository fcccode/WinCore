#ifndef _ASSEMBLY_H_
#define _ASSEMBLY_H_

#define ASMSTUB extern "C" void __cdecl

#define ASM(x)			asm_## x
#define ASM_END(x)		asm_## x ##_end
#define ASM_SIZE(x)		(unsigned long)asm_## x ##_end - (unsigned long)asm_## x

ASMSTUB asm_push_eax();
ASMSTUB asm_push_eax_end();

ASMSTUB asm_pop_eax();
ASMSTUB asm_pop_eax_end();

ASMSTUB asm_sub_esp();
ASMSTUB asm_sub_esp_end();

ASMSTUB asm_jmp_eax();
ASMSTUB asm_jmp_eax_end();

ASMSTUB asm_change_ret_addr();
ASMSTUB asm_change_ret_addr_end();

ASMSTUB asm_remove_stack_args_and_return();
ASMSTUB asm_remove_stack_args_and_return_end();

ASMSTUB asm_remove_stack_args_save_retaddr();
ASMSTUB asm_remove_stack_args_save_retaddr_end();

ASMSTUB asm_add_stack_args_save_retaddr();
ASMSTUB asm_add_stack_args_save_retaddr_end();

ASMSTUB asm_first_arg_into_ecx();
ASMSTUB asm_first_arg_into_ecx_end();

ASMSTUB asm_jmp_far();
ASMSTUB asm_jmp_far_end();

ASMSTUB asm_push_ecx_and_extra_arg();
ASMSTUB asm_push_ecx_and_extra_arg_end();

ASMSTUB asm_push_extra_arg();
ASMSTUB asm_push_extra_arg_end();

ASMSTUB asm_push_two_args();
ASMSTUB asm_push_two_args_end();

ASMSTUB asm_push_ecx_and_two_args();
ASMSTUB asm_push_ecx_and_two_args_end();

ASMSTUB asm_pop_ecx_jmp();
ASMSTUB asm_pop_ecx_jmp_end();

extern "C" 
{
	DWORD __stdcall asm_call_fn(void* function_address, unsigned int argc, DWORD* args, void* instance, bool do_cleanup, DWORD* return_value = NULL, void* return_address = NULL);
	void __stdcall asm_call_fn_end();
}

#endif
