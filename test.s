			# This code was produced by the CERI Compiler
FormatString1:	.string "%llu"	# used by printf to display 64-bit unsigned integers
FormatString2:	.string "%lf"	# used by printf to display 64-bit floating point numbers
FormatString3:	.string "%c"	# used by printf to display a 8-bit single character
TrueString:	.string "TRUE"	# used by printf to display the boolean value TRUE
FalseString:	.string "FALSE"	# used by printf to display the boolean value FALSE
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
	push $1
	pop a
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl	$3779571220, (%rsp)	# Conversion of 1.88 (32 bit high part)
	movl	$1073615994, 4(%rsp)	# Conversion of 1.88 (32 bit low part)
	pop e
	push a
	pop %rdx # The value to be displayed
	movq $FormatString1, %rsi 
	movl $1, %edi
	movl $0, %eax
	call __printf_chk@PLT
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
