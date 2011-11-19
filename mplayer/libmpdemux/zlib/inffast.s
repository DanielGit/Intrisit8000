.file	1 "inffast.c"
.section .mdebug.abi32
.previous
.section	.rodata.str1.4,"aMS",@progbits,1
.align	2
$LC0:
.ascii	"invalid distance too far back\000"
.align	2
$LC1:
.ascii	"invalid distance code\000"
.align	2
$LC2:
.ascii	"invalid literal/length code\000"
.section	.text.inflate_fast,"ax",@progbits
.align	2
.align	5
.globl	inflate_fast
.ent	inflate_fast
inflate_fast:
.frame	$sp,48,$31		# vars= 8, regs= 9/0, args= 0, gp= 0
.mask	0x40ff0000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-48
sw	$22,32($sp)
sw	$21,28($sp)
sw	$19,20($sp)
sw	$17,12($sp)
sw	$16,8($sp)
sw	$fp,40($sp)
sw	$23,36($sp)
sw	$20,24($sp)
sw	$18,16($sp)
lw	$14,28($4)
lw	$2,12($4)
lw	$3,16($4)
lw	$7,0($4)
lw	$8,76($14)
lw	$6,80($14)
addiu	$13,$2,-1
move	$16,$4
li	$2,1			# 0x1
lw	$4,4($4)
sll	$6,$2,$6
subu	$5,$5,$3
sll	$2,$2,$8
addiu	$7,$7,-1
lw	$8,32($14)
addu	$4,$7,$4
addu	$3,$13,$3
subu	$5,$13,$5
lw	$fp,36($14)
lw	$20,40($14)
lw	$23,44($14)
lw	$11,48($14)
lw	$10,52($14)
lw	$25,68($14)
lw	$18,72($14)
sw	$8,0($sp)
addiu	$19,$4,-5
sw	$5,4($sp)
addiu	$17,$3,-257
addiu	$21,$2,-1
addiu	$22,$6,-1
li	$24,1			# 0x1
sltu	$2,$10,15
$L113:
beq	$2,$0,$L111
and	$2,$11,$21

lbu	$2,1($7)
addiu	$7,$7,2
lbu	$3,0($7)
sll	$2,$2,$10
addiu	$4,$10,8
addu	$2,$11,$2
sll	$3,$3,$4
addu	$11,$2,$3
addiu	$10,$10,16
and	$2,$11,$21
$L111:
sll	$2,$2,2
addu	$2,$25,$2
lbu	$3,1($2)
lbu	$4,0($2)
subu	$10,$10,$3
srl	$11,$11,$3
beq	$4,$0,$L5
lhu	$3,2($2)

andi	$2,$4,0x10
bne	$2,$0,$L7
move	$5,$4

andi	$2,$4,0x40
beq	$2,$0,$L78
andi	$2,$5,0x20

j	$L106
nop

$L12:
bne	$8,$0,$L112
andi	$4,$5,0xf

bne	$9,$0,$L10
andi	$2,$5,0x20

$L78:
$L11:
sll	$2,$24,$5
addiu	$2,$2,-1
and	$2,$2,$11
addu	$2,$3,$2
sll	$2,$2,2
addu	$2,$25,$2
lbu	$3,1($2)
lbu	$4,0($2)
subu	$10,$10,$3
srl	$11,$11,$3
move	$5,$4
andi	$9,$4,0x40
andi	$8,$4,0x10
bne	$4,$0,$L12
lhu	$3,2($2)

$L5:
addiu	$13,$13,1
sb	$3,0($13)
$L13:
sltu	$2,$7,$19
$L115:
beq	$2,$0,$L34
sltu	$8,$13,$17

bne	$8,$0,$L113
sltu	$2,$10,15

$L34:
srl	$2,$10,3
subu	$7,$7,$2
addiu	$3,$7,1
sll	$2,$2,3
addiu	$4,$13,1
sltu	$5,$7,$19
subu	$6,$10,$2
sw	$3,0($16)
beq	$5,$0,$L68
sw	$4,12($16)

$L101:
subu	$2,$19,$7
addiu	$2,$2,5
beq	$8,$0,$L71
sw	$2,4($16)

$L102:
subu	$2,$17,$13
addiu	$3,$2,257
$L73:
li	$2,1			# 0x1
sll	$2,$2,$6
addiu	$2,$2,-1
and	$2,$11,$2
sw	$3,16($16)
sw	$2,48($14)
sw	$6,52($14)
lw	$fp,40($sp)
lw	$23,36($sp)
lw	$22,32($sp)
lw	$21,28($sp)
lw	$20,24($sp)
lw	$19,20($sp)
lw	$18,16($sp)
lw	$17,12($sp)
lw	$16,8($sp)
j	$31
addiu	$sp,$sp,48

