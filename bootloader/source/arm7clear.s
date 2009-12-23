	.arm
	.global arm7clearRAM

arm7clearRAM:

	// clear exclusive IWRAM
	// 0380:0000 to 0380:FFFF, total 64KiB
	mov	r0, #0
	mov	r1, #0
	mov	r2, #0
	mov	r3, #0
	mov	r4, #0
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0x03800000
	sub	r8, #0x00008000
	mov	r9, #0x03800000
	orr	r9, r9, #0x10000
clear_IWRAM_loop:
	stmia	r8!, {r0, r1, r2, r3, r4, r5, r6, r7}
	cmp	r8, r9
	blt	clear_IWRAM_loop

	// clear most of EWRAM - except after 0x023FFE00, which has the ARM9 loop
	mov	r8, #0x02000000
	mov	r9, #0x02400000
	sub	r9, #0x00000200
clear_EWRAM_loop:
	stmia	r8!, {r0, r1, r2, r3, r4, r5, r6, r7}
	cmp	r8, r9
	blt	clear_EWRAM_loop
	
	bx	lr
  
