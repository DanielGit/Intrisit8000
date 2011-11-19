.file	1 "inflate.c"
.section .mdebug.abi32
.previous
.section	.text.inflateReset,"ax",@progbits
.align	2
.align	5
.globl	inflateReset
.ent	inflateReset
inflateReset:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

bne	$4,$0,$L8
nop

$L2:
j	$31
li	$2,-2			# 0xfffffffffffffffe

$L8:
lw	$3,28($4)
move	$2,$0
beq	$3,$0,$L2
addiu	$5,$3,1320

sw	$0,24($3)
sw	$5,68($3)
sw	$0,20($4)
sw	$0,8($4)
sw	$0,24($4)
sw	$0,0($3)
sw	$0,4($3)
sw	$0,12($3)
sw	$0,32($3)
sw	$0,36($3)
sw	$0,48($3)
sw	$0,52($3)
sw	$5,100($3)
j	$31
sw	$5,72($3)

.set	macro
.set	reorder
.end	inflateReset
.size	inflateReset, .-inflateReset
.section	.text.inflateEnd,"ax",@progbits
.align	2
.align	5
.globl	inflateEnd
.ent	inflateEnd
inflateEnd:
.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
.mask	0x80010000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-24
sw	$16,16($sp)
sw	$31,20($sp)
bne	$4,$0,$L18
move	$16,$4

li	$2,-2			# 0xfffffffffffffffe
$L16:
lw	$31,20($sp)
$L19:
lw	$16,16($sp)
j	$31
addiu	$sp,$sp,24

$L18:
lw	$3,28($4)
beq	$3,$0,$L16
li	$2,-2			# 0xfffffffffffffffe

lw	$6,36($4)
beq	$6,$0,$L19
lw	$31,20($sp)

lw	$2,44($3)
beq	$2,$0,$L14
move	$5,$2

jal	$6
lw	$4,40($4)

lw	$3,28($16)
lw	$6,36($16)
$L14:
lw	$4,40($16)
jal	$6
move	$5,$3

move	$2,$0
j	$L16
sw	$0,28($16)

.set	macro
.set	reorder
.end	inflateEnd
.size	inflateEnd, .-inflateEnd
.section	.text.inflateSyncPoint,"ax",@progbits
.align	2
.align	5
.globl	inflateSyncPoint
.ent	inflateSyncPoint
inflateSyncPoint:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

bne	$4,$0,$L28
nop

li	$4,-2			# 0xfffffffffffffffe
$L26:
$L29:
j	$31
move	$2,$4

$L28:
lw	$5,28($4)
beq	$5,$0,$L29
li	$4,-2			# 0xfffffffffffffffe

lw	$3,0($5)
li	$2,13			# 0xd
bne	$3,$2,$L26
move	$4,$0

lw	$2,52($5)
j	$L26
sltu	$4,$2,1

.set	macro
.set	reorder
.end	inflateSyncPoint
.size	inflateSyncPoint, .-inflateSyncPoint
.section	.text.inflateCopy,"ax",@progbits
.align	2
.align	5
.globl	inflateCopy
.ent	inflateCopy
inflateCopy:
.frame	$sp,40,$31		# vars= 0, regs= 6/0, args= 16, gp= 0
.mask	0x801f0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
sw	$17,20($sp)
sw	$16,16($sp)
sw	$31,36($sp)
sw	$20,32($sp)
sw	$19,28($sp)
sw	$18,24($sp)
move	$17,$4
bne	$4,$0,$L55
move	$16,$5

li	$2,-2			# 0xfffffffffffffffe
$L39:
lw	$31,36($sp)
$L56:
lw	$20,32($sp)
$L57:
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
j	$31
addiu	$sp,$sp,40

$L55:
beq	$5,$0,$L39
li	$2,-2			# 0xfffffffffffffffe

lw	$18,28($5)
beq	$18,$0,$L56
lw	$31,36($sp)

lw	$3,32($5)
beq	$3,$0,$L57
lw	$20,32($sp)

lw	$2,36($5)
beq	$2,$0,$L57
li	$2,-2			# 0xfffffffffffffffe

lw	$4,40($5)
li	$6,7080			# 0x1ba8
jal	$3
li	$5,1			# 0x1

beq	$2,$0,$L54
move	$19,$2

lw	$2,44($18)
beq	$2,$0,$L40
li	$3,1			# 0x1

lw	$5,28($18)
lw	$2,32($16)
lw	$4,40($16)
sll	$5,$3,$5
jal	$2
li	$6,1			# 0x1

beq	$2,$0,$L42
move	$20,$2

$L44:
move	$6,$16
move	$7,$17
addiu	$8,$16,48
$L45:
lw	$2,0($6)
lw	$3,4($6)
lw	$4,8($6)
lw	$5,12($6)
addiu	$6,$6,16
sw	$2,0($7)
sw	$3,4($7)
sw	$4,8($7)
sw	$5,12($7)
bne	$6,$8,$L45
addiu	$7,$7,16

lw	$2,4($6)
lw	$3,0($6)
addiu	$8,$18,7072
sw	$2,4($7)
sw	$3,0($7)
move	$6,$18
move	$7,$19
$L46:
lw	$2,0($6)
lw	$3,4($6)
lw	$4,8($6)
lw	$5,12($6)
addiu	$6,$6,16
sw	$2,0($7)
sw	$3,4($7)
sw	$4,8($7)
sw	$5,12($7)
bne	$6,$8,$L46
addiu	$7,$7,16

lw	$2,4($6)
lw	$3,0($6)
sw	$2,4($7)
sw	$3,0($7)
lw	$4,68($18)
lw	$6,72($18)
lw	$5,100($18)
addiu	$2,$18,1320
addiu	$3,$19,1320
subu	$5,$5,$2
subu	$4,$4,$2
subu	$6,$6,$2
addu	$5,$3,$5
addu	$4,$3,$4
addu	$3,$3,$6
sw	$4,68($19)
sw	$3,72($19)
beq	$20,$0,$L47
sw	$5,100($19)

lw	$2,28($18)
li	$6,1			# 0x1
sll	$6,$6,$2
lw	$5,44($18)
lui	$2,%hi(memcpy)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$4,$20

$L47:
move	$2,$0
sw	$19,28($17)
j	$L39
sw	$20,44($19)

$L42:
lw	$4,40($16)
lw	$2,36($16)
jal	$2
move	$5,$19

$L54:
j	$L39
li	$2,-4			# 0xfffffffffffffffc

$L40:
j	$L44
move	$20,$0

.set	macro
.set	reorder
.end	inflateCopy
.size	inflateCopy, .-inflateCopy
.section	.text.updatewindow,"ax",@progbits
.align	2
.align	5
.ent	updatewindow
updatewindow:
.frame	$sp,40,$31		# vars= 0, regs= 6/0, args= 16, gp= 0
.mask	0x801f0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
sw	$19,28($sp)
sw	$16,16($sp)
sw	$31,36($sp)
sw	$20,32($sp)
sw	$18,24($sp)
sw	$17,20($sp)
lw	$17,28($4)
move	$19,$4
lw	$7,44($17)
beq	$7,$0,$L77
move	$16,$5

$L59:
lw	$6,32($17)
beq	$6,$0,$L78
li	$2,1			# 0x1

lw	$2,16($19)
subu	$16,$16,$2
sltu	$3,$16,$6
beq	$3,$0,$L79
nop

$L65:
lw	$4,40($17)
subu	$18,$6,$4
sltu	$2,$16,$18
bne	$2,$0,$L67
lui	$2,%hi(memcpy)

lw	$5,12($19)
subu	$5,$5,$16
addu	$4,$4,$7
subu	$16,$16,$18
addiu	$20,$2,%lo(memcpy)
jal	$20
move	$6,$18

bne	$16,$0,$L80
nop

lw	$2,40($17)
lw	$6,32($17)
addu	$2,$2,$18
beq	$2,$6,$L81
sw	$2,40($17)

$L71:
lw	$3,36($17)
sltu	$2,$3,$6
beq	$2,$0,$L83
move	$4,$0

$L73:
addu	$2,$3,$18
move	$4,$0
sw	$2,36($17)
$L62:
lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L78:
lw	$3,28($17)
sll	$6,$2,$3
sw	$6,32($17)
sw	$0,40($17)
sw	$0,36($17)
lw	$2,16($19)
subu	$16,$16,$2
sltu	$3,$16,$6
bne	$3,$0,$L65
nop

$L79:
lw	$5,12($19)
lui	$2,%hi(memcpy)
move	$4,$7
addiu	$2,$2,%lo(memcpy)
jal	$2
subu	$5,$5,$6

lw	$3,32($17)
sw	$0,40($17)
sw	$3,36($17)
move	$4,$0
lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L67:
lw	$5,12($19)
move	$6,$16
addu	$4,$4,$7
addiu	$2,$2,%lo(memcpy)
jal	$2
subu	$5,$5,$16

lw	$2,40($17)
move	$18,$16
lw	$6,32($17)
addu	$2,$2,$18
bne	$2,$6,$L71
sw	$2,40($17)

$L81:
lw	$3,36($17)
sltu	$2,$3,$6
bne	$2,$0,$L73
sw	$0,40($17)

move	$4,$0
$L83:
lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L80:
lw	$5,12($19)
lw	$4,44($17)
subu	$5,$5,$16
jal	$20
move	$6,$16

lw	$3,32($17)
sw	$16,40($17)
sw	$3,36($17)
move	$4,$0
lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L77:
lw	$5,28($17)
lw	$2,32($4)
li	$3,1			# 0x1
lw	$4,40($4)
sll	$5,$3,$5
jal	$2
li	$6,1			# 0x1

move	$7,$2
bne	$2,$0,$L59
sw	$2,44($17)

j	$L62
li	$4,1			# 0x1

.set	macro
.set	reorder
.end	updatewindow
.size	updatewindow, .-updatewindow
.section	.text.inflateSetDictionary,"ax",@progbits
.align	2
.align	5
.globl	inflateSetDictionary
.ent	inflateSetDictionary
inflateSetDictionary:
.frame	$sp,40,$31		# vars= 0, regs= 6/0, args= 16, gp= 0
.mask	0x801f0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
sw	$19,28($sp)
sw	$18,24($sp)
sw	$17,20($sp)
sw	$31,36($sp)
sw	$20,32($sp)
sw	$16,16($sp)
move	$17,$4
move	$18,$5
bne	$4,$0,$L98
move	$19,$6

li	$3,-2			# 0xfffffffffffffffe
$L91:
lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$3
j	$31
addiu	$sp,$sp,40

$L98:
lw	$20,28($4)
beq	$20,$0,$L91
li	$3,-2			# 0xfffffffffffffffe

