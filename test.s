			# This code was produced by the CERI Compiler
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	       .data
	a      .quad 0
	b      .quad 0
	c      .quad 0
	b      .quad 0
	push $33
