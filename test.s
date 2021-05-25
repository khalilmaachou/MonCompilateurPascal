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
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl	$0, (%rsp)	# Conversion of 1 (32 bit high part)
	movl	$1072693248, 4(%rsp)	# Conversion of 1 (32 bit low part)
	pop e
	push $3
	pop a
DO1:
WHILE1:
	push a
	push c
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	jb Vrai1	# If below
	push $0		# False
	jmp Suite1
Vrai1:	push $0xFFFFFFFFFFFFFFFF		# True
Suite1:
	pop %rax
	cmpq $0,%rax
	je ENDWHILE1
	push $2
	pop b
	jmp WHILE1
ENDWHILE1:
	push a
	push b
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	jb Vrai2	# If below
	push $0		# False
	jmp Suite2
Vrai2:	push $0xFFFFFFFFFFFFFFFF		# True
Suite2:
	pop %rax
	cmpq $0,%rax
	jne DO1
	push $2
	pop a
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
