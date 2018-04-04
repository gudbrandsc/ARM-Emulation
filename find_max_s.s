	.global find_max_s
	.func find_max_s
	
find_max_s:
	sub sp, sp, #8
	str r4, [sp, #4]
	str r5, [sp, #8]	
	mov r2, #1 // int i
	ldr r3, [r0, #0]  // max = array[0]	
loop:
	cmp r2, r1 //if i<= array.length
	bge done // Exit
	lsl r4, r2, #2 //R4 = i + 1
	ldr r5, [r0, r4] // R5 = array[i]
	add r2, r2, #1 // i= i + 1
	cmp r5, r3 // if(array[i] >= max)
	bge set_largest // if(array[i) >= res -> if true
	b loop //Repeat loop
set_largest:
	mov r3, r5 // sum = array[i]
	b loop // go back to loop
done:
	mov r0, r3 // set max to return value
	add sp, sp, #8
	bx lr

