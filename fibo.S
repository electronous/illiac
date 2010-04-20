# Illiac 3 assembly code

# I'm not sure on the syntax yet or if you can even _do_ this in IBAL,
# but it is legal Illiac 3 as far as I know

# We're going to cheat and use pointer register values, so we are limited
# to using 16-bit width for fibo.  Going any bigger requires using the stack.

#Here's how to do Fibo without Specify

Fibo_Without_Specify: # Assumes n, the number of iterations, is in R4
						# and that n > 2
	Assign H, R4, R4-2 				# skip n = 1, 2
	Assign H, R2, R3 = 1			# setup fibo numbers
	Assign H, R5, R5 = FWOS_Done	# setup label
FWOS_Loop:
	Assign H, R4, R4 - 1  			# test n
	Ifn CS, R5						# if done, goto FWOS_Done
	Assign H, R3, R3 + R2			# fibo(n) = fibo(n-1) + fibo(n-2)

	# The next 3 lines are swap R2, R3
	Assign H, R6, R1
	Assign H, R1, R3
	Assign H, R3, R6

	Assign H, R0, R0 = FWOS_Loop	# J FWOS_Loop
FWOS_Done:
	# There's nothing to actually do here, R3 contains Fibo(n)

