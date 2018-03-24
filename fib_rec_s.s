	.global fib_rec_s
	.func fib_rec_s

fib_rec_s:
	sub sp, sp, #16
	str lr, [sp]
	cmp r0, #0 // if n == 0
	moveq r0, #0 // return 0
	beq end // end fuction 
	cmp r0, #1 // if n == 1 
	moveq r0, #1 // return 1
	beq end // end fuction 
	bne rec // else call recursive
	b end
rec:	
	str r0, [sp, #4] //Store R0 in sp
	sub r0, r0, #1 // n-1
	bl fib_rec_s //Call fib_rec_s(n-1)
	str r0, [sp, #8]
	ldr r0, [sp, #4]
	sub r0, r0, #2 //n-2
	bl fib_rec_s // call fib_rec_s(n-2)
	ldr r1, [sp, #8]
	add r0, r1, r0 // add fib(n-1)+fib(n-2)
end:
	ldr lr, [sp] 
	add sp, sp, #16
	bx lr	//return R0

