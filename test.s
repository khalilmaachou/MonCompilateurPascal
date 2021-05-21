			# This code was produced by the CERI Compiler
	.data
	.align 8
FormatString1:    .string "%llu\n"  # used by printf to display 64-bit unsigned integers
a:	.quad 0
b:	.quad 0
c:	.quad 0
z:	.quad 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push $0
	pop c
	push c
	push $2
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop %rdx # The value to be displayed
	movq $FormatString1, %rsi 
	movl $1, %edi
	movl $0, %eax
	call __printf_chk@PLT
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
