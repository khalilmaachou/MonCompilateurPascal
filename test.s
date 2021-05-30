			# This code was produced by the CERI Compiler
	.data
FormatString1:	.string "%llu"	# used by printf to display 64-bit unsigned integers
FormatString2:	.string "%lf"	# used by printf to display 64-bit floating point numbers
FormatString3:	.string "%c"	# used by printf to display a 8-bit single character
TrueString:	.string "TRUE"	# used by printf to display the boolean value TRUE
FalseString:	.string "FALSE"	# used by printf to display the boolean value FALSE
	.align 8
e:	.double 0.0
a:	.quad 0
b:	.quad 0
c:	.quad 0
d:	.quad 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push $2
	pop d
	push $0
	pop c
	push $0
	pop a
FOR1:
	push d
	push $1
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop %rax
	cmpq %rax,a
	je ENDFOR1
	push $0
	pop b
FOR2:
	push $2
	pop %rax
	cmpq %rax,b
	je ENDFOR2
	push $0
	pop d
FOR3:
	push $2
	pop %rax
	cmpq %rax,d
	je ENDFOR3
	push c
	push $1
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop c
	incq d	#ADD
	jmp FOR3
ENDFOR3:
	incq b	#ADD
	jmp FOR2
ENDFOR2:
	incq a	#ADD
	jmp FOR1
ENDFOR1:
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl	$0, (%rsp)	# Conversion of 3.5 (32 bit high part)
	movl	$1074528256, 4(%rsp)	# Conversion of 3.5 (32 bit low part)
	pop e
	push c
	pop %rdx # The value to be displayed
	movq $FormatString1, %rsi 
	movl $1, %edi
	movl $0, %eax
	call __printf_chk@PLT
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