$L7:
andi	$4,$5,0xf
$L112:
bne	$4,$0,$L98
move	$15,$3

sltu	$2,$10,15
bne	$2,$0,$L99
nop

$L19:
and	$2,$11,$22
sll	$2,$2,2
addu	$2,$18,$2
lbu	$8,0($2)
lbu	$3,1($2)
andi	$4,$8,0x10
subu	$10,$10,$3
srl	$11,$11,$3
bne	$4,$0,$L21
lhu	$12,2($2)

andi	$2,$8,0x40
beq	$2,$0,$L79
lui	$2,%hi($LC1)

j	$L116
addiu	$2,$2,%lo($LC1)

$L26:
bne	$9,$0,$L24
lui	$2,%hi($LC1)

$L79:
$L25:
sll	$2,$24,$8
addiu	$2,$2,-1
and	$2,$2,$11
addu	$2,$12,$2
sll	$2,$2,2
addu	$2,$18,$2
lbu	$3,0($2)
lbu	$4,1($2)
andi	$5,$3,0x10
andi	$9,$3,0x40
subu	$10,$10,$4
srl	$11,$11,$4
lhu	$12,2($2)
beq	$5,$0,$L26
move	$8,$3

$L21:
andi	$4,$8,0xf
sltu	$2,$10,$4
beq	$2,$0,$L114
sll	$2,$24,$4

addiu	$7,$7,1
lbu	$2,0($7)
sll	$2,$2,$10
addiu	$10,$10,8
sltu	$3,$10,$4
beq	$3,$0,$L27
addu	$11,$11,$2

addiu	$7,$7,1
lbu	$2,0($7)
sll	$2,$2,$10
addu	$11,$11,$2
addiu	$10,$10,8
$L27:
sll	$2,$24,$4
$L114:
addiu	$2,$2,-1
and	$2,$11,$2
addu	$6,$12,$2
lw	$2,4($sp)
subu	$10,$10,$4
subu	$3,$13,$2
sltu	$2,$3,$6
beq	$2,$0,$L30
srl	$11,$11,$4

subu	$9,$6,$3
sltu	$2,$fp,$9
bne	$2,$0,$L75
lui	$2,%hi($LC0)

bne	$20,$0,$L35
addiu	$12,$23,-1

lw	$3,0($sp)
subu	$2,$3,$9
sltu	$3,$9,$15
bne	$3,$0,$L37
addu	$8,$12,$2

$L38:
sltu	$2,$15,3
bne	$2,$0,$L40
li	$2,-1431699456			# 0xffffffffaaaa0000

addiu	$3,$15,-3
ori	$2,$2,0xaaab
multu	$3,$2
move	$5,$13
mfhi	$3
srl	$3,$3,1
sll	$2,$3,1
addu	$2,$2,$3
addu	$2,$13,$2
addiu	$4,$2,3
$L55:
lbu	$2,1($8)
sb	$2,1($5)
lbu	$3,2($8)
addiu	$8,$8,3
sb	$3,2($5)
lbu	$2,0($8)
addiu	$5,$5,3
bne	$5,$4,$L55
sb	$2,0($5)

addu	$2,$15,$13
subu	$15,$2,$5
move	$13,$5
$L40:
beq	$15,$0,$L115
sltu	$2,$7,$19

lbu	$2,1($8)
addiu	$13,$13,1
sb	$2,0($13)
li	$2,2			# 0x2
bne	$15,$2,$L13
addiu	$3,$8,1

lbu	$2,1($3)
$L109:
addiu	$13,$13,1
j	$L13
sb	$2,0($13)

$L10:
$L106:
beq	$2,$0,$L100
lui	$2,%hi($LC2)

li	$2,11			# 0xb
sw	$2,0($14)
srl	$2,$10,3
subu	$7,$7,$2
addiu	$3,$7,1
sll	$2,$2,3
addiu	$4,$13,1
sltu	$5,$7,$19
sltu	$8,$13,$17
subu	$6,$10,$2
sw	$3,0($16)
bne	$5,$0,$L101
sw	$4,12($16)

$L68:
subu	$3,$7,$19
li	$2,5			# 0x5
subu	$2,$2,$3
bne	$8,$0,$L102
sw	$2,4($16)

