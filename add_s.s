	.global add_s
	.func add_s

add_s:
	add r3, r0, #2
	add r3, r0, r3
	sub r3,r1,r0
	sub r3,r1,#2	
	mov r3, r1
	mov r7, #6	
	bx lr
