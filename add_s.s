	.global add_s
	.func add_s

add_s:
	add r3, r0, #4080
	add r3, r0, r3
	sub r3,r1,r0
	sub r3,r1,#4080	
	mov r3, r1
	mov r7, #4080	
	cmp r1,#4
	cmp r1,r1	
	b loop
	add r1,r2,r4
loop:
	add r5,r2,r4	
	bx lr

