	.arm
	.global arm7clearRAM

arm7clearRAM:

	push	{r0-r9}
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

	// clear most of EWRAM - except after RAM end - 0xc000, which has the bootstub
	mov	r8, #0x02000000
	
	ldr	r9,=0x4004008
	ldr	r9,[r9]
	ands	r9,r9,#0x8000
	bne	dsi_mode

	mov	r9, #0x02400000
	b	ds_mode
dsi_mode:
	mov	r9, #0x03000000	
ds_mode:
	sub	r9, #0x0000c000
clear_EWRAM_loop:
	stmia	r8!, {r0, r1, r2, r3, r4, r5, r6, r7}
	cmp	r8, r9
	blt	clear_EWRAM_loop

	pop	{r0-r9}
	
	bx	lr
  