lw	$3,0($20)
li	$2,10			# 0xa
bne	$3,$2,$L91
li	$3,-2			# 0xfffffffffffffffe

lui	$16,%hi(adler32)
addiu	$16,$16,%lo(adler32)
move	$4,$0
move	$5,$0
jal	$16
move	$6,$0

move	$4,$2
move	$5,$18
jal	$16
move	$6,$19

lw	$3,20($20)
bne	$2,$3,$L91
li	$3,-3			# 0xfffffffffffffffd

lui	$2,%hi(updatewindow)
lw	$5,16($17)
addiu	$2,$2,%lo(updatewindow)
jal	$2
move	$4,$17

bne	$2,$0,$L99
li	$2,28			# 0x1c

lw	$6,32($20)
sltu	$2,$6,$19
beq	$2,$0,$L94
lui	$2,%hi(memcpy)

addu	$5,$18,$19
lw	$4,44($20)
addiu	$2,$2,%lo(memcpy)
jal	$2
subu	$5,$5,$6

lw	$3,32($20)
sw	$3,36($20)
$L96:
li	$2,1			# 0x1
move	$3,$0
j	$L91
sw	$2,12($20)

$L99:
li	$3,-4			# 0xfffffffffffffffc
j	$L91
sw	$2,0($20)

$L94:
lw	$4,44($20)
addu	$4,$6,$4
subu	$4,$4,$19
move	$5,$18
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$19

j	$L96
sw	$19,36($20)

.set	macro
.set	reorder
.end	inflateSetDictionary
.size	inflateSetDictionary, .-inflateSetDictionary
.section	.rodata.str1.4,"aMS",@progbits,1
.align	2
$LC0:
.ascii	"incorrect header check\000"
.align	2
$LC1:
.ascii	"unknown compression method\000"
.align	2
$LC2:
.ascii	"invalid window size\000"
.align	2
$LC3:
.ascii	"unknown header flags set\000"
.align	2
$LC4:
.ascii	"header crc mismatch\000"
.align	2
$LC5:
.ascii	"invalid block type\000"
.align	2
$LC6:
.ascii	"invalid stored block lengths\000"
.align	2
$LC7:
.ascii	"too many length or distance symbols\000"
.align	2
$LC8:
.ascii	"invalid code lengths set\000"
.align	2
$LC9:
.ascii	"invalid bit length repeat\000"
.align	2
$LC10:
.ascii	"invalid literal/lengths set\000"
.align	2
$LC11:
.ascii	"invalid distances set\000"
.align	2
$LC12:
.ascii	"invalid literal/length code\000"
.align	2
$LC13:
.ascii	"invalid distance code\000"
.align	2
$LC14:
.ascii	"invalid distance too far back\000"
.align	2
$LC15:
.ascii	"incorrect data check\000"
.align	2
$LC16:
.ascii	"incorrect length check\000"
.section	.text.inflate,"ax",@progbits
.align	2
.align	5
.globl	inflate
.ent	inflate
inflate:
.frame	$sp,96,$31		# vars= 32, regs= 10/0, args= 24, gp= 0
.mask	0xc0ff0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-96
sw	$17,60($sp)
sw	$31,92($sp)
sw	$fp,88($sp)
sw	$23,84($sp)
sw	$22,80($sp)
sw	$21,76($sp)
sw	$20,72($sp)
sw	$19,68($sp)
sw	$18,64($sp)
sw	$16,56($sp)
move	$17,$4
bne	$4,$0,$L483
sw	$5,100($sp)

$L101:
li	$19,-2			# 0xfffffffffffffffe
$L140:
move	$2,$19
$L501:
lw	$31,92($sp)
lw	$fp,88($sp)
lw	$23,84($sp)
lw	$22,80($sp)
lw	$21,76($sp)
lw	$20,72($sp)
lw	$19,68($sp)
lw	$18,64($sp)
lw	$17,60($sp)
lw	$16,56($sp)
j	$31
addiu	$sp,$sp,96

$L483:
lw	$18,28($4)
beq	$18,$0,$L140
li	$19,-2			# 0xfffffffffffffffe

lw	$4,12($4)
beq	$4,$0,$L501
move	$2,$19

lw	$5,0($17)
beq	$5,$0,$L484
nop

lw	$3,4($17)
$L105:
lw	$7,0($18)
li	$2,11			# 0xb
beq	$7,$2,$L485
nop

$L107:
lw	$6,16($17)
lw	$21,48($18)
lw	$16,52($18)
sw	$6,40($sp)
sw	$3,32($sp)
sw	$4,44($sp)
move	$23,$5
move	$22,$3
sw	$6,36($sp)
move	$19,$0
$L481:
sltu	$2,$7,29
beq	$2,$0,$L101
lui	$3,%hi($L139)

sll	$2,$7,2
addiu	$3,$3,%lo($L139)
addu	$2,$2,$3
lw	$4,0($2)
j	$4
nop

.rdata
.align	2
.align	2
$L139:
.word	$L110
.word	$L111
.word	$L112
.word	$L113
.word	$L412
.word	$L413
.word	$L414
.word	$L415
.word	$L416
.word	$L119
.word	$L120
.word	$L121
.word	$L122
.word	$L123
.word	$L124
.word	$L125
.word	$L126
.word	$L417
.word	$L128
.word	$L129
.word	$L130
.word	$L131
.word	$L132
.word	$L133
.word	$L134
.word	$L418
.word	$L480
.word	$L137
.word	$L138
.section	.text.inflate
$L122:
lw	$2,4($18)
bne	$2,$0,$L230
andi	$3,$16,0x7

sltu	$2,$16,3
beq	$2,$0,$L502
srl	$4,$21,1

$L235:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,3
addu	$21,$21,$2
bne	$3,$0,$L235
addiu	$23,$23,1

srl	$4,$21,1
$L502:
andi	$3,$21,0x1
andi	$6,$4,0x3
li	$2,1			# 0x1
beq	$6,$2,$L238
sw	$3,4($18)

beq	$6,$0,$L237
li	$7,13			# 0xd

li	$2,2			# 0x2
beq	$6,$2,$L239
li	$2,3			# 0x3

beq	$6,$2,$L486
lui	$2,%hi($LC5)

$L236:
lw	$7,0($18)
srl	$21,$4,2
j	$L481
addiu	$16,$16,-3

$L119:
sltu	$2,$16,32
beq	$2,$0,$L504
srl	$4,$21,8

$L227:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,32
addu	$21,$21,$2
bne	$3,$0,$L227
addiu	$23,$23,1

srl	$4,$21,8
$L504:
andi	$3,$21,0xff00
sll	$5,$21,24
srl	$2,$21,24
andi	$4,$4,0xff00
sll	$3,$3,8
addu	$2,$2,$4
addu	$3,$3,$5
addu	$2,$2,$3
li	$7,10			# 0xa
sw	$2,20($18)
sw	$7,0($18)
sw	$2,48($17)
move	$21,$0
move	$16,$0
$L120:
lw	$2,12($18)
beq	$2,$0,$L424
lui	$2,%hi(adler32)

move	$4,$0
move	$5,$0
addiu	$2,$2,%lo(adler32)
jal	$2
move	$6,$0

li	$7,11			# 0xb
sw	$2,20($18)
sw	$7,0($18)
sw	$2,48($17)
$L121:
lw	$3,100($sp)
li	$2,5			# 0x5
bne	$3,$2,$L122
nop

$L141:
lw	$4,40($sp)
$L503:
sw	$22,4($17)
sw	$4,16($17)
lw	$5,44($sp)
lw	$2,32($18)
sw	$5,12($17)
sw	$23,0($17)
sw	$21,48($18)
bne	$2,$0,$L387
sw	$16,52($18)

lw	$2,0($18)
sltu	$2,$2,24
beq	$2,$0,$L423
lw	$6,36($sp)

lw	$5,16($17)
beq	$6,$5,$L505
subu	$16,$6,$5

$L387:
lui	$2,%hi(updatewindow)
lw	$5,36($sp)
addiu	$2,$2,%lo(updatewindow)
jal	$2
move	$4,$17

bne	$2,$0,$L487
li	$2,28			# 0x1c

$L423:
lw	$5,16($17)
lw	$6,36($sp)
subu	$16,$6,$5
$L505:
lw	$4,4($17)
lw	$5,32($sp)
lw	$2,8($17)
lw	$3,20($17)
subu	$20,$5,$4
addu	$2,$2,$20
addu	$3,$3,$16
sw	$3,20($17)
sw	$2,8($17)
lw	$2,24($18)
lw	$3,8($18)
addu	$2,$2,$16
beq	$3,$0,$L392
sw	$2,24($18)

beq	$16,$0,$L392
nop

lw	$2,16($18)
beq	$2,$0,$L395
lui	$2,%hi(adler32)

lw	$5,12($17)
lui	$2,%hi(crc32)
lw	$4,20($18)
subu	$5,$5,$16
addiu	$2,$2,%lo(crc32)
jal	$2
move	$6,$16

sw	$2,20($18)
$L500:
sw	$2,48($17)
$L392:
lw	$4,0($18)
lw	$6,52($18)
lw	$5,4($18)
xori	$4,$4,0xb
li	$3,128			# 0x80
movn	$3,$0,$4
li	$2,64			# 0x40
movz	$2,$0,$5
addu	$3,$3,$6
addu	$2,$2,$3
bne	$20,$0,$L404
sw	$2,44($17)

bne	$16,$0,$L506
lw	$6,100($sp)

$L406:
bne	$19,$0,$L501
move	$2,$19

j	$L140
li	$19,-5			# 0xfffffffffffffffb

$L404:
lw	$6,100($sp)
$L506:
li	$2,4			# 0x4
bne	$6,$2,$L501
move	$2,$19

j	$L406
nop

$L496:
lw	$3,100($18)
lw	$5,88($18)
lw	$4,48($sp)
li	$2,6			# 0x6
sw	$3,72($18)
sw	$2,80($18)
lw	$6,92($18)
addiu	$2,$18,80
sll	$5,$5,1
lui	$3,%hi(inflate_table)
addu	$5,$4,$5
move	$7,$fp
li	$4,2			# 0x2
sw	$2,16($sp)
addiu	$3,$3,%lo(inflate_table)
jal	$3
sw	$20,20($sp)

bne	$2,$0,$L426
move	$19,$2

li	$7,18			# 0x12
sw	$7,0($18)
$L128:
sltu	$2,$22,6
bne	$2,$0,$L302
lw	$4,40($sp)

sltu	$2,$4,258
beq	$2,$0,$L488
lw	$5,44($sp)

