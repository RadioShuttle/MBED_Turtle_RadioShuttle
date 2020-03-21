#ifdef __ARMCC_VERSION
#elif defined(__GNUC__)
	.cpu cortex-m4
	.eabi_attribute 27, 1
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 4
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"mbed-util.cpp"
	.text
	.section	.text.CPUID,"ax",%progbits
	.align	1
	.global	CPUID
	.arch armv7e-m
	.syntax unified
	.thumb
	.thumb_func
	.fpu fpv4-sp-d16
	.type	CPUID, %function
CPUID:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	cmp	r1, #15
	ble	.L4
	ldr	r3, .L6
	eors	r3, r3, r2
	subs	r3, r3, r0
	add	r2, r0, #12
.L3:
	ldrb	r1, [r0, r3]	@ zero_extendqisi2
	strb	r1, [r0], #1
	cmp	r2, r0
	bne	.L3
	movs	r0, #16
	bx	lr
.L4:
	movs	r0, #0
	bx	lr
.L7:
	.align	2
.L6:
	.word	1252663493
	.size	CPUID, .-CPUID
	.section	.text.AddRSLicense,"ax",%progbits
	.align	1
	.global	AddRSLicense
	.syntax unified
	.thumb
	.thumb_func
	.fpu fpv4-sp-d16
	.type	AddRSLicense, %function
AddRSLicense:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, lr}
	subs	r4, r0, #1
	adds	r4, r4, #3
	sub	sp, sp, #20
	bhi	.L15
	mov	r4, sp
	ldr	r6, .L22
	ldr	r7, .L22+4
	mov	r5, r4
.L10:
	mov	ip, r6
	adds	r6, r6, #1
	ldrb	ip, [ip]	@ zero_extendqisi2
	strb	ip, [r4], #1
	cmp	r6, r7
	bne	.L10
	ldr	r4, .L22+8
	movs	r6, #0
.L11:
	ldr	r7, [r5], #4
	adds	r6, r6, #1
	cmp	r6, #4
	add	r4, r4, r7
	bne	.L11
	add	r2, r2, r4
	subs	r5, r2, r3
	it	ne
	movne	r5, #1
	cmp	r1, #0
	it	lt
	addlt	r1, r1, #3
	movs	r4, #0
	asrs	r1, r1, #2
	mov	r6, r4
.L14:
	cmp	r1, r4
	ble	.L8
	cbz	r4, .L13
	ldr	r7, [r0, r4, lsl #2]
	cmp	r0, r7
	bne	.L13
	cmp	r2, r3
	it	eq
	streq	r6, [r0, r4, lsl #2]
.L13:
	adds	r4, r4, #1
	b	.L14
.L15:
	movs	r5, #1
.L8:
	mov	r0, r5
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, pc}
.L23:
	.align	2
.L22:
	.word	536835472
	.word	536835484
	.word	-2091612265
	.size	AddRSLicense, .-AddRSLicense
	.global	__aeabi_ui2d
	.global	__aeabi_ddiv
	.global	__aeabi_d2f
	.section	.text.BatteryVoltage,"ax",%progbits
	.align	1
	.global	BatteryVoltage
	.syntax unified
	.thumb
	.thumb_func
	.fpu fpv4-sp-d16
	.type	BatteryVoltage, %function
BatteryVoltage:
	@ args = 0, pretend = 0, frame = 128
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, lr}
	ldr	r5, .L32
	ldr	r6, .L32+4
	ldr	r3, [r5, #76]
	orr	r3, r3, #8192
	str	r3, [r5, #76]
	ldr	r3, [r5, #76]
	sub	sp, sp, #132
	and	r3, r3, #8192
	str	r3, [sp]
	ldr	r3, [sp]
	ldr	r3, [r5, #136]
	orr	r3, r3, #805306368
	str	r3, [r5, #136]
	add	r0, sp, #28
	str	r6, [sp, #28]
	bl	HAL_ADC_DeInit
	cmp	r0, #0
	bne	.L26
	movs	r3, #1
	strd	r3, r0, [sp, #64]
	strd	r0, r0, [sp, #32]
	strd	r0, r0, [sp, #40]
	movs	r7, #4
	strh	r0, [sp, #52]	@ movhi
	str	r3, [sp, #56]
	strb	r0, [sp, #60]
	str	r0, [sp, #72]
	strb	r0, [sp, #76]
	mov	r3, #4096
	strb	r0, [sp, #84]
	add	r0, sp, #28
	str	r7, [sp, #48]
	str	r3, [sp, #80]
	bl	HAL_ADC_Init
	cmp	r0, #0
	bne	.L27
	ldr	r2, .L32+8
	movs	r3, #6
	strd	r2, r3, [sp, #4]
	strd	r7, r0, [sp, #20]
	movs	r3, #7
	movs	r4, #127
	add	r1, sp, r7
	add	r0, sp, #28
	strd	r3, r4, [sp, #12]
	bl	HAL_ADC_ConfigChannel
	cmp	r0, #0
	bne	.L28
	mov	r1, r4
	add	r0, sp, #28
	bl	HAL_ADCEx_Calibration_Start
	cmp	r0, #0
	bne	.L29
	add	r0, sp, #28
	bl	HAL_ADC_Start
	cmp	r0, #0
	bne	.L30
	movs	r1, #10
	add	r0, sp, #28
	bl	HAL_ADC_PollForConversion
	cmp	r0, #0
	bne	.L31
	add	r0, sp, #28
	bl	HAL_ADC_GetValue
	ldr	r3, .L32+12
	udiv	r0, r3, r0
	bl	__aeabi_ui2d
	ldr	r3, .L32+16
	movs	r2, #0
	bl	__aeabi_ddiv
	bl	__aeabi_d2f
	mov	r4, r0	@ float
	add	r0, sp, #28
	bl	ADC_Disable
	add	r0, sp, #28
	bl	HAL_ADC_DeInit
	ldr	r3, [r5, #136]
	bic	r3, r3, #805306368
	str	r3, [r5, #136]
	ldr	r3, [r5, #76]
	bic	r3, r3, #8192
	str	r3, [r5, #76]
	ldr	r3, [r6, #8]
	bic	r3, r3, #25165824
	str	r3, [r6, #8]
.L24:
	mov	r0, r4	@ float
	add	sp, sp, #132
	@ sp needed
	pop	{r4, r5, r6, r7, pc}
.L26:
	mov	r4, #1065353216
	b	.L24
.L27:
	mov	r4, #1073741824
	b	.L24
.L28:
	ldr	r4, .L32+20
	b	.L24
.L29:
	mov	r4, #1082130432
	b	.L24
.L30:
	ldr	r4, .L32+24
	b	.L24
.L31:
	ldr	r4, .L32+28
	b	.L24
.L33:
	.align	2
.L32:
	.word	1073876992
	.word	1342439424
	.word	-2147483647
	.word	4914000
	.word	1083129856
	.word	1077936128
	.word	1084227584
	.word	1086324736
	.size	BatteryVoltage, .-BatteryVoltage
	.ident	"GCC: (GNU Tools for Arm Embedded Processors 9-2019-q4-major) 9.2.1 20191025 (release) [ARM/arm-9-branch revision 277599]"
#endif
