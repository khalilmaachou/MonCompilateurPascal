			# This code was produced by the CERI Compiler
	.data
	.align 8
a:	.quad 0
b:	.quad 0
c:	.quad 0
z:	.quad 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
WHILE1:
	push a
	push b
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	jb Vrai1	# If below
	push $0		# False
	jmp Suite1
Vrai1:	push $0xFFFFFFFFFFFFFFFF		# True
Suite1:
	pop %rax
	cmpq %rax,$0
	je ENDWHILE1
	push $0
	pop a
	push $2
	pop %rax
FOR2:
	cmpq %rax,a
	ja ENDFOR2
	push $1
	pop a
	incq	a# ADD
	jmp FOR2
ENDFOR2:
	jmp WHILE1
ENDWHILE1:
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