$L302:
lw	$3,76($18)
li	$2,1			# 0x1
sll	$2,$2,$3
addiu	$7,$2,-1
lw	$9,68($18)
and	$2,$21,$7
sll	$2,$2,2
addu	$2,$9,$2
lbu	$4,1($2)
lbu	$6,0($2)
sltu	$3,$16,$4
bne	$3,$0,$L307
lhu	$5,2($2)

$L303:
beq	$6,$0,$L308
andi	$2,$6,0xf0

bne	$2,$0,$L310
li	$2,1			# 0x1

addu	$3,$6,$4
sll	$2,$2,$3
addiu	$10,$2,-1
and	$3,$10,$21
srl	$3,$3,$4
addu	$3,$5,$3
sll	$3,$3,2
addu	$3,$9,$3
lbu	$7,1($3)
lbu	$6,0($3)
addu	$2,$4,$7
sltu	$2,$16,$2
bne	$2,$0,$L315
lhu	$8,2($3)

subu	$2,$16,$4
$L499:
srl	$3,$21,$4
subu	$16,$2,$7
sw	$8,56($18)
beq	$6,$0,$L317
srl	$21,$3,$7

$L318:
andi	$2,$6,0x20
bne	$2,$0,$L507
li	$7,11			# 0xb

andi	$2,$6,0x40
bne	$2,$0,$L428
lui	$2,%hi($LC12)

andi	$4,$6,0xf
li	$7,19			# 0x13
sw	$4,64($18)
sw	$7,0($18)
$L129:
lw	$4,64($18)
beq	$4,$0,$L508
li	$7,20			# 0x14

sltu	$2,$16,$4
bne	$2,$0,$L328
li	$2,1			# 0x1

$L497:
sll	$2,$2,$4
lw	$3,56($18)
addiu	$2,$2,-1
and	$2,$2,$21
addu	$3,$3,$2
sw	$3,56($18)
subu	$16,$16,$4
srl	$21,$21,$4
li	$7,20			# 0x14
$L508:
sw	$7,0($18)
$L130:
lw	$3,80($18)
li	$2,1			# 0x1
sll	$2,$2,$3
addiu	$7,$2,-1
lw	$8,72($18)
and	$2,$21,$7
sll	$2,$2,2
addu	$2,$8,$2
lbu	$5,1($2)
lbu	$4,0($2)
sltu	$3,$16,$5
beq	$3,$0,$L329
lhu	$6,2($2)

$L332:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addu	$21,$21,$2
and	$3,$21,$7
sll	$3,$3,2
addu	$3,$3,$8
lbu	$5,1($3)
addiu	$16,$16,8
sltu	$2,$16,$5
addiu	$23,$23,1
lbu	$4,0($3)
bne	$2,$0,$L332
lhu	$6,2($3)

$L329:
andi	$2,$4,0xf0
beq	$2,$0,$L333
li	$2,1			# 0x1

move	$7,$5
$L335:
andi	$2,$4,0x40
srl	$21,$21,$7
bne	$2,$0,$L429
subu	$16,$16,$5

andi	$4,$4,0xf
li	$7,21			# 0x15
sw	$6,60($18)
sw	$4,64($18)
sw	$7,0($18)
$L131:
lw	$4,64($18)
beq	$4,$0,$L420
sltu	$2,$16,$4

beq	$2,$0,$L509
li	$2,1			# 0x1

$L347:
beq	$22,$0,$L141
nop

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,$4
addu	$21,$21,$2
bne	$3,$0,$L347
addiu	$23,$23,1

li	$2,1			# 0x1
$L509:
sll	$2,$2,$4
lw	$3,60($18)
addiu	$2,$2,-1
and	$2,$2,$21
addu	$3,$3,$2
subu	$16,$16,$4
srl	$21,$21,$4
sw	$3,60($18)
$L342:
lw	$2,36($18)
lw	$4,36($sp)
lw	$5,40($sp)
addu	$2,$4,$2
subu	$2,$2,$5
sltu	$2,$2,$3
bne	$2,$0,$L430
lui	$2,%hi($LC14)

li	$7,22			# 0x16
sw	$7,0($18)
$L132:
lw	$6,40($sp)
beq	$6,$0,$L141
lw	$2,36($sp)

lw	$5,60($18)
subu	$3,$2,$6
sltu	$2,$3,$5
beq	$2,$0,$L351
lw	$4,44($sp)

lw	$4,40($18)
subu	$6,$5,$3
sltu	$2,$4,$6
beq	$2,$0,$L353
subu	$2,$4,$6

lw	$2,32($18)
subu	$6,$6,$4
lw	$3,44($18)
subu	$2,$2,$6
addu	$5,$2,$3
$L355:
lw	$3,56($18)
sltu	$2,$3,$6
bne	$2,$0,$L356
move	$4,$3

move	$4,$6
$L356:
lw	$6,40($sp)
sltu	$2,$4,$6
movn	$6,$4,$2
lw	$2,40($sp)
subu	$3,$3,$6
subu	$2,$2,$6
lw	$4,44($sp)
sw	$3,56($18)
sw	$2,40($sp)
move	$3,$6
$L358:
lbu	$2,0($5)
addiu	$3,$3,-1
sb	$2,0($4)
addiu	$5,$5,1
bne	$3,$0,$L358
addiu	$4,$4,1

lw	$3,44($sp)
lw	$2,56($18)
addu	$3,$3,$6
bne	$2,$0,$L421
sw	$3,44($sp)

li	$7,18			# 0x12
j	$L481
sw	$7,0($18)

$L245:
li	$7,14			# 0xe
sw	$3,56($18)
sw	$7,0($18)
move	$21,$0
move	$16,$0
$L124:
lw	$3,56($18)
beq	$3,$0,$L247
li	$7,11			# 0xb

sltu	$2,$3,$22
move	$4,$3
lw	$5,40($sp)
movz	$4,$22,$2
sltu	$3,$4,$5
move	$20,$5
movn	$20,$4,$3
beq	$20,$0,$L141
lw	$4,44($sp)

lui	$2,%hi(memcpy)
move	$5,$23
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$20

lw	$3,56($18)
lw	$6,40($sp)
lw	$2,44($sp)
lw	$7,0($18)
subu	$3,$3,$20
subu	$6,$6,$20
addu	$2,$2,$20
subu	$22,$22,$20
addu	$23,$23,$20
sw	$6,40($sp)
sw	$2,44($sp)
j	$L481
sw	$3,56($18)

$L125:
sltu	$2,$16,14
beq	$2,$0,$L510
andi	$4,$21,0x1f

$L250:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,14
addu	$21,$21,$2
bne	$3,$0,$L250
addiu	$23,$23,1

andi	$4,$21,0x1f
$L510:
srl	$2,$21,5
srl	$3,$21,10
addiu	$6,$4,257
andi	$2,$2,0x1f
andi	$3,$3,0xf
addiu	$11,$2,1
addiu	$7,$3,4
sltu	$2,$6,287
sw	$7,84($18)
sw	$6,88($18)
sw	$11,92($18)
srl	$21,$21,14
beq	$2,$0,$L251
addiu	$16,$16,-14

sltu	$2,$11,31
beq	$2,$0,$L511
lui	$2,%hi($LC7)

li	$7,16			# 0x10
sw	$7,0($18)
sw	$0,96($18)
$L126:
lw	$8,96($18)
lw	$7,84($18)
sltu	$2,$8,$7
beq	$2,$0,$L254
lui	$2,%hi(order.1615)

sll	$3,$8,1
addiu	$2,$2,%lo(order.1615)
addu	$5,$3,$2
$L256:
sltu	$2,$16,3
beq	$2,$0,$L259
nop

$L257:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,3
addu	$21,$21,$2
bne	$3,$0,$L257
addiu	$23,$23,1

$L259:
lhu	$2,0($5)
addiu	$8,$8,1
sll	$2,$2,1
andi	$3,$21,0x7
addu	$2,$2,$18
sltu	$4,$8,$7
sh	$3,104($2)
sw	$8,96($18)
srl	$21,$21,3
addiu	$16,$16,-3
bne	$4,$0,$L256
addiu	$5,$5,2

$L254:
sltu	$2,$8,19
beq	$2,$0,$L261
lui	$2,%hi(order.1615)

sll	$3,$8,1
addiu	$2,$2,%lo(order.1615)
addu	$3,$3,$2
li	$4,19			# 0x13
$L263:
lhu	$2,0($3)
addiu	$8,$8,1
sll	$2,$2,1
addu	$2,$2,$18
sh	$0,104($2)
bne	$8,$4,$L263
addiu	$3,$3,2

sw	$8,96($18)
$L261:
li	$2,7			# 0x7
sw	$2,76($18)
addiu	$3,$18,1320
addiu	$2,$18,76
sw	$3,68($18)
sw	$3,100($18)
addiu	$3,$18,744
sw	$2,16($sp)
lui	$2,%hi(inflate_table)
move	$4,$0
addiu	$5,$18,104
li	$6,19			# 0x13
addiu	$7,$18,100
addiu	$2,$2,%lo(inflate_table)
jal	$2
sw	$3,20($sp)

beq	$2,$0,$L264
move	$19,$2

lui	$2,%hi($LC8)
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC8)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L328:
beq	$22,$0,$L141
nop

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,$4
addu	$21,$21,$2
bne	$3,$0,$L328
addiu	$23,$23,1

j	$L497
li	$2,1			# 0x1

$L137:
j	$L141
li	$19,-3			# 0xfffffffffffffffd

$L138:
j	$L140
li	$19,-4			# 0xfffffffffffffffc

$L123:
li	$3,-8			# 0xfffffffffffffff8
andi	$2,$16,0x7
and	$16,$16,$3
sltu	$4,$16,32
beq	$4,$0,$L241
srl	$21,$21,$2

$L244:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,32
addu	$21,$21,$2
bne	$3,$0,$L244
addiu	$23,$23,1

$L241:
srl	$2,$21,16
xori	$2,$2,0xffff
andi	$3,$21,0xffff
beq	$3,$2,$L245
lui	$2,%hi($LC6)

li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC6)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L417:
lw	$8,96($18)
lw	$6,88($18)
lw	$11,92($18)
$L482:
addu	$10,$6,$11
sltu	$2,$8,$10
beq	$2,$0,$L280
li	$2,1			# 0x1

lw	$3,76($18)
sll	$2,$2,$3
addiu	$7,$2,-1
lw	$9,68($18)
and	$2,$21,$7
sll	$2,$2,2
addu	$2,$9,$2
lbu	$4,1($2)
sltu	$3,$16,$4
beq	$3,$0,$L269
lhu	$5,2($2)

$L268:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addu	$21,$21,$2
and	$3,$21,$7
sll	$3,$3,2
addu	$3,$3,$9
lbu	$4,1($3)
addiu	$16,$16,8
sltu	$2,$16,$4
addiu	$23,$23,1
bne	$2,$0,$L268
lhu	$5,2($3)

