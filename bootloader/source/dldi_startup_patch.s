
.arm

.global dldi_startup_patch

.align 4

dldi_startup_patch:
	mov		r0, #1	@ Return true
	bx		lr