$L71:
subu	$3,$13,$17
li	$2,257			# 0x101
j	$L73
subu	$3,$2,$3

$L99:
lbu	$2,1($7)
$L108:
addiu	$7,$7,2
lbu	$3,0($7)
sll	$2,$2,$10
addiu	$4,$10,8
addu	$2,$11,$2
sll	$3,$3,$4
addu	$11,$2,$3
j	$L19
addiu	$10,$10,16

$L98:
sltu	$2,$10,$4
bne	$2,$0,$L103
sll	$2,$24,$4

addiu	$2,$2,-1
and	$2,$11,$2
addu	$15,$15,$2
subu	$10,$10,$4
srl	$11,$11,$4
$L104:
sltu	$2,$10,15
beq	$2,$0,$L19
nop

j	$L108
lbu	$2,1($7)

$L100:
addiu	$2,$2,%lo($LC2)
li	$3,27			# 0x1b
sltu	$8,$13,$17
sw	$2,24($16)
j	$L34
sw	$3,0($14)

$L24:
addiu	$2,$2,%lo($LC1)
$L116:
li	$3,27			# 0x1b
sltu	$8,$13,$17
sw	$2,24($16)
j	$L34
sw	$3,0($14)

$L30:
subu	$5,$13,$6
$L59:
lbu	$2,1($5)
addiu	$15,$15,-3
sb	$2,1($13)
lbu	$3,2($5)
addiu	$5,$5,3
sb	$3,2($13)
lbu	$4,0($5)
addiu	$13,$13,3
sltu	$2,$15,3
beq	$2,$0,$L59
sb	$4,0($13)

beq	$15,$0,$L115
sltu	$2,$7,$19

lbu	$2,1($5)
addiu	$13,$13,1
sb	$2,0($13)
li	$2,2			# 0x2
bne	$15,$2,$L13
addiu	$3,$5,1

j	$L109
lbu	$2,1($3)

$L103:
addiu	$7,$7,1
lbu	$2,0($7)
sll	$2,$2,$10
addu	$11,$11,$2
sll	$2,$24,$4
addiu	$2,$2,-1
and	$2,$11,$2
addiu	$10,$10,8
addu	$15,$15,$2
subu	$10,$10,$4
j	$L104
srl	$11,$11,$4

$L35:
sltu	$2,$20,$9
bne	$2,$0,$L105
lw	$8,0($sp)

subu	$2,$20,$9
sltu	$3,$9,$15
beq	$3,$0,$L38
addu	$8,$12,$2

move	$5,$13
move	$3,$9
$L53:
addiu	$8,$8,1
lbu	$2,0($8)
addiu	$5,$5,1
addiu	$3,$3,-1
bne	$3,$0,$L53
sb	$2,0($5)

addu	$13,$13,$9
$L110:
subu	$15,$15,$9
j	$L38
subu	$8,$13,$6

$L105:
subu	$4,$9,$20
addu	$2,$8,$20
subu	$2,$2,$9
sltu	$3,$4,$15
beq	$3,$0,$L38
addu	$8,$12,$2

subu	$15,$15,$4
move	$5,$13
move	$3,$4
$L46:
addiu	$8,$8,1
lbu	$2,0($8)
addiu	$5,$5,1
addiu	$3,$3,-1
bne	$3,$0,$L46
sb	$2,0($5)

sltu	$2,$20,$15
bne	$2,$0,$L48
addu	$13,$13,$4

j	$L38
move	$8,$12

$L37:
move	$5,$13
move	$3,$9
$L41:
addiu	$8,$8,1
lbu	$2,0($8)
addiu	$5,$5,1
addiu	$3,$3,-1
bne	$3,$0,$L41
sb	$2,0($5)

j	$L110
addu	$13,$13,$9

$L48:
move	$5,$13
move	$3,$20
$L50:
addiu	$12,$12,1
lbu	$2,0($12)
addiu	$5,$5,1
addiu	$3,$3,-1
bne	$3,$0,$L50
sb	$2,0($5)

addu	$13,$13,$20
subu	$15,$15,$20
j	$L38
subu	$8,$13,$6

$L75:
addiu	$2,$2,%lo($LC0)
li	$3,27			# 0x1b
sltu	$8,$13,$17
sw	$2,24($16)
j	$L34
sw	$3,0($14)

.set	macro
.set	reorder
.end	inflate_fast
.size	inflate_fast, .-inflate_fast
.ident	"GCC: (GNU) 4.1.2"