$L269:
sltu	$2,$5,16
bne	$2,$0,$L270
sll	$2,$8,1

li	$2,16			# 0x10
beq	$5,$2,$L489
li	$2,17			# 0x11

beq	$5,$2,$L490
addiu	$5,$4,7

sltu	$2,$16,$5
beq	$2,$0,$L512
srl	$2,$21,$4

$L291:
beq	$22,$0,$L141
nop

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,$5
addu	$21,$21,$2
bne	$3,$0,$L291
addiu	$23,$23,1

srl	$2,$21,$4
$L512:
andi	$3,$2,0x7f
subu	$4,$16,$4
addiu	$16,$4,-7
addiu	$3,$3,11
srl	$21,$2,7
j	$L281
move	$4,$0

$L412:
lw	$4,16($18)
$L114:
andi	$2,$4,0x400
beq	$2,$0,$L513
li	$7,5			# 0x5

sltu	$2,$16,16
beq	$2,$0,$L514
andi	$2,$4,0x200

$L194:
beq	$22,$0,$L141
nop

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,16
addu	$21,$21,$2
bne	$3,$0,$L194
addiu	$23,$23,1

andi	$2,$4,0x200
$L514:
bne	$2,$0,$L491
sw	$21,56($18)

move	$21,$0
move	$16,$0
$L189:
li	$7,5			# 0x5
$L513:
j	$L115
sw	$7,0($18)

$L413:
lw	$4,16($18)
$L115:
andi	$2,$4,0x400
beq	$2,$0,$L515
li	$7,6			# 0x6

lw	$3,56($18)
sltu	$2,$3,$22
move	$20,$3
movz	$20,$22,$2
beq	$20,$0,$L199
andi	$2,$4,0x200

beq	$2,$0,$L516
subu	$3,$3,$20

lw	$4,20($18)
lui	$2,%hi(crc32)
move	$5,$23
addiu	$2,$2,%lo(crc32)
jal	$2
move	$6,$20

lw	$3,56($18)
sw	$2,20($18)
subu	$3,$3,$20
$L516:
subu	$22,$22,$20
addu	$23,$23,$20
sw	$3,56($18)
$L199:
bne	$3,$0,$L141
li	$7,6			# 0x6

lw	$4,16($18)
$L515:
j	$L116
sw	$7,0($18)

$L133:
lw	$4,40($sp)
beq	$4,$0,$L503
lw	$5,44($sp)

lw	$2,56($18)
li	$7,18			# 0x12
sb	$2,0($5)
addiu	$4,$4,-1
addiu	$5,$5,1
sw	$7,0($18)
sw	$5,44($sp)
j	$L481
sw	$4,40($sp)

$L110:
lw	$4,8($18)
beq	$4,$0,$L151
li	$7,12			# 0xc

sltu	$2,$16,16
beq	$2,$0,$L517
andi	$2,$4,0x2

$L156:
beq	$22,$0,$L141
nop

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,16
addu	$21,$21,$2
bne	$3,$0,$L156
addiu	$23,$23,1

andi	$2,$4,0x2
$L517:
beq	$2,$0,$L518
andi	$2,$4,0x1

li	$2,35615			# 0x8b1f
beq	$21,$2,$L492
andi	$2,$4,0x1

$L518:
beq	$2,$0,$L160
sw	$0,16($18)

andi	$4,$21,0xff
srl	$3,$21,8
sll	$4,$4,8
li	$2,138543104			# 0x8420000
addu	$4,$4,$3
ori	$2,$2,0x1085
multu	$4,$2
mfhi	$2
subu	$3,$4,$2
srl	$3,$3,1
addu	$2,$2,$3
srl	$2,$2,4
sll	$3,$2,5
subu	$3,$3,$2
bne	$4,$3,$L519
lui	$2,%hi($LC0)

andi	$3,$21,0xf
li	$2,8			# 0x8
bne	$3,$2,$L520
lui	$2,%hi($LC1)

srl	$21,$21,4
andi	$2,$21,0xf
lw	$3,28($18)
addiu	$2,$2,8
sltu	$2,$3,$2
beq	$2,$0,$L165
lui	$2,%hi($LC2)

li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC2)
addiu	$16,$16,-4
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L111:
sltu	$2,$16,16
beq	$2,$0,$L521
andi	$3,$21,0xff

$L170:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,16
addu	$21,$21,$2
bne	$3,$0,$L170
addiu	$23,$23,1

andi	$3,$21,0xff
$L521:
li	$2,8			# 0x8
beq	$3,$2,$L171
sw	$21,16($18)

lui	$2,%hi($LC1)
$L520:
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC1)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L307:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addu	$21,$21,$2
and	$3,$21,$7
sll	$3,$3,2
addu	$3,$3,$9
lbu	$4,1($3)
addiu	$16,$16,8
sltu	$2,$16,$4
addiu	$23,$23,1
lbu	$6,0($3)
bne	$2,$0,$L307
lhu	$5,2($3)

j	$L303
nop

$L416:
lw	$4,16($18)
$L118:
andi	$2,$4,0x200
beq	$2,$0,$L219
sltu	$2,$16,16

beq	$2,$0,$L221
nop

$L224:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,16
addu	$21,$21,$2
bne	$3,$0,$L224
addiu	$23,$23,1

$L221:
lhu	$2,20($18)
beq	$2,$21,$L225
lui	$2,%hi($LC4)

li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC4)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L418:
lw	$4,8($18)
$L135:
beq	$4,$0,$L522
li	$7,26			# 0x1a

lw	$4,16($18)
beq	$4,$0,$L522
sltu	$2,$16,32

beq	$2,$0,$L381
nop

$L384:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,32
addu	$21,$21,$2
bne	$3,$0,$L384
addiu	$23,$23,1

$L381:
lw	$2,24($18)
beq	$2,$21,$L493
lui	$2,%hi($LC16)

li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC16)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L493:
move	$21,$0
move	$16,$0
li	$7,26			# 0x1a
$L522:
sw	$7,0($18)
$L480:
j	$L141
li	$19,1			# 0x1

$L134:
lw	$4,8($18)
beq	$4,$0,$L523
li	$7,25			# 0x19

sltu	$2,$16,32
beq	$2,$0,$L524
lw	$2,40($sp)

$L367:
beq	$22,$0,$L141
nop

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,32
addu	$21,$21,$2
bne	$3,$0,$L367
addiu	$23,$23,1

lw	$2,40($sp)
$L524:
lw	$6,36($sp)
lw	$3,20($17)
subu	$5,$6,$2
addu	$3,$3,$5
sw	$3,20($17)
lw	$2,24($18)
addu	$2,$2,$5
beq	$5,$0,$L368
sw	$2,24($18)

lw	$2,16($18)
beq	$2,$0,$L370
lw	$2,44($sp)

lw	$3,44($sp)
lw	$4,20($18)
lui	$2,%hi(crc32)
move	$6,$5
addiu	$2,$2,%lo(crc32)
jal	$2
subu	$5,$3,$5

sw	$2,20($18)
sw	$2,48($17)
$L368:
lw	$4,16($18)
beq	$4,$0,$L373
srl	$4,$21,8

move	$3,$21
$L375:
lw	$2,20($18)
beq	$3,$2,$L494
lw	$2,40($sp)

li	$7,27			# 0x1b
sw	$2,36($sp)
lui	$2,%hi($LC15)
addiu	$2,$2,%lo($LC15)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L112:
sltu	$2,$16,32
beq	$2,$0,$L178
nop

$L180:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,32
addu	$21,$21,$2
bne	$3,$0,$L180
addiu	$23,$23,1

$L178:
lw	$2,16($18)
andi	$2,$2,0x200
beq	$2,$0,$L525
li	$7,3			# 0x3

srl	$4,$21,24
srl	$2,$21,8
srl	$3,$21,16
sb	$2,25($sp)
sb	$3,26($sp)
sb	$4,27($sp)
sb	$21,24($sp)
lw	$4,20($18)
lui	$2,%hi(crc32)
addiu	$5,$sp,24
addiu	$2,$2,%lo(crc32)
jal	$2
li	$6,4			# 0x4

sw	$2,20($18)
li	$7,3			# 0x3
$L525:
move	$16,$0
move	$21,$0
sw	$7,0($18)
$L186:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,16
addu	$21,$21,$2
bne	$3,$0,$L186
addiu	$23,$23,1

lw	$4,16($18)
$L498:
andi	$2,$4,0x200
beq	$2,$0,$L526
li	$7,4			# 0x4

srl	$2,$21,8
sb	$2,25($sp)
sb	$21,24($sp)
lw	$4,20($18)
lui	$2,%hi(crc32)
addiu	$5,$sp,24
addiu	$2,$2,%lo(crc32)
jal	$2
li	$6,2			# 0x2

lw	$4,16($18)
sw	$2,20($18)
li	$7,4			# 0x4
$L526:
move	$21,$0
move	$16,$0
j	$L114
sw	$7,0($18)

$L113:
sltu	$2,$16,16
bne	$2,$0,$L186
nop

j	$L498
lw	$4,16($18)

$L414:
lw	$4,16($18)
$L116:
andi	$2,$4,0x800
beq	$2,$0,$L527
li	$7,7			# 0x7

beq	$22,$0,$L141
move	$20,$0

addu	$2,$23,$20
$L528:
lbu	$fp,0($2)
beq	$fp,$0,$L207
addiu	$20,$20,1

bne	$22,$20,$L528
addu	$2,$23,$20

$L207:
andi	$2,$4,0x2000
bne	$2,$0,$L495
lui	$2,%hi(crc32)

$L209:
addu	$23,$23,$20
bne	$fp,$0,$L141
subu	$22,$22,$20

lw	$4,16($18)
li	$7,7			# 0x7
$L527:
sw	$7,0($18)
$L117:
andi	$2,$4,0x1000
beq	$2,$0,$L529
li	$7,8			# 0x8

beq	$22,$0,$L141
move	$20,$0

addu	$2,$23,$20
$L530:
lbu	$fp,0($2)
beq	$fp,$0,$L215
addiu	$20,$20,1

bne	$22,$20,$L530
addu	$2,$23,$20

$L215:
andi	$2,$4,0x2000
beq	$2,$0,$L217
lui	$2,%hi(crc32)

lw	$4,20($18)
move	$5,$23
addiu	$2,$2,%lo(crc32)
jal	$2
move	$6,$20

sw	$2,20($18)
$L217:
addu	$23,$23,$20
bne	$fp,$0,$L141
subu	$22,$22,$20

lw	$4,16($18)
li	$7,8			# 0x8
$L529:
j	$L118
sw	$7,0($18)

