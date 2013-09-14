; 
; Copyright (c) 2013 Stijn "tcpie" Hinterding (contact: contact at tcpie dot eu)
; 
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
; 
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.

.586
.model flat, stdcall
option casemap :none

; ------
; Prototypes
asm_push_eax PROTO C
asm_push_eax_end PROTO C

asm_pop_eax PROTO C
asm_pop_eax_end PROTO C

asm_sub_esp PROTO C
asm_sub_esp_end PROTO C

asm_jmp_eax PROTO C
asm_jmp_eax_end PROTO C

asm_change_ret_addr PROTO C
asm_change_ret_addr_end PROTO C

asm_remove_stack_args_and_return PROTO C
asm_remove_stack_args_and_return_end PROTO C

asm_remove_stack_args_save_retaddr PROTO C
asm_remove_stack_args_save_retaddr_end PROTO C

asm_add_stack_args_save_retaddr PROTO C
asm_add_stack_args_save_retaddr_end PROTO C

asm_first_arg_into_ecx PROTO C
asm_first_arg_into_ecx_end PROTO C

asm_jmp_far PROTO C
asm_jmp_far_end PROTO C

asm_push_two_args PROTO C
asm_push_two_args_end PROTO C

asm_push_ecx_and_two_args PROTO C
asm_push_ecx_and_two_args_end PROTO C

asm_pop_ecx_jmp PROTO C
asm_pop_ecx_jmp_end PROTO C

asm_call_fn PROTO address:DWORD, argc:DWORD, args:DWORD, instance:DWORD, do_cleanup:BYTE, retval_ptr:DWORD, return_address:DWORD
asm_call_fn_end PROTO

; ------
; Data
.DATA
DwordMarker DWORD 1337h


; ------
; Code

.CODE

asm_push_eax PROC C
	push eax
asm_push_eax ENDP

asm_push_eax_end PROC C
asm_push_eax_end ENDP

; ----------------------

asm_pop_eax PROC C
	pop eax
asm_pop_eax ENDP

asm_pop_eax_end PROC C
asm_pop_eax_end ENDP

; ----------------------

asm_sub_esp PROC C
	sub esp, 1337h
	ret
asm_sub_esp ENDP

asm_sub_esp_end PROC C
asm_sub_esp_end ENDP

; ----------------------

asm_jmp_eax PROC C
	jmp eax
asm_jmp_eax ENDP

asm_jmp_eax_end PROC C
asm_jmp_eax_end ENDP

; ----------------------

asm_jmp_far PROC C
	;jmp DWORD PTR DwordMarker
	BYTE 0E9h
	DWORD 1337h
asm_jmp_far ENDP

asm_jmp_far_end PROC C
asm_jmp_far_end ENDP

; ----------------------

asm_change_ret_addr PROC C
	pop eax;#1
	xor ecx, ecx;#2
	sub esp, 4;#3

	loop_start:
		imul ecx, ecx, 4;#3
		add ecx, esp;#2
		mov edx, DWORD PTR ds:[ecx + 4];#?
		mov DWORD PTR ds:[ecx], edx;#?
		sub ecx, esp;#2
		sar ecx, 2;#3
		inc ecx;#1
		cmp ecx, 1337h;#6
	jl loop_start;#2

	mov DWORD PTR ss:[esp + 1337h], eax;#?
	push 1337h;#5
	mov eax, 1337h;#5
	jmp eax;#2
asm_change_ret_addr ENDP

asm_change_ret_addr_end PROC C
asm_change_ret_addr_end ENDP

; ----------------------

asm_remove_stack_args_and_return PROC C
	add esp, 1337h
	ret
asm_remove_stack_args_and_return ENDP

asm_remove_stack_args_and_return_end PROC C
asm_remove_stack_args_and_return_end ENDP

; ----------------------

asm_remove_stack_args_save_retaddr PROC C
	add esp, 1337h
	pop edx
	sub esp, 1337h
	push edx
	ret
asm_remove_stack_args_save_retaddr ENDP

asm_remove_stack_args_save_retaddr_end PROC C
asm_remove_stack_args_save_retaddr_end ENDP

; ----------------------

asm_add_stack_args_save_retaddr PROC C
	pop edx
	sub esp, 1337h
	push edx
	ret
asm_add_stack_args_save_retaddr ENDP

asm_add_stack_args_save_retaddr_end PROC C
asm_add_stack_args_save_retaddr_end ENDP

; ----------------------

asm_first_arg_into_ecx PROC C
	pop eax
	pop ecx
	push eax
	mov eax, 1337h
	jmp eax
asm_first_arg_into_ecx ENDP

asm_first_arg_into_ecx_end PROC C
asm_first_arg_into_ecx_end ENDP

; ----------------------

asm_push_ecx_and_extra_arg PROC C
	pop eax
	push ecx
	push 1337h
	push eax
	mov ecx, 1337h
	mov eax, 1337h
	jmp eax
asm_push_ecx_and_extra_arg ENDP

asm_push_ecx_and_extra_arg_end PROC C
asm_push_ecx_and_extra_arg_end ENDP

; ----------------------

asm_push_extra_arg PROC C
	pop eax
	push 1337h
	push eax
	mov ecx, 1337h
	mov eax, 1337h
	jmp eax
asm_push_extra_arg ENDP

asm_push_extra_arg_end PROC C
asm_push_extra_arg_end ENDP

; ----------------------

asm_push_two_args PROC C
	push 1337h
	push 1337h
	mov eax, 1337h
	jmp eax
asm_push_two_args ENDP

asm_push_two_args_end PROC C
asm_push_two_args_end ENDP

; ----------------------

asm_push_ecx_and_two_args PROC C
	push ebp
	push eax
	push ecx
	push edx
	push ebx
	push edi
	push 1337h
	push 1337h
	mov eax, 1337h
	jmp eax
asm_push_ecx_and_two_args ENDP

asm_push_ecx_and_two_args_end PROC C
asm_push_ecx_and_two_args_end ENDP

; ----------------------

asm_pop_ecx_jmp PROC C
	pop edi;
	pop ebx;
	pop edx;
	pop ecx;
	pop eax;	
	pop ebp
	BYTE 0E9h;
	DWORD 1337h;
asm_pop_ecx_jmp ENDP

asm_pop_ecx_jmp_end PROC C
asm_pop_ecx_jmp_end ENDP

asm_call_fn PROC address:DWORD, argc:DWORD, args:DWORD, instance:DWORD, do_cleanup:BYTE, retval_ptr:DWORD, return_address:DWORD
	
	cmp argc, 2
	je no_invalid_argc
		xor eax, eax
	no_invalid_argc:

	mov ecx, argc
	
	cmp ecx, 0
	je no_args

		loop_start:					; Push all required args onto the stack
			mov eax, argc
			sub eax, ecx
			imul eax, 4
			add eax, args
			push [eax]
		loop loop_start

	no_args:

	mov ecx, instance
	call address				; Call the desired function

	cmp do_cleanup, 0
	je no_cleanup

		mov ecx, argc
		imul ecx, 4
		add esp, ecx				; Clean up the stack if needed

	no_cleanup:

	cmp return_address, 0
	je no_special_return

		mov ecx, return_address		; Return to the desired address
		mov [esp + 4], ecx

	no_special_return:

	cmp retval_ptr, 0
	je fn_end
		
		mov ecx, retval_ptr
		mov [ecx], eax

	fn_end:
	ret
asm_call_fn ENDP

asm_call_fn_end PROC
asm_call_fn_end ENDP

; ----------------------

END