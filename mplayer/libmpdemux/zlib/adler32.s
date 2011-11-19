.file	1 "adler32.c"
.section .mdebug.abi32
.previous
.section	.text.adler32,"ax",@progbits
.align	2
.align	5
.globl	adler32
.ent	adler32
adler32:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

move	$8,$5
beq	$5,$0,$L23
move	$11,$6

srl	$10,$4,16
beq	$6,$0,$L5
andi	$9,$4,0xffff

$L16:
sltu	$2,$11,5552
beq	$2,$0,$L7
slt	$2,$11,16

move	$7,$11
bne	$2,$0,$L9
move	$11,$0

$L17:
lbu	$2,0($8)
lbu	$3,1($8)
addu	$2,$2,$9
lbu	$4,2($8)
addu	$3,$2,$3
lbu	$5,3($8)
addu	$2,$2,$10
addu	$4,$3,$4
addu	$2,$2,$3
lbu	$6,4($8)
addu	$5,$4,$5
addu	$2,$2,$4
lbu	$3,5($8)
addu	$6,$5,$6
addu	$2,$2,$5
lbu	$4,6($8)
addu	$3,$6,$3
addu	$2,$2,$6
lbu	$5,7($8)
addu	$4,$3,$4
addu	$2,$2,$3
lbu	$6,8($8)
addu	$5,$4,$5
addu	$2,$2,$4
lbu	$3,9($8)
addu	$6,$5,$6
addu	$2,$2,$5
lbu	$4,10($8)
addu	$3,$6,$3
addu	$2,$2,$6
lbu	$5,11($8)
addu	$4,$3,$4
addu	$2,$2,$3
lbu	$6,12($8)
addu	$5,$4,$5
addu	$2,$2,$4
lbu	$3,13($8)
addu	$6,$5,$6
addu	$2,$2,$5
lbu	$4,14($8)
addu	$3,$6,$3
lbu	$5,15($8)
addu	$2,$2,$6
addu	$4,$3,$4
addu	$2,$2,$3
addiu	$7,$7,-16
addu	$9,$4,$5
addu	$2,$2,$4
slt	$3,$7,16
addu	$10,$2,$9
beq	$3,$0,$L17
addiu	$8,$8,16

$L9:
beq	$7,$0,$L24
li	$2,-2147024896			# 0xffffffff80070000

move	$4,$8
move	$3,$7
$L13:
lbu	$2,0($4)
addiu	$3,$3,-1
addu	$9,$9,$2
addiu	$4,$4,1
bne	$3,$0,$L13
addu	$10,$10,$9

addu	$8,$8,$7
li	$2,-2147024896			# 0xffffffff80070000
$L24:
ori	$2,$2,0x8071
multu	$10,$2
mfhi	$5
multu	$9,$2
srl	$5,$5,15
mfhi	$2
sll	$6,$5,4
srl	$2,$2,15
sll	$7,$2,4
sll	$4,$2,16
sll	$3,$5,16
subu	$4,$4,$7
subu	$3,$3,$6
addu	$4,$4,$2
addu	$3,$3,$5
subu	$9,$9,$4
bne	$11,$0,$L16
subu	$10,$10,$3

$L5:
sll	$2,$10,16
j	$31
or	$2,$2,$9

$L7:
addiu	$11,$11,-5552
j	$L17
li	$7,5552			# 0x15b0

$L23:
j	$31
li	$2,1			# 0x1

.set	macro
.set	reorder
.end	adler32
.size	adler32, .-adler32
.ident	"GCC: (GNU) 4.1.2"