$L415:
j	$L117
lw	$4,16($18)

$L315:
beq	$22,$0,$L141
nop

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addu	$21,$21,$2
and	$3,$21,$10
srl	$3,$3,$4
addu	$3,$3,$5
sll	$3,$3,2
addu	$3,$3,$9
lbu	$7,1($3)
addiu	$16,$16,8
addu	$2,$7,$4
sltu	$2,$16,$2
addiu	$23,$23,1
lbu	$6,0($3)
bne	$2,$0,$L315
lhu	$8,2($3)

j	$L499
subu	$2,$16,$4

$L487:
li	$19,-4			# 0xfffffffffffffffc
j	$L140
sw	$2,0($18)

$L491:
srl	$2,$21,8
sb	$2,25($sp)
sb	$21,24($sp)
lw	$4,20($18)
lui	$2,%hi(crc32)
addiu	$5,$sp,24
addiu	$2,$2,%lo(crc32)
jal	$2
li	$6,2			# 0x2

lw	$4,16($18)
move	$21,$0
move	$16,$0
j	$L189
sw	$2,20($18)

$L485:
li	$7,12			# 0xc
j	$L107
sw	$7,0($18)

$L484:
lw	$3,4($17)
beq	$3,$0,$L105
li	$19,-2			# 0xfffffffffffffffe

j	$L501
move	$2,$19

$L225:
move	$21,$0
move	$16,$0
$L219:
lui	$2,%hi(crc32)
move	$4,$0
move	$5,$0
addiu	$2,$2,%lo(crc32)
jal	$2
move	$6,$0

li	$7,11			# 0xb
sw	$2,20($18)
sw	$7,0($18)
j	$L481
sw	$2,48($17)

$L230:
li	$2,-8			# 0xfffffffffffffff8
li	$7,24			# 0x18
srl	$21,$21,$3
and	$16,$16,$2
j	$L481
sw	$7,0($18)

$L395:
lw	$5,12($17)
lw	$4,20($18)
subu	$5,$5,$16
addiu	$2,$2,%lo(adler32)
jal	$2
move	$6,$16

j	$L500
sw	$2,20($18)

$L420:
j	$L342
lw	$3,60($18)

$L351:
lw	$3,56($18)
subu	$5,$4,$5
j	$L356
move	$4,$3

$L421:
j	$L481
lw	$7,0($18)

$L494:
lw	$3,40($sp)
lw	$4,8($18)
sw	$3,36($sp)
move	$21,$0
move	$16,$0
li	$7,25			# 0x19
$L523:
j	$L135
sw	$7,0($18)

$L247:
$L507:
j	$L481
sw	$7,0($18)

$L430:
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC14)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L495:
lw	$4,20($18)
move	$5,$23
addiu	$2,$2,%lo(crc32)
jal	$2
move	$6,$20

j	$L209
sw	$2,20($18)

$L429:
lui	$2,%hi($LC13)
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC13)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L333:
addu	$3,$4,$5
sll	$2,$2,$3
addiu	$10,$2,-1
and	$3,$10,$21
srl	$3,$3,$5
addu	$3,$6,$3
sll	$3,$3,2
addu	$3,$8,$3
lbu	$7,1($3)
move	$9,$6
addu	$2,$5,$7
sltu	$2,$16,$2
lbu	$4,0($3)
beq	$2,$0,$L336
lhu	$6,2($3)

$L339:
beq	$22,$0,$L503
lw	$4,40($sp)

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addu	$21,$21,$2
and	$3,$21,$10
srl	$3,$3,$5
addu	$3,$3,$9
sll	$3,$3,2
addu	$3,$3,$8
lbu	$7,1($3)
addiu	$16,$16,8
addu	$2,$7,$5
sltu	$2,$16,$2
addiu	$23,$23,1
lbu	$4,0($3)
bne	$2,$0,$L339
lhu	$6,2($3)

$L336:
subu	$16,$16,$5
srl	$21,$21,$5
j	$L335
move	$5,$7

$L251:
lui	$2,%hi($LC7)
$L511:
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC7)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L270:
addu	$2,$2,$18
addiu	$8,$8,1
subu	$16,$16,$4
srl	$21,$21,$4
sh	$5,104($2)
j	$L482
sw	$8,96($18)

$L353:
lw	$3,44($18)
j	$L355
addu	$5,$2,$3

$L151:
j	$L481
sw	$7,0($18)

$L488:
lui	$2,%hi(inflate_fast)
sw	$5,12($17)
lw	$5,36($sp)
sw	$4,16($17)
sw	$22,4($17)
sw	$23,0($17)
sw	$21,48($18)
sw	$16,52($18)
addiu	$2,$2,%lo(inflate_fast)
jal	$2
move	$4,$17

lw	$6,12($17)
lw	$2,16($17)
lw	$23,0($17)
lw	$22,4($17)
lw	$21,48($18)
lw	$16,52($18)
lw	$7,0($18)
sw	$6,44($sp)
j	$L481
sw	$2,40($sp)

$L160:
lui	$2,%hi($LC0)
$L519:
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC0)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L308:
subu	$16,$16,$4
srl	$21,$21,$4
sw	$5,56($18)
$L317:
li	$7,23			# 0x17
j	$L481
sw	$7,0($18)

$L171:
andi	$2,$21,0xe000
beq	$2,$0,$L173
andi	$2,$21,0x200

lui	$2,%hi($LC3)
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC3)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L489:
addiu	$5,$4,2
sltu	$2,$16,$5
beq	$2,$0,$L274
nop

$L277:
beq	$22,$0,$L141
nop

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,$5
addu	$21,$21,$2
bne	$3,$0,$L277
addiu	$23,$23,1

$L274:
subu	$16,$16,$4
beq	$8,$0,$L292
srl	$21,$21,$4

sll	$2,$8,1
addu	$2,$2,$18
andi	$3,$21,0x3
lhu	$4,102($2)
addiu	$3,$3,3
srl	$21,$21,2
addiu	$16,$16,-2
$L281:
addu	$2,$3,$8
sltu	$2,$10,$2
bne	$2,$0,$L292
nop

beq	$3,$0,$L482
sll	$2,$8,1

addu	$2,$2,$18
andi	$5,$4,0xffff
addiu	$2,$2,104
move	$4,$0
$L295:
addiu	$4,$4,1
sh	$5,0($2)
addiu	$8,$8,1
bne	$3,$4,$L295
addiu	$2,$2,2

j	$L482
sw	$8,96($18)

$L237:
j	$L236
sw	$7,0($18)

$L238:
lui	$2,%hi(lenfix.1563)
addiu	$2,$2,%lo(lenfix.1563)
li	$3,9			# 0x9
sw	$2,68($18)
lui	$2,%hi(distfix.1564)
li	$7,18			# 0x12
sw	$3,76($18)
addiu	$2,$2,%lo(distfix.1564)
li	$3,5			# 0x5
sw	$2,72($18)
sw	$3,80($18)
j	$L236
sw	$7,0($18)

$L373:
andi	$3,$21,0xff00
andi	$4,$4,0xff00
sll	$3,$3,8
srl	$2,$21,24
sll	$5,$21,24
addu	$2,$2,$4
addu	$3,$3,$5
j	$L375
addu	$3,$2,$3

$L486:
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC5)
sw	$2,24($17)
j	$L236
sw	$7,0($18)

$L239:
li	$7,15			# 0xf
j	$L236
sw	$7,0($18)

$L173:
beq	$2,$0,$L531
li	$7,2			# 0x2

srl	$2,$21,8
sb	$2,25($sp)
sb	$21,24($sp)
lw	$4,20($18)
lui	$2,%hi(crc32)
addiu	$5,$sp,24
addiu	$2,$2,%lo(crc32)
jal	$2
li	$6,2			# 0x2

sw	$2,20($18)
li	$7,2			# 0x2
$L531:
move	$16,$0
move	$21,$0
j	$L180
sw	$7,0($18)

$L370:
move	$6,$5
subu	$5,$2,$5
lui	$2,%hi(adler32)
addiu	$2,$2,%lo(adler32)
jal	$2
lw	$4,20($18)

sw	$2,20($18)
j	$L368
sw	$2,48($17)

$L292:
lui	$2,%hi($LC9)
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC9)
sw	$2,24($17)
sw	$7,0($18)
$L280:
addiu	$4,$18,104
addiu	$3,$18,1320
sw	$4,48($sp)
li	$2,9			# 0x9
sw	$3,68($18)
sw	$2,76($18)
sw	$3,100($18)
addiu	$2,$18,76
addiu	$fp,$18,100
addiu	$20,$18,744
lw	$5,48($sp)
lui	$3,%hi(inflate_table)
li	$4,1			# 0x1
move	$7,$fp
sw	$2,16($sp)
addiu	$3,$3,%lo(inflate_table)
jal	$3
sw	$20,20($sp)

beq	$2,$0,$L496
move	$19,$2

lui	$2,%hi($LC10)
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC10)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L490:
addiu	$5,$4,3
sltu	$2,$16,$5
beq	$2,$0,$L532
srl	$2,$21,$4

$L290:
beq	$22,$0,$L141
nop

lbu	$2,0($23)
addiu	$22,$22,-1
sll	$2,$2,$16
addiu	$16,$16,8
sltu	$3,$16,$5
addu	$21,$21,$2
bne	$3,$0,$L290
addiu	$23,$23,1

srl	$2,$21,$4
$L532:
andi	$3,$2,0x7
subu	$4,$16,$4
addiu	$16,$4,-3
addiu	$3,$3,3
srl	$21,$2,3
j	$L281
move	$4,$0

$L428:
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC12)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L426:
lui	$2,%hi($LC11)
li	$7,27			# 0x1b
addiu	$2,$2,%lo($LC11)
sw	$7,0($18)
j	$L481
sw	$2,24($17)

$L165:
lui	$2,%hi(adler32)
move	$4,$0
move	$5,$0
addiu	$2,$2,%lo(adler32)
jal	$2
move	$6,$0

andi	$5,$21,0x200
li	$3,9			# 0x9
li	$4,11			# 0xb
movz	$3,$4,$5
move	$7,$3
move	$21,$0
move	$16,$0
sw	$2,20($18)
sw	$3,0($18)
j	$L481
sw	$2,48($17)

$L310:
subu	$16,$16,$4
srl	$21,$21,$4
j	$L318
sw	$5,56($18)

$L492:
lui	$16,%hi(crc32)
addiu	$16,$16,%lo(crc32)
move	$4,$0
move	$5,$0
jal	$16
move	$6,$0

sw	$2,20($18)
li	$3,-117
li	$2,31
sb	$2,24($sp)
sb	$3,25($sp)
lw	$4,20($18)
addiu	$5,$sp,24
jal	$16
li	$6,2			# 0x2

