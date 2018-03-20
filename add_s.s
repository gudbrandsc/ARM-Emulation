
	.global add_s
    .func add_s

add_s:
	add r3,r0,r1
	sub r3,r1,r0	
	cmp r3, r1
	bx lr
