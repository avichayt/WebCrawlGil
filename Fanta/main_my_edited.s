	.file	"main.c"
 # GNU C11 (MinGW.org GCC-6.3.0-1) version 6.3.0 (mingw32)
 #	compiled by GNU C version 6.3.0, GMP version 6.1.2, MPFR version 3.1.5, MPC version 1.0.3, isl version 0.15
 # GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
 # options passed:  -iprefix c:\mingw\bin\../lib/gcc/mingw32/6.3.0/ main.c
 # -mtune=generic -march=i586 -fverbose-asm
 # options enabled:  -faggressive-loop-optimizations
 # -fasynchronous-unwind-tables -fauto-inc-dec -fchkp-check-incomplete-type
 # -fchkp-check-read -fchkp-check-write -fchkp-instrument-calls
 # -fchkp-narrow-bounds -fchkp-optimize -fchkp-store-bounds
 # -fchkp-use-static-bounds -fchkp-use-static-const-bounds
 # -fchkp-use-wrappers -fcommon -fdelete-null-pointer-checks
 # -fdwarf2-cfi-asm -fearly-inlining -feliminate-unused-debug-types
 # -ffunction-cse -fgcse-lm -fgnu-runtime -fgnu-unique -fident
 # -finline-atomics -fira-hoist-pressure -fira-share-save-slots
 # -fira-share-spill-slots -fivopts -fkeep-inline-dllexport
 # -fkeep-static-consts -fleading-underscore -flifetime-dse
 # -flto-odr-type-merging -fmath-errno -fmerge-debug-strings -fpeephole
 # -fplt -fprefetch-loop-arrays -freg-struct-return
 # -fsched-critical-path-heuristic -fsched-dep-count-heuristic
 # -fsched-group-heuristic -fsched-interblock -fsched-last-insn-heuristic
 # -fsched-rank-heuristic -fsched-spec -fsched-spec-insn-heuristic
 # -fsched-stalled-insns-dep -fschedule-fusion -fsemantic-interposition
 # -fset-stack-executable -fshow-column -fsigned-zeros
 # -fsplit-ivs-in-unroller -fssa-backprop -fstdarg-opt
 # -fstrict-volatile-bitfields -fsync-libcalls -ftrapping-math
 # -ftree-cselim -ftree-forwprop -ftree-loop-if-convert -ftree-loop-im
 # -ftree-loop-ivcanon -ftree-loop-optimize -ftree-parallelize-loops=
 # -ftree-phiprop -ftree-reassoc -ftree-scev-cprop -funit-at-a-time
 # -funwind-tables -fverbose-asm -fzero-initialized-in-bss -m32 -m80387
 # -m96bit-long-double -maccumulate-outgoing-args -malign-double
 # -malign-stringops -mavx256-split-unaligned-load
 # -mavx256-split-unaligned-store -mfancy-math-387 -mfp-ret-in-387
 # -mieee-fp -mlong-double-80 -mms-bitfields -mno-red-zone -mno-sse4
 # -mpush-args -msahf -mstack-arg-probe -mstv -mvzeroupper

	.comm	____CANARY___, 8, 3
	.def	___main;	.scl	2;	.type	32;	.endef
	.section .rdata,"dr"
LC0:
	.ascii "%d : %d\0"
	.text
	.globl	_main
	.def	_main;	.scl	2;	.type	32;	.endef
_main:
LFB15:
	.cfi_startproc
	pushl	%ebp	 #
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp	 #,
	.cfi_def_cfa_register 5
	pushl	%ebx	 #
	andl	$-16, %esp	 #,
	subl	$32, %esp	 #,
	.cfi_offset 3, -12
	call	___main	 #
	movl	$0, (%esp)	 #,
	call	_time	 #
	movl	%eax, (%esp)	 # _6,
	call	_srand	 #
	call	_rand	 #
	cltd
	movl	%eax, ____CANARY___	 # _10, ___CANARY___
	movl	%edx, ____CANARY___+4	 # _10, ___CANARY___
	movl	$____CANARY___+4, %ebx	 #, _12
	call	_rand	 #
	movl	%eax, (%ebx)	 # _14, *_12
	movl	$____CANARY___+4, %eax	 #, _16
	movb	$0, (%eax)	 #, *_16
	movl	$0, 28(%esp)	 #, i
	jmp	L2	 #
L3:
	movl	28(%esp), %eax	 # i, tmp97
	movl	%eax, (%esp)	 # tmp97,
	call	_foo	 #
	movl	%eax, 8(%esp)	 # _20,
	movl	28(%esp), %eax	 # i, tmp98
	movl	%eax, 4(%esp)	 # tmp98,
	movl	$LC0, (%esp)	 #,
	call	_printf	 #
	addl	$1, 28(%esp)	 #, i
L2:
	cmpl	$9, 28(%esp)	 #, i
	jle	L3	 #,
	movl	$0, %eax	 #, _23
	movl	-4(%ebp), %ebx	 #,
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE15:
	.section .rdata,"dr"
LC1:
	.ascii "num is %d\12\0"
	.text
	.globl	_foo
	.def	_foo;	.scl	2;	.type	32;	.endef
_foo:
LFB16:
	.cfi_startproc
	call _canaryStart
	pushl	%ebp	 #
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp	 #,
	.cfi_def_cfa_register 5
	subl	$24, %esp	 #,
	addl	$1, 16(%ebp)	 #, num
	movl	16(%ebp), %eax	 # num, tmp89
	movl	%eax, 4(%esp)	 # tmp89,
	movl	$LC1, (%esp)	 #,
	call	_printf	 #
	movl	16(%ebp), %eax	 # num, _5
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	call _canaryEnd
	ret
	.cfi_endproc
LFE16:
	.ident	"GCC: (MinGW.org GCC-6.3.0-1) 6.3.0"
	.def	_time;	.scl	2;	.type	32;	.endef
	.def	_srand;	.scl	2;	.type	32;	.endef
	.def	_rand;	.scl	2;	.type	32;	.endef
	.def	_printf;	.scl	2;	.type	32;	.endef


_canaryStart:
	.intel_syntax
	mov eax, [esp]
	push eax
	push eax
	mov ebx, offset ____CANARY___
	mov eax, [ebx]
	mov [esp + 4], eax
	mov eax, [ebx + 4]
	mov [esp + 8], eax
	ret

_canaryEnd:
	.intel_syntax
	push eax
	push edx
	push ebx
	mov eax, [esp + 0x14]
	mov edx, [esp + 0x10]
	mov ebx, offset ____CANARY___
	cmp [ebx + 4], eax
	jnz EXIT
	cmp [ebx], edx
	jnz EXIT
	jmp DONE
	EXIT:
		push -1
		call _exit
	DONE:
		pop ebx
		pop edx
		mov eax, [esp + 4]
		mov [esp + 0x0c], eax
		pop eax
		add esp, 8
		ret