move	$21,$0
li	$7,1			# 0x1
move	$16,$0
sw	$7,0($18)
j	$L481
sw	$2,20($18)

$L424:
lw	$2,40($sp)
lw	$3,44($sp)
li	$19,2			# 0x2
sw	$22,4($17)
sw	$2,16($17)
sw	$3,12($17)
sw	$16,52($18)
sw	$23,0($17)
j	$L140
sw	$21,48($18)

$L264:
lw	$6,88($18)
lw	$11,92($18)
li	$7,17			# 0x11
move	$8,$0
sw	$7,0($18)
j	$L482
sw	$0,96($18)

.set	macro
.set	reorder
.end	inflate
.size	inflate, .-inflate
.section	.text.inflateSync,"ax",@progbits
.align	2
.align	5
.globl	inflateSync
.ent	inflateSync
inflateSync:
.frame	$sp,8,$31		# vars= 8, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-8
bne	$4,$0,$L588
move	$9,$4

li	$3,-2			# 0xfffffffffffffffe
$L540:
move	$2,$3
j	$31
addiu	$sp,$sp,8

$L588:
lw	$7,28($4)
beq	$7,$0,$L540
li	$3,-2			# 0xfffffffffffffffe

lw	$8,4($4)
bne	$8,$0,$L537
nop

lw	$2,52($7)
sltu	$2,$2,8
bne	$2,$0,$L540
li	$3,-5			# 0xfffffffffffffffb

$L537:
lw	$2,0($7)
li	$6,29			# 0x1d
beq	$2,$6,$L581
li	$3,-8			# 0xfffffffffffffff8

lw	$2,52($7)
lw	$5,48($7)
and	$3,$2,$3
andi	$2,$2,0x7
sll	$5,$5,$2
sltu	$4,$3,8
sw	$6,0($7)
sw	$5,48($7)
bne	$4,$0,$L543
sw	$3,52($7)

move	$6,$sp
$L545:
sb	$5,0($6)
lw	$2,52($7)
lw	$5,48($7)
addiu	$2,$2,-8
srl	$3,$5,8
sltu	$4,$2,8
move	$5,$3
sw	$3,48($7)
sw	$2,52($7)
beq	$4,$0,$L545
addiu	$6,$6,1

subu	$8,$6,$sp
sw	$0,96($7)
addiu	$12,$7,96
beq	$8,$0,$L547
move	$5,$0

move	$6,$0
j	$L550
li	$10,4			# 0x4

$L591:
move	$5,$0
$L556:
addiu	$6,$6,1
$L594:
beq	$6,$8,$L547
sltu	$2,$5,4

beq	$2,$0,$L547
nop

$L550:
addu	$2,$sp,$6
lbu	$4,0($2)
sltu	$3,$5,2
li	$2,255			# 0xff
movn	$2,$0,$3
beq	$2,$4,$L590
nop

bne	$4,$0,$L591
subu	$5,$10,$5

j	$L594
addiu	$6,$6,1

$L590:
j	$L556
addiu	$5,$5,1

$L543:
addiu	$12,$7,96
move	$5,$0
sw	$0,96($7)
$L547:
sw	$5,0($12)
lw	$8,4($9)
$L541:
lw	$10,0($9)
beq	$8,$0,$L560
lw	$5,96($7)

sltu	$2,$5,4
beq	$2,$0,$L560
li	$11,4			# 0x4

j	$L563
move	$6,$0

$L593:
move	$5,$0
$L569:
addiu	$6,$6,1
$L595:
beq	$8,$6,$L582
sltu	$2,$5,4

beq	$2,$0,$L582
nop

$L563:
addu	$2,$10,$6
lbu	$4,0($2)
sltu	$3,$5,2
li	$2,255			# 0xff
movn	$2,$0,$3
beq	$2,$4,$L592
nop

bne	$4,$0,$L593
subu	$5,$11,$5

j	$L595
addiu	$6,$6,1

$L592:
j	$L569
addiu	$5,$5,1

$L582:
move	$8,$6
$L574:
sw	$5,0($12)
lw	$2,4($9)
lw	$3,8($9)
subu	$2,$2,$6
sw	$2,4($9)
lw	$4,96($7)
addu	$2,$10,$8
addu	$5,$6,$3
sw	$2,0($9)
li	$2,4			# 0x4
beq	$4,$2,$L576
sw	$5,8($9)

j	$L540
li	$3,-3			# 0xfffffffffffffffd

$L576:
beq	$7,$0,$L578
lw	$4,20($9)

addiu	$2,$7,1320
sw	$0,24($7)
sw	$2,68($7)
sw	$0,20($9)
sw	$0,8($9)
sw	$0,24($9)
sw	$0,0($7)
sw	$0,4($7)
sw	$0,12($7)
sw	$0,32($7)
sw	$0,36($7)
sw	$0,48($7)
sw	$0,52($7)
sw	$2,100($7)
sw	$2,72($7)
$L578:
li	$2,11			# 0xb
move	$3,$0
sw	$4,20($9)
sw	$2,0($7)
j	$L540
sw	$5,8($9)

$L581:
j	$L541
addiu	$12,$7,96

$L560:
move	$6,$0
j	$L574
move	$8,$0

.set	macro
.set	reorder
.end	inflateSync
.size	inflateSync, .-inflateSync
.section	.text.inflateInit2_,"ax",@progbits
.align	2
.align	5
.globl	inflateInit2_
.ent	inflateInit2_
inflateInit2_:
.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
.mask	0x80030000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-32
sw	$17,20($sp)
sw	$16,16($sp)
sw	$31,24($sp)
move	$17,$4
bne	$6,$0,$L618
move	$16,$5

$L597:
li	$3,-6			# 0xfffffffffffffffa
$L609:
lw	$31,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$3
j	$31
addiu	$sp,$sp,32

$L618:
lb	$3,0($6)
li	$2,49			# 0x31
bne	$3,$2,$L597
li	$2,56			# 0x38

bne	$7,$2,$L609
li	$3,-6			# 0xfffffffffffffffa

bne	$4,$0,$L619
nop

$L601:
j	$L609
li	$3,-2			# 0xfffffffffffffffe

$L619:
lw	$3,32($4)
bne	$3,$0,$L603
sw	$0,24($4)

lui	$2,%hi(zcalloc)
addiu	$3,$2,%lo(zcalloc)
sw	$3,32($4)
sw	$0,40($4)
$L603:
lw	$2,36($17)
bne	$2,$0,$L605
lui	$2,%hi(zcfree)

addiu	$2,$2,%lo(zcfree)
sw	$2,36($17)
$L605:
lw	$4,40($17)
li	$5,1			# 0x1
jal	$3
li	$6,7080			# 0x1ba8

beq	$2,$0,$L620
move	$5,$2

bltz	$16,$L621
sw	$2,28($17)

sra	$2,$16,4
addiu	$2,$2,1
slt	$4,$16,48
andi	$3,$16,0xf
movn	$16,$3,$4
sw	$2,8($5)
$L612:
addiu	$2,$16,-8
sltu	$2,$2,8
beq	$2,$0,$L622
nop

lw	$4,28($17)
sw	$16,28($5)
beq	$4,$0,$L601
sw	$0,44($5)

addiu	$2,$4,1320
move	$3,$0
sw	$0,24($4)
sw	$2,68($4)
sw	$0,20($17)
sw	$0,8($17)
sw	$0,24($17)
sw	$0,0($4)
sw	$0,4($4)
sw	$0,12($4)
sw	$0,32($4)
sw	$0,36($4)
sw	$0,48($4)
sw	$0,52($4)
sw	$2,100($4)
j	$L609
sw	$2,72($4)

$L620:
j	$L609
li	$3,-4			# 0xfffffffffffffffc

$L622:
lw	$2,36($17)
jal	$2
lw	$4,40($17)

li	$3,-2			# 0xfffffffffffffffe
j	$L609
sw	$0,28($17)

$L621:
subu	$16,$0,$16
j	$L612
sw	$0,8($2)

.set	macro
.set	reorder
.end	inflateInit2_
.size	inflateInit2_, .-inflateInit2_
.section	.text.inflateInit_,"ax",@progbits
.align	2
.align	5
.globl	inflateInit_
.ent	inflateInit_
inflateInit_:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

lui	$25,%hi(inflateInit2_)
move	$7,$6
addiu	$25,$25,%lo(inflateInit2_)
move	$6,$5
jr	$25
li	$5,15			# 0xf

