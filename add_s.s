	.global add_s
	.func add_s

add_s:
	sub sp, sp, #16
	str lr, [sp]
	str r1, [sp, #4]
	str r3, [sp, #8]
	ldr r7,[sp, #4]
	ldr r8, [sp, #8]
//	ldr r1,[sp]
	add sp, sp, #16
	//	LDRB r6, [r0,r1]
//	str r1, [r0,r5]
//	cmp r1,#4080
//	add r3, r0, #4080
//	addeq r3, r0, r3
//	subne r3,r1,r0
//	subeq r0, r0, #4080	
//	mov r3, r1
//	mov r7, #4080	
//	cmp r1,r1	
//	addeq r1,r1,r1

///	beq loop
//	add r1,r2,r4
//loop:
//	add r5,r2,r4	
	bx lr

