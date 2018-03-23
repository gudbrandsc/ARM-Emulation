	.global fib_iter_s
	.func fib_iter_s
fib_iter_s:
	cmp r0, #1 // if(n > 1)
//	bls set_one //if not
//	push {r4, r5}
	mov r1, #0 // int f1
	mov r2, #1 // int f2
	mov r3, #1 // int i
	mov r4, #0 // int res

loop:

	cmp r3, r0 //if i == n
	bge done // if true end

	add r4, r2, r1  // sum = f1 + f2
	add r3, r3, #1 // i = i + 1
	mov r1, r2 // f1 = f2
	mov r2, r4 // f2 = res

	b loop // loop back
done:
	mov r0, r4 // mov fib value to return register
//	pop {r4, r5}
	b end // return
set_one:
	mov r0, #1 // set return value as 1 
	b end // return
end:
	bx lr