.set	macro
.set	reorder
.end	inflateInit_
.size	inflateInit_, .-inflateInit_
.rdata
.align	2
.type	order.1615, @object
.size	order.1615, 38
order.1615:
.half	16
.half	17
.half	18
.half	0
.half	8
.half	7
.half	9
.half	6
.half	10
.half	5
.half	11
.half	4
.half	12
.half	3
.half	13
.half	2
.half	14
.half	1
.half	15
.align	2
.type	distfix.1564, @object
.size	distfix.1564, 128
distfix.1564:
.byte	16
.byte	5
.half	1
.byte	23
.byte	5
.half	257
.byte	19
.byte	5
.half	17
.byte	27
.byte	5
.half	4097
.byte	17
.byte	5
.half	5
.byte	25
.byte	5
.half	1025
.byte	21
.byte	5
.half	65
.byte	29
.byte	5
.half	16385
.byte	16
.byte	5
.half	3
.byte	24
.byte	5
.half	513
.byte	20
.byte	5
.half	33
.byte	28
.byte	5
.half	8193
.byte	18
.byte	5
.half	9
.byte	26
.byte	5
.half	2049
.byte	22
.byte	5
.half	129
.byte	64
.byte	5
.half	0
.byte	16
.byte	5
.half	2
.byte	23
.byte	5
.half	385
.byte	19
.byte	5
.half	25
.byte	27
.byte	5
.half	6145
.byte	17
.byte	5
.half	7
.byte	25
.byte	5
.half	1537
.byte	21
.byte	5
.half	97
.byte	29
.byte	5
.half	24577
.byte	16
.byte	5
.half	4
.byte	24
.byte	5
.half	769
.byte	20
.byte	5
.half	49
.byte	28
.byte	5
.half	12289
.byte	18
.byte	5
.half	13
.byte	26
.byte	5
.half	3073
.byte	22
.byte	5
.half	193
.byte	64
.byte	5
.half	0
.align	2
.type	lenfix.1563, @object
.size	lenfix.1563, 2048
lenfix.1563:
.byte	96
.byte	7
.half	0
.byte	0
.byte	8
.half	80
.byte	0
.byte	8
.half	16
.byte	20
.byte	8
.half	115
.byte	18
.byte	7
.half	31
.byte	0
.byte	8
.half	112
.byte	0
.byte	8
.half	48
.byte	0
.byte	9
.half	192
.byte	16
.byte	7
.half	10
.byte	0
.byte	8
.half	96
.byte	0
.byte	8
.half	32
.byte	0
.byte	9
.half	160
.byte	0
.byte	8
.half	0
.byte	0
.byte	8
.half	128
.byte	0
.byte	8
.half	64
.byte	0
.byte	9
.half	224
.byte	16
.byte	7
.half	6
.byte	0
.byte	8
.half	88
.byte	0
.byte	8
.half	24
.byte	0
.byte	9
.half	144
.byte	19
.byte	7
.half	59
.byte	0
.byte	8
.half	120
.byte	0
.byte	8
.half	56
.byte	0
.byte	9
.half	208
.byte	17
.byte	7
.half	17
.byte	0
.byte	8
.half	104
.byte	0
.byte	8
.half	40
.byte	0
.byte	9
.half	176
.byte	0
.byte	8
.half	8
.byte	0
.byte	8
.half	136
.byte	0
.byte	8
.half	72
.byte	0
.byte	9
.half	240
.byte	16
.byte	7
.half	4
.byte	0
.byte	8
.half	84
.byte	0
.byte	8
.half	20
.byte	21
.byte	8
.half	227
.byte	19
.byte	7
.half	43
.byte	0
.byte	8
.half	116
.byte	0
.byte	8
.half	52
.byte	0
.byte	9
.half	200
.byte	17
.byte	7
.half	13
.byte	0
.byte	8
.half	100
.byte	0
.byte	8
.half	36
.byte	0
.byte	9
.half	168
.byte	0
.byte	8
.half	4
.byte	0
.byte	8
.half	132
.byte	0
.byte	8
.half	68
.byte	0
.byte	9
.half	232
.byte	16
.byte	7
.half	8
.byte	0
.byte	8
.half	92
.byte	0
.byte	8
.half	28
.byte	0
.byte	9
.half	152
.byte	20
.byte	7
.half	83
.byte	0
.byte	8
.half	124
.byte	0
.byte	8
.half	60
.byte	0
.byte	9
.half	216
.byte	18
.byte	7
.half	23
.byte	0
.byte	8
.half	108
.byte	0
.byte	8
.half	44
.byte	0
.byte	9
.half	184
.byte	0
.byte	8
.half	12
.byte	0
.byte	8
.half	140
.byte	0
.byte	8
.half	76
.byte	0
.byte	9
.half	248
.byte	16
.byte	7
.half	3
.byte	0
.byte	8
.half	82
.byte	0
.byte	8
.half	18
.byte	21
.byte	8
.half	163
.byte	19
.byte	7
.half	35
.byte	0
.byte	8
.half	114
.byte	0
.byte	8
.half	50
.byte	0
.byte	9
.half	196
.byte	17
.byte	7
.half	11
.byte	0
.byte	8
.half	98
.byte	0
.byte	8
.half	34
.byte	0
.byte	9
.half	164
.byte	0
.byte	8
.half	2
.byte	0
.byte	8
.half	130
.byte	0
.byte	8
.half	66
.byte	0
.byte	9
.half	228
.byte	16
.byte	7
.half	7
.byte	0
.byte	8
.half	90
.byte	0
.byte	8
.half	26
.byte	0
.byte	9
.half	148
.byte	20
.byte	7
.half	67
.byte	0
.byte	8
.half	122
.byte	0
.byte	8
.half	58
.byte	0
.byte	9
.half	212
.byte	18
.byte	7
.half	19
.byte	0
.byte	8
.half	106
.byte	0
.byte	8
.half	42
.byte	0
.byte	9
.half	180
.byte	0
.byte	8
.half	10
.byte	0
.byte	8
.half	138
.byte	0
.byte	8
.half	74
.byte	0
.byte	9
.half	244
.byte	16
.byte	7
.half	5
.byte	0
.byte	8
.half	86
.byte	0
.byte	8
.half	22
.byte	64
.byte	8
.half	0
.byte	19
.byte	7
.half	51
.byte	0
.byte	8
.half	118
.byte	0
.byte	8
.half	54
.byte	0
.byte	9
.half	204
.byte	17
.byte	7
.half	15
.byte	0
.byte	8
.half	102
.byte	0
.byte	8
.half	38
.byte	0
.byte	9
.half	172
.byte	0
.byte	8
.half	6
.byte	0
.byte	8
.half	134
.byte	0
.byte	8
.half	70
.byte	0
.byte	9
.half	236
.byte	16
.byte	7
.half	9
.byte	0
.byte	8
.half	94
.byte	0
.byte	8
.half	30
.byte	0
.byte	9
.half	156
.byte	20
.byte	7
.half	99
.byte	0
.byte	8
.half	126
.byte	0
.byte	8
.half	62
.byte	0
.byte	9
.half	220
.byte	18
.byte	7
.half	27
.byte	0
.byte	8
.half	110
.byte	0
.byte	8
.half	46
.byte	0
.byte	9
.half	188
.byte	0
.byte	8
.half	14
.byte	0
.byte	8
.half	142
.byte	0
.byte	8
.half	78
.byte	0
.byte	9
.half	252
.byte	96
.byte	7
.half	0
.byte	0
.byte	8
.half	81
.byte	0
.byte	8
.half	17
.byte	21
.byte	8
.half	131
.byte	18
.byte	7
.half	31
.byte	0
.byte	8
.half	113
.byte	0
.byte	8
.half	49
.byte	0
.byte	9
.half	194
.byte	16
.byte	7
.half	10
.byte	0
.byte	8
.half	97
.byte	0
.byte	8
.half	33
.byte	0
.byte	9
.half	162
.byte	0
.byte	8
.half	1
.byte	0
.byte	8
.half	129
.byte	0
.byte	8
.half	65
.byte	0
.byte	9
.half	226
.byte	16
.byte	7
.half	6
.byte	0
.byte	8
.half	89
.byte	0
.byte	8
.half	25
.byte	0
.byte	9
.half	146
.byte	19
.byte	7
.half	59
.byte	0
.byte	8
.half	121
.byte	0
.byte	8
.half	57
.byte	0
.byte	9
.half	210
.byte	17
.byte	7
.half	17
.byte	0
.byte	8
.half	105
.byte	0
.byte	8
.half	41
.byte	0
.byte	9
.half	178
.byte	0
.byte	8
.half	9
.byte	0
.byte	8
.half	137
.byte	0
.byte	8
.half	73
.byte	0
.byte	9
.half	242
.byte	16
.byte	7
.half	4
.byte	0
.byte	8
.half	85
.byte	0
.byte	8
.half	21
.byte	16
.byte	8
.half	258
.byte	19
.byte	7
.half	43
.byte	0
.byte	8
.half	117
.byte	0
.byte	8
.half	53
.byte	0
.byte	9
.half	202
.byte	17
.byte	7
.half	13
.byte	0
.byte	8
.half	101
.byte	0
.byte	8
.half	37
.byte	0
.byte	9
.half	170
.byte	0
.byte	8
.half	5
.byte	0
.byte	8
.half	133
.byte	0
.byte	8
.half	69
.byte	0
.byte	9
.half	234
.byte	16
.byte	7
.half	8
.byte	0
.byte	8
.half	93
.byte	0
.byte	8
.half	29
.byte	0
.byte	9
.half	154
.byte	20
.byte	7
.half	83
.byte	0
.byte	8
.half	125
.byte	0
.byte	8
.half	61
.byte	0
.byte	9
.half	218
.byte	18
.byte	7
.half	23
.byte	0
.byte	8
.half	109
.byte	0
.byte	8
.half	45
.byte	0
.byte	9
.half	186
.byte	0
.byte	8
.half	13
.byte	0
.byte	8
.half	141
.byte	0
.byte	8
.half	77
.byte	0
.byte	9
.half	250
.byte	16
.byte	7
.half	3
.byte	0
.byte	8
.half	83
.byte	0
.byte	8
.half	19
.byte	21
.byte	8
.half	195
.byte	19
.byte	7
.half	35
.byte	0
.byte	8
.half	115
.byte	0
.byte	8
.half	51
.byte	0
.byte	9
.half	198
.byte	17
.byte	7
.half	11
.byte	0
.byte	8
.half	99
.byte	0
.byte	8
.half	35
.byte	0
.byte	9
.half	166
.byte	0
.byte	8
.half	3
.byte	0
.byte	8
.half	131
.byte	0
.byte	8
.half	67
.byte	0
.byte	9
.half	230
.byte	16
.byte	7
.half	7
.byte	0
.byte	8
.half	91
.byte	0
.byte	8
.half	27
.byte	0
.byte	9
.half	150
.byte	20
.byte	7
.half	67
.byte	0
.byte	8
.half	123
.byte	0
.byte	8
.half	59
.byte	0
.byte	9
.half	214
.byte	18
.byte	7
.half	19
.byte	0
.byte	8
.half	107
.byte	0
.byte	8
.half	43
.byte	0
.byte	9
.half	182
.byte	0
.byte	8
.half	11
.byte	0
.byte	8
.half	139
.byte	0
.byte	8
.half	75
.byte	0
.byte	9
.half	246
.byte	16
.byte	7
.half	5
.byte	0
.byte	8
.half	87
.byte	0
.byte	8
.half	23
.byte	64
.byte	8
.half	0
.byte	19
.byte	7
.half	51
.byte	0
.byte	8
.half	119
.byte	0
.byte	8
.half	55
.byte	0
.byte	9
.half	206
.byte	17
.byte	7
.half	15
.byte	0
.byte	8
.half	103
.byte	0
.byte	8
.half	39
.byte	0
.byte	9
.half	174
.byte	0
.byte	8
.half	7
.byte	0
.byte	8
.half	135
.byte	0
.byte	8
.half	71
.byte	0
.byte	9
.half	238
.byte	16
.byte	7
.half	9
.byte	0
.byte	8
.half	95
.byte	0
.byte	8
.half	31
.byte	0
.byte	9
.half	158
.byte	20
.byte	7
.half	99
.byte	0
.byte	8
.half	127
.byte	0
.byte	8
.half	63
.byte	0
.byte	9
.half	222
.byte	18
.byte	7
.half	27
.byte	0
.byte	8
.half	111
.byte	0
.byte	8
.half	47
.byte	0
.byte	9
.half	190
.byte	0
.byte	8
.half	15
.byte	0
.byte	8
.half	143
.byte	0
.byte	8
.half	79
.byte	0
.byte	9
.half	254
.byte	96
.byte	7
.half	0
.byte	0
.byte	8
.half	80
.byte	0
.byte	8
.half	16
.byte	20
.byte	8
.half	115
.byte	18
.byte	7
.half	31
.byte	0
.byte	8
.half	112
.byte	0
.byte	8
.half	48
.byte	0
.byte	9
.half	193
.byte	16
.byte	7
.half	10
.byte	0
.byte	8
.half	96
.byte	0
.byte	8
.half	32
.byte	0
.byte	9
.half	161
.byte	0
.byte	8
.half	0
.byte	0
.byte	8
.half	128
.byte	0
.byte	8
.half	64
.byte	0
.byte	9
.half	225
.byte	16
.byte	7
.half	6
.byte	0
.byte	8
.half	88
.byte	0
.byte	8
.half	24
.byte	0
.byte	9
.half	145
.byte	19
.byte	7
.half	59
.byte	0
.byte	8
.half	120
.byte	0
.byte	8
.half	56
.byte	0
.byte	9
.half	209
.byte	17
.byte	7
.half	17
.byte	0
.byte	8
.half	104
.byte	0
.byte	8
.half	40
.byte	0
.byte	9
.half	177
.byte	0
.byte	8
.half	8
.byte	0
.byte	8
.half	136
.byte	0
.byte	8
.half	72
.byte	0
.byte	9
.half	241
.byte	16
.byte	7
.half	4
.byte	0
.byte	8
.half	84
.byte	0
.byte	8
.half	20
.byte	21
.byte	8
.half	227
.byte	19
.byte	7
.half	43
.byte	0
.byte	8
.half	116
.byte	0
.byte	8
.half	52
.byte	0
.byte	9
.half	201
.byte	17
.byte	7
.half	13
.byte	0
.byte	8
.half	100
.byte	0
.byte	8
.half	36
.byte	0
.byte	9
.half	169
.byte	0
.byte	8
.half	4
.byte	0
.byte	8
.half	132
.byte	0
.byte	8
.half	68
.byte	0
.byte	9
.half	233
.byte	16
.byte	7
.half	8
.byte	0
.byte	8
.half	92
.byte	0
.byte	8
.half	28
.byte	0
.byte	9
.half	153
.byte	20
.byte	7
.half	83
.byte	0
.byte	8
.half	124
.byte	0
.byte	8
.half	60
.byte	0
.byte	9
.half	217
.byte	18
.byte	7
.half	23
.byte	0
.byte	8
.half	108
.byte	0
.byte	8
.half	44
.byte	0
.byte	9
.half	185
.byte	0
.byte	8
.half	12
.byte	0
.byte	8
.half	140
.byte	0
.byte	8
.half	76
.byte	0
.byte	9
.half	249
.byte	16
.byte	7
.half	3
.byte	0
.byte	8
.half	82
.byte	0
.byte	8
.half	18
.byte	21
.byte	8
.half	163
.byte	19
.byte	7
.half	35
.byte	0
.byte	8
.half	114
.byte	0
.byte	8
.half	50
.byte	0
.byte	9
.half	197
.byte	17
.byte	7
.half	11
.byte	0
.byte	8
.half	98
.byte	0
.byte	8
.half	34
.byte	0
.byte	9
.half	165
.byte	0
.byte	8
.half	2
.byte	0
.byte	8
.half	130
.byte	0
.byte	8
.half	66
.byte	0
.byte	9
.half	229
.byte	16
.byte	7
.half	7
.byte	0
.byte	8
.half	90
.byte	0
.byte	8
.half	26
.byte	0
.byte	9
.half	149
.byte	20
.byte	7
.half	67
.byte	0
.byte	8
.half	122
.byte	0
.byte	8
.half	58
.byte	0
.byte	9
.half	213
.byte	18
.byte	7
.half	19
.byte	0
.byte	8
.half	106
.byte	0
.byte	8
.half	42
.byte	0
.byte	9
.half	181
.byte	0
.byte	8
.half	10
.byte	0
.byte	8
.half	138
.byte	0
.byte	8
.half	74
.byte	0
.byte	9
.half	245
.byte	16
.byte	7
.half	5
.byte	0
.byte	8
.half	86
.byte	0
.byte	8
.half	22
.byte	64
.byte	8
.half	0
.byte	19
.byte	7
.half	51
.byte	0
.byte	8
.half	118
.byte	0
.byte	8
.half	54
.byte	0
.byte	9
.half	205
.byte	17
.byte	7
.half	15
.byte	0
.byte	8
.half	102
.byte	0
.byte	8
.half	38
.byte	0
.byte	9
.half	173
.byte	0
.byte	8
.half	6
.byte	0
.byte	8
.half	134
.byte	0
.byte	8
.half	70
.byte	0
.byte	9
.half	237
.byte	16
.byte	7
.half	9
.byte	0
.byte	8
.half	94
.byte	0
.byte	8
.half	30
.byte	0
.byte	9
.half	157
.byte	20
.byte	7
.half	99
.byte	0
.byte	8
.half	126
.byte	0
.byte	8
.half	62
.byte	0
.byte	9
.half	221
.byte	18
.byte	7
.half	27
.byte	0
.byte	8
.half	110
.byte	0
.byte	8
.half	46
.byte	0
.byte	9
.half	189
.byte	0
.byte	8
.half	14
.byte	0
.byte	8
.half	142
.byte	0
.byte	8
.half	78
.byte	0
.byte	9
.half	253
.byte	96
.byte	7
.half	0
.byte	0
.byte	8
.half	81
.byte	0
.byte	8
.half	17
.byte	21
.byte	8
.half	131
.byte	18
.byte	7
.half	31
.byte	0
.byte	8
.half	113
.byte	0
.byte	8
.half	49
.byte	0
.byte	9
.half	195
.byte	16
.byte	7
.half	10
.byte	0
.byte	8
.half	97
.byte	0
.byte	8
.half	33
.byte	0
.byte	9
.half	163
.byte	0
.byte	8
.half	1
.byte	0
.byte	8
.half	129
.byte	0
.byte	8
.half	65
.byte	0
.byte	9
.half	227
.byte	16
.byte	7
.half	6
.byte	0
.byte	8
.half	89
.byte	0
.byte	8
.half	25
.byte	0
.byte	9
.half	147
.byte	19
.byte	7
.half	59
.byte	0
.byte	8
.half	121
.byte	0
.byte	8
.half	57
.byte	0
.byte	9
.half	211
.byte	17
.byte	7
.half	17
.byte	0
.byte	8
.half	105
.byte	0
.byte	8
.half	41
.byte	0
.byte	9
.half	179
.byte	0
.byte	8
.half	9
.byte	0
.byte	8
.half	137
.byte	0
.byte	8
.half	73
.byte	0
.byte	9
.half	243
.byte	16
.byte	7
.half	4
.byte	0
.byte	8
.half	85
.byte	0
.byte	8
.half	21
.byte	16
.byte	8
.half	258
.byte	19
.byte	7
.half	43
.byte	0
.byte	8
.half	117
.byte	0
.byte	8
.half	53
.byte	0
.byte	9
.half	203
.byte	17
.byte	7
.half	13
.byte	0
.byte	8
.half	101
.byte	0
.byte	8
.half	37
.byte	0
.byte	9
.half	171
.byte	0
.byte	8
.half	5
.byte	0
.byte	8
.half	133
.byte	0
.byte	8
.half	69
.byte	0
.byte	9
.half	235
.byte	16
.byte	7
.half	8
.byte	0
.byte	8
.half	93
.byte	0
.byte	8
.half	29
.byte	0
.byte	9
.half	155
.byte	20
.byte	7
.half	83
.byte	0
.byte	8
.half	125
.byte	0
.byte	8
.half	61
.byte	0
.byte	9
.half	219
.byte	18
.byte	7
.half	23
.byte	0
.byte	8
.half	109
.byte	0
.byte	8
.half	45
.byte	0
.byte	9
.half	187
.byte	0
.byte	8
.half	13
.byte	0
.byte	8
.half	141
.byte	0
.byte	8
.half	77
.byte	0
.byte	9
.half	251
.byte	16
.byte	7
.half	3
.byte	0
.byte	8
.half	83
.byte	0
.byte	8
.half	19
.byte	21
.byte	8
.half	195
.byte	19
.byte	7
.half	35
.byte	0
.byte	8
.half	115
.byte	0
.byte	8
.half	51
.byte	0
.byte	9
.half	199
.byte	17
.byte	7
.half	11
.byte	0
.byte	8
.half	99
.byte	0
.byte	8
.half	35
.byte	0
.byte	9
.half	167
.byte	0
.byte	8
.half	3
.byte	0
.byte	8
.half	131
.byte	0
.byte	8
.half	67
.byte	0
.byte	9
.half	231
.byte	16
.byte	7
.half	7
.byte	0
.byte	8
.half	91
.byte	0
.byte	8
.half	27
.byte	0
.byte	9
.half	151
.byte	20
.byte	7
.half	67
.byte	0
.byte	8
.half	123
.byte	0
.byte	8
.half	59
.byte	0
.byte	9
.half	215
.byte	18
.byte	7
.half	19
.byte	0
.byte	8
.half	107
.byte	0
.byte	8
.half	43
.byte	0
.byte	9
.half	183
.byte	0
.byte	8
.half	11
.byte	0
.byte	8
.half	139
.byte	0
.byte	8
.half	75
.byte	0
.byte	9
.half	247
.byte	16
.byte	7
.half	5
.byte	0
.byte	8
.half	87
.byte	0
.byte	8
.half	23
.byte	64
.byte	8
.half	0
.byte	19
.byte	7
.half	51
.byte	0
.byte	8
.half	119
.byte	0
.byte	8
.half	55
.byte	0
.byte	9
.half	207
.byte	17
.byte	7
.half	15
.byte	0
.byte	8
.half	103
.byte	0
.byte	8
.half	39
.byte	0
.byte	9
.half	175
.byte	0
.byte	8
.half	7
.byte	0
.byte	8
.half	135
.byte	0
.byte	8
.half	71
.byte	0
.byte	9
.half	239
.byte	16
.byte	7
.half	9
.byte	0
.byte	8
.half	95
.byte	0
.byte	8
.half	31
.byte	0
.byte	9
.half	159
.byte	20
.byte	7
.half	99
.byte	0
.byte	8
.half	127
.byte	0
.byte	8
.half	63
.byte	0
.byte	9
.half	223
.byte	18
.byte	7
.half	27
.byte	0
.byte	8
.half	111
.byte	0
.byte	8
.half	47
.byte	0
.byte	9
.half	191
.byte	0
.byte	8
.half	15
.byte	0
.byte	8
.half	143
.byte	0
.byte	8
.half	79
.byte	0
.byte	9
.half	255
.ident	"GCC: (GNU) 4.1.2"
