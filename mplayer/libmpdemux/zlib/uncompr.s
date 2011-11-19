.file	1 "uncompr.c"
.section .mdebug.abi32
.previous
.section	.rodata.str1.4,"aMS",@progbits,1
.align	2
$LC0:
.ascii	"1.2.1\000"
.section	.text.uncompress,"ax",@progbits
.align	2
.align	5
.globl	uncompress
.ent	uncompress
uncompress:
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
sw	$2,32($sp)
move	$18,$5
lui	$2,%hi(inflateInit_)
lui	$5,%hi($LC0)
sw	$6,16($sp)
sw	$4,28($sp)
addiu	$5,$5,%lo($LC0)
move	$4,$17
li	$6,56			# 0x38
sw	$7,20($sp)
sw	$0,48($sp)
addiu	$2,$2,%lo(inflateInit_)
jal	$2
sw	$0,52($sp)

move	$16,$2
move	$4,$17
beq	$2,$0,$L11
li	$5,4			# 0x4

$L2:
move	$2,$16
$L13:
lw	$31,84($sp)
lw	$18,80($sp)
lw	$17,76($sp)
lw	$16,72($sp)
j	$31
addiu	$sp,$sp,88

$L11:
lui	$2,%hi(inflate)
addiu	$2,$2,%lo(inflate)
jal	$2
nop

move	$16,$2
li	$2,1			# 0x1
bne	$16,$2,$L12
move	$4,$17

lw	$2,36($sp)
sw	$2,0($18)
lui	$2,%hi(inflateEnd)
addiu	$2,$2,%lo(inflateEnd)
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
lui	$2,%hi(inflateEnd)
addiu	$2,$2,%lo(inflateEnd)
jal	$2
nop

li	$3,2			# 0x2
beq	$16,$3,$L6
li	$2,-5			# 0xfffffffffffffffb

bne	$16,$2,$L13
move	$2,$16

lw	$2,20($sp)
bne	$2,$0,$L13
move	$2,$16

$L6:
j	$L2
li	$16,-3			# 0xfffffffffffffffd

.set	macro
.set	reorder
.end	uncompress
.size	uncompress, .-uncompress
.ident	"GCC: (GNU) 4.1.2"
