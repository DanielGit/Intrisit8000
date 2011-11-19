.file	1 "compress.c"
.section .mdebug.abi32
.previous
.section	.text.compressBound,"ax",@progbits
.align	2
.align	5
.globl	compressBound
.ent	compressBound
compressBound:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

srl	$2,$4,12
addu	$2,$4,$2
addiu	$2,$2,11
srl	$4,$4,14
j	$31
addu	$2,$4,$2

.set	macro
.set	reorder
.end	compressBound
.size	compressBound, .-compressBound
.section	.rodata.str1.4,"aMS",@progbits,1
.align	2
$LC0:
.ascii	"1.2.1\000"
.section	.text.compress2,"ax",@progbits
.align	2
.align	5
.globl	compress2
.ent	compress2
compress2:
.frame	$sp,88,$31		# vars= 56, regs= 4/0, args= 16, gp= 0
.mask	0x80070000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-88
sw	$18,80($sp)
sw	$17,76($sp)
sw	$16,72($sp)
sw	$31,84($sp)
lw	$2,0($5)
addiu	$17,$sp,16
sw	$6,16($sp)
sw	$2,32($sp)
move	$18,$5
lui	$6,%hi($LC0)
lw	$5,104($sp)
lui	$2,%hi(deflateInit_)
sw	$7,20($sp)
sw	$4,28($sp)
addiu	$6,$6,%lo($LC0)
move	$4,$17
li	$7,56			# 0x38
sw	$0,48($sp)
sw	$0,52($sp)
addiu	$2,$2,%lo(deflateInit_)
jal	$2
sw	$0,56($sp)

move	$16,$2
move	$4,$17
beq	$2,$0,$L11
li	$5,4			# 0x4

$L4:
move	$2,$16
$L13:
lw	$31,84($sp)
lw	$18,80($sp)
lw	$17,76($sp)
lw	$16,72($sp)
j	$31
addiu	$sp,$sp,88

$L11:
lui	$2,%hi(deflate)
addiu	$2,$2,%lo(deflate)
jal	$2
nop

move	$16,$2
li	$2,1			# 0x1
bne	$16,$2,$L12
move	$4,$17

lw	$2,36($sp)
sw	$2,0($18)
lui	$2,%hi(deflateEnd)
addiu	$2,$2,%lo(deflateEnd)
jal	$2
move	$4,$17

move	$16,$2
move	$2,$16
lw	$31,84($sp)
lw	$18,80($sp)
lw	$17,76($sp)
lw	$16,72($sp)
j	$31
addiu	$sp,$sp,88

$L12:
lui	$2,%hi(deflateEnd)
addiu	$2,$2,%lo(deflateEnd)
jal	$2
nop

bne	$16,$0,$L13
move	$2,$16

j	$L4
li	$16,-5			# 0xfffffffffffffffb

.set	macro
.set	reorder
.end	compress2
.size	compress2, .-compress2
.section	.text.compress,"ax",@progbits
.align	2
.align	5
.globl	compress
.ent	compress
compress:
.frame	$sp,88,$31		# vars= 56, regs= 4/0, args= 16, gp= 0
.mask	0x80070000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-88
sw	$18,80($sp)
sw	$17,76($sp)
sw	$16,72($sp)
sw	$31,84($sp)
lw	$2,0($5)
addiu	$17,$sp,16
sw	$6,16($sp)
sw	$2,32($sp)
lui	$6,%hi($LC0)
lui	$2,%hi(deflateInit_)
sw	$7,20($sp)
sw	$4,28($sp)
move	$18,$5
move	$4,$17
li	$5,-1			# 0xffffffffffffffff
addiu	$6,$6,%lo($LC0)
li	$7,56			# 0x38
sw	$0,48($sp)
sw	$0,52($sp)
addiu	$2,$2,%lo(deflateInit_)
jal	$2
sw	$0,56($sp)

move	$16,$2
move	$4,$17
beq	$2,$0,$L21
li	$5,4			# 0x4

$L15:
move	$2,$16
$L23:
lw	$31,84($sp)
lw	$18,80($sp)
lw	$17,76($sp)
lw	$16,72($sp)
j	$31
addiu	$sp,$sp,88

$L21:
lui	$2,%hi(deflate)
addiu	$2,$2,%lo(deflate)
jal	$2
nop

move	$16,$2
li	$2,1			# 0x1
bne	$16,$2,$L22
move	$4,$17

lw	$2,36($sp)
sw	$2,0($18)
lui	$2,%hi(deflateEnd)
addiu	$2,$2,%lo(deflateEnd)
jal	$2
move	$4,$17

move	$16,$2
move	$2,$16
lw	$31,84($sp)
lw	$18,80($sp)
lw	$17,76($sp)
lw	$16,72($sp)
j	$31
addiu	$sp,$sp,88

$L22:
lui	$2,%hi(deflateEnd)
addiu	$2,$2,%lo(deflateEnd)
jal	$2
nop

bne	$16,$0,$L23
move	$2,$16

j	$L15
li	$16,-5			# 0xfffffffffffffffb

.set	macro
.set	reorder
.end	compress
.size	compress, .-compress
.ident	"GCC: (GNU) 4.1.2"
