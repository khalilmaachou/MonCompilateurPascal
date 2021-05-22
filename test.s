			# This code was produced by the CERI Compiler
	.data
	.align 8
a:	.quad 0
b:	.quad 0
c:	.quad 0
e:	.double 0.0
f:	.double 0.0
h:	.byte 0
i:	.byte 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	movq $0,%rcx
	movb $'a',%cl
	push %rcx
	pop h
	movq $0,%rcx
	movb h,%cl
	push %rcx
	pop %rdx # The value to be displayed
	movq $FormatString1, %rsi 
	movl $1, %edi
	movl $0, %eax
	call __printf_chk@PLT
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
