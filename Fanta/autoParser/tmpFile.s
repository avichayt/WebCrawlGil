	.file	"tmp_c.c"
	.text
	.globl	FAILURE_CANARY_MSG
	.section	.rodata
.LC0:
	.string	"buffer overflow!\n"
	.section	.data.rel.local,"aw",@progbits
	.align 4
	.type	FAILURE_CANARY_MSG, @object
	.size	FAILURE_CANARY_MSG, 4
FAILURE_CANARY_MSG:
	.long	.LC0
	.section	.rodata
.LC1:
	.string	"stack End: %lx\n"
	.align 4
.LC2:
	.string	"\npress a button to run shellcode!"
.LC3:
	.string	"done"
	.text
	.globl	main
	.type	main, @function

_canaryStart:
	.intel_syntax noprefix
    sub esp, 8
    push eax
    push ebx
	mov eax, [esp + 16]
    mov [esp + 8], eax
    lea eax, [esp + 12]
    push eax
	call getNewCanary
	add esp, 4
	mov ebx, eax
	mov eax, [ebx]
	mov [esp + 12], eax
	mov eax, [ebx + 4]
	mov [esp + 16], eax
	mov eax, [esp + 20]
	xor [esp + 16], eax
	pop ebx
	pop eax
	ret
	.att_syntax 

_canaryEnd:
	.intel_syntax noprefix
	push eax
	push edx
	push ebx
	push ecx
	call popCanary
	
	mov ecx, [esp + 0x18]
	xor ecx, [esp + 0x1c]
	mov edx, [esp + 0x14]
	
	mov ebx, eax
	cmp [ebx + 4], ecx
	jnz EXIT
	cmp [ebx], edx
	jnz EXIT
	jmp DONE
	EXIT:
	    push FAILURE_CANARY_MSG
	    call printf
		push -1
		call exit
	DONE:
	    pop ecx
		pop ebx
		pop edx
		mov eax, [esp + 4]
		mov [esp + 0x0c], eax
		pop eax
		add esp, 8
		ret
	.att_syntax 
main:
	pushl 8(%esp)
	call initProtector
	addl $4, %esp
.LFB5:
	.cfi_startproc
	leal	4(%esp), %ecx
	.cfi_def_cfa 1, 0
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	.cfi_escape 0x10,0x5,0x2,0x75,0
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%ecx
	.cfi_escape 0xf,0x3,0x75,0x78,0x6
	.cfi_escape 0x10,0x3,0x2,0x75,0x7c
	subl	$16, %esp
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
#APP
# 37 "tmp_c.c" 1
	mov %esp, %eax;
# 0 "" 2
#NO_APP
	movl	%eax, -12(%ebp)
	subl	$8, %esp
	pushl	-12(%ebp)
	leal	.LC1@GOTOFF(%ebx), %eax
	pushl	%eax
	call	printf@PLT
	addl	$16, %esp
	subl	$12, %esp
	leal	.LC2@GOTOFF(%ebx), %eax
	pushl	%eax
	call	puts@PLT
	addl	$16, %esp
	call	getchar@PLT
	call	getchar@PLT
	movl	stdin@GOT(%ebx), %eax
	movl	(%eax), %eax
	subl	$12, %esp
	pushl	%eax
	call	fflush@PLT
	addl	$16, %esp
	subl	$12, %esp
	leal	.LC3@GOTOFF(%ebx), %eax
	pushl	%eax
	call	puts@PLT
	addl	$16, %esp
	movl	$0, %eax
	leal	-8(%ebp), %esp
	popl	%ecx
	.cfi_restore 1
	.cfi_def_cfa 1, 0
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_restore 5
	leal	-4(%ecx), %esp
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE5:
	.size	main, .-main
	.section	.rodata
.LC4:
	.string	"parent canary = %llx\n"
.LC5:
	.string	"child canary = %llx\n"
	.text
	.globl	g
	.type	g, @function
g:
	call _canaryStart
.LFB6:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$36, %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	call	fork@PLT
	testl	%eax, %eax
	je	.L4
#APP
# 66 "tmp_c.c" 1
	mov %ebp, %eax
# 0 "" 2
#NO_APP
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	addl	$4, %eax
	movl	4(%eax), %edx
	movl	(%eax), %eax
	movl	%eax, -32(%ebp)
	movl	%edx, -28(%ebp)
	subl	$4, %esp
	pushl	-28(%ebp)
	pushl	-32(%ebp)
	leal	.LC4@GOTOFF(%ebx), %eax
	pushl	%eax
	call	printf@PLT
	addl	$16, %esp
	jmp	.L6
.L4:
#APP
# 75 "tmp_c.c" 1
	mov %ebp, %eax
# 0 "" 2
#NO_APP
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	addl	$4, %eax
	movl	4(%eax), %edx
	movl	(%eax), %eax
	movl	%eax, -24(%ebp)
	movl	%edx, -20(%ebp)
	subl	$4, %esp
	pushl	-20(%ebp)
	pushl	-24(%ebp)
	leal	.LC5@GOTOFF(%ebx), %eax
	pushl	%eax
	call	printf@PLT
	addl	$16, %esp
.L6:
	nop
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	call _canaryEnd
	ret
	.cfi_endproc
.LFE6:
	.size	g, .-g
	.globl	foo
	.type	foo, @function
foo:
	call _canaryStart
.LFB7:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$20, %esp
	.cfi_offset 3, -12
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$12, %esp
	pushl	$4097
	call	malloc@PLT
	addl	$16, %esp
	movl	%eax, -12(%ebp)
	call	getchar@PLT
	nop
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	call _canaryEnd
	ret
	.cfi_endproc
.LFE7:
	.size	foo, .-foo
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
.LFB8:
	.cfi_startproc
	movl	(%esp), %ebx
	ret
	.cfi_endproc
.LFE8:
	.ident	"GCC: (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0"
	.section	.note.GNU-stack,"",@progbits
