.file	1 "deflate.c"
.section .mdebug.abi32
.previous
.section	.text.deflatePrime,"ax",@progbits
.align	2
.align	5
.globl	deflatePrime
.ent	deflatePrime
deflatePrime:
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
li	$2,1			# 0x1
sll	$2,$2,$5
lw	$4,28($4)
addiu	$2,$2,-1
and	$6,$2,$6
beq	$4,$0,$L2
move	$2,$0

sh	$6,5808($4)
j	$31
sw	$5,5812($4)

.set	macro
.set	reorder
.end	deflatePrime
.size	deflatePrime, .-deflatePrime
.section	.text.deflateEnd,"ax",@progbits
.align	2
.align	5
.globl	deflateEnd
.ent	deflateEnd
deflateEnd:
.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
.mask	0x80030000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-32
sw	$16,16($sp)
sw	$31,24($sp)
sw	$17,20($sp)
bne	$4,$0,$L28
move	$16,$4

$L10:
li	$3,-2			# 0xfffffffffffffffe
lw	$31,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$3
j	$31
addiu	$sp,$sp,32

$L28:
lw	$5,28($4)
beq	$5,$0,$L10
li	$2,42			# 0x2a

lw	$17,4($5)
beq	$17,$2,$L13
li	$2,113			# 0x71

bne	$17,$2,$L29
li	$2,666			# 0x29a

$L13:
lw	$3,8($5)
$L30:
beq	$3,$0,$L16
nop

lw	$2,36($16)
lw	$4,40($16)
jal	$2
move	$5,$3

lw	$5,28($16)
$L16:
lw	$3,60($5)
beq	$3,$0,$L18
nop

lw	$2,36($16)
lw	$4,40($16)
jal	$2
move	$5,$3

lw	$5,28($16)
$L18:
lw	$3,56($5)
beq	$3,$0,$L20
nop

lw	$2,36($16)
lw	$4,40($16)
jal	$2
move	$5,$3

lw	$5,28($16)
$L20:
lw	$3,48($5)
beq	$3,$0,$L22
nop

lw	$2,36($16)
lw	$4,40($16)
jal	$2
move	$5,$3

lw	$5,28($16)
$L22:
lw	$3,36($16)
jal	$3
lw	$4,40($16)

xori	$2,$17,0x71
sw	$0,28($16)
li	$3,-3			# 0xfffffffffffffffd
movn	$3,$0,$2
lw	$31,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$3
j	$31
addiu	$sp,$sp,32

$L29:
bne	$17,$2,$L10
nop

j	$L30
lw	$3,8($5)

.set	macro
.set	reorder
.end	deflateEnd
.size	deflateEnd, .-deflateEnd
.section	.text.longest_match,"ax",@progbits
.align	2
.align	5
.ent	longest_match
longest_match:
.frame	$sp,16,$31		# vars= 0, regs= 4/0, args= 0, gp= 0
.mask	0x000f0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-16
sw	$19,12($sp)
sw	$18,8($sp)
sw	$17,4($sp)
sw	$16,0($sp)
move	$11,$4
lw	$4,36($4)
lw	$3,100($11)
lw	$25,48($11)
addiu	$2,$4,-262
sltu	$2,$2,$3
move	$6,$5
lw	$7,116($11)
addu	$8,$3,$25
lw	$9,112($11)
bne	$2,$0,$L32
lw	$16,136($11)

move	$17,$0
$L34:
lw	$13,108($11)
lw	$2,132($11)
addu	$3,$8,$9
sltu	$2,$9,$2
srl	$4,$7,2
sltu	$5,$13,$16
lbu	$14,0($3)
lw	$24,56($11)
lw	$15,44($11)
lbu	$18,-1($3)
movz	$7,$4,$2
movn	$16,$13,$5
j	$L37
addiu	$19,$8,258

$L39:
and	$2,$6,$15
$L64:
sll	$2,$2,1
addu	$2,$2,$24
lhu	$6,0($2)
sltu	$3,$17,$6
beq	$3,$0,$L58
addiu	$7,$7,-1

beq	$7,$0,$L63
sltu	$2,$5,$13

$L37:
addu	$4,$6,$25
addu	$3,$4,$9
lbu	$2,0($3)
bne	$2,$14,$L39
move	$5,$9

lbu	$2,-1($3)
bne	$2,$18,$L64
and	$2,$6,$15

lbu	$3,0($4)
lbu	$2,0($8)
bne	$3,$2,$L64
and	$2,$6,$15

lbu	$3,1($4)
lbu	$2,1($8)
bne	$3,$2,$L64
and	$2,$6,$15

addiu	$8,$8,2
addiu	$10,$4,2
$L44:
lbu	$3,1($8)
lbu	$2,1($10)
bne	$3,$2,$L45
addiu	$4,$8,1

lbu	$3,2($8)
lbu	$2,2($10)
bne	$3,$2,$L45
addiu	$4,$8,2

lbu	$3,3($8)
lbu	$2,3($10)
bne	$3,$2,$L45
addiu	$4,$8,3

lbu	$3,4($8)
lbu	$2,4($10)
bne	$3,$2,$L45
addiu	$4,$8,4

lbu	$3,5($8)
lbu	$2,5($10)
bne	$3,$2,$L45
addiu	$4,$8,5

lbu	$3,6($8)
lbu	$2,6($10)
bne	$3,$2,$L45
addiu	$4,$8,6

lbu	$2,7($10)
lbu	$3,7($8)
addiu	$4,$8,7
bne	$3,$2,$L45
addiu	$10,$10,8

lbu	$3,8($8)
lbu	$2,0($10)
addiu	$4,$8,8
sltu	$12,$4,$19
bne	$3,$2,$L45
move	$8,$4

bne	$12,$0,$L44
nop

$L45:
subu	$2,$19,$4
li	$3,258			# 0x102
subu	$3,$3,$2
slt	$4,$9,$3
beq	$4,$0,$L39
addiu	$8,$19,-258

slt	$2,$3,$16
beq	$2,$0,$L61
sw	$6,104($11)

addu	$2,$8,$3
lbu	$14,0($2)
lbu	$18,-1($2)
move	$5,$3
j	$L39
move	$9,$3

$L61:
move	$5,$3
$L58:
sltu	$2,$5,$13
$L63:
movz	$5,$13,$2
lw	$19,12($sp)
lw	$18,8($sp)
lw	$17,4($sp)
lw	$16,0($sp)
move	$2,$5
j	$31
addiu	$sp,$sp,16

$L32:
subu	$2,$3,$4
j	$L34
addiu	$17,$2,262

.set	macro
.set	reorder
.end	longest_match
.size	longest_match, .-longest_match
.section	.text.deflateCopy,"ax",@progbits
.align	2
.align	5
.globl	deflateCopy
.ent	deflateCopy
deflateCopy:
.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
.mask	0x800f0000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
sw	$16,16($sp)
sw	$31,32($sp)
sw	$19,28($sp)
sw	$18,24($sp)
sw	$17,20($sp)
bne	$5,$0,$L86
move	$16,$4

li	$8,-2			# 0xfffffffffffffffe
$L73:
lw	$31,32($sp)
$L87:
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$8
j	$31
addiu	$sp,$sp,40

$L86:
beq	$4,$0,$L73
li	$8,-2			# 0xfffffffffffffffe

lw	$18,28($5)
beq	$18,$0,$L87
lw	$31,32($sp)

move	$6,$5
move	$7,$4
addiu	$8,$5,48
$L70:
lw	$2,0($6)
lw	$3,4($6)
lw	$4,8($6)
lw	$5,12($6)
addiu	$6,$6,16
sw	$2,0($7)
sw	$3,4($7)
sw	$4,8($7)
sw	$5,12($7)
bne	$6,$8,$L70
addiu	$7,$7,16

lw	$2,4($6)
lw	$3,0($6)
sw	$2,4($7)
sw	$3,0($7)
lw	$2,32($16)
lw	$4,40($16)
li	$5,1			# 0x1
jal	$2
li	$6,5816			# 0x16b8

beq	$2,$0,$L85
move	$17,$2

sw	$2,28($16)
move	$6,$18
move	$7,$2
addiu	$8,$18,5808
$L74:
lw	$2,0($6)
lw	$3,4($6)
lw	$4,8($6)
lw	$5,12($6)
addiu	$6,$6,16
sw	$2,0($7)
sw	$3,4($7)
sw	$4,8($7)
sw	$5,12($7)
bne	$6,$8,$L74
addiu	$7,$7,16

lw	$2,4($6)
lw	$3,0($6)
sw	$2,4($7)
sw	$3,0($7)
lw	$4,40($16)
lw	$5,36($17)
lw	$2,32($16)
li	$6,2			# 0x2
jal	$2
sw	$16,0($17)

lw	$4,40($16)
lw	$5,36($17)
lw	$3,32($16)
li	$6,2			# 0x2
jal	$3
sw	$2,48($17)

lw	$4,40($16)
lw	$5,68($17)
lw	$3,32($16)
li	$6,2			# 0x2
jal	$3
sw	$2,56($17)

lw	$4,40($16)
lw	$7,32($16)
lw	$5,5780($17)
sw	$2,60($17)
jal	$7
li	$6,4			# 0x4

lw	$4,48($17)
move	$19,$2
beq	$4,$0,$L75
sw	$2,8($17)

lw	$2,56($17)
beq	$2,$0,$L88
lui	$2,%hi(deflateEnd)

lw	$2,60($17)
beq	$2,$0,$L88
lui	$2,%hi(deflateEnd)

beq	$19,$0,$L89
addiu	$2,$2,%lo(deflateEnd)

lw	$6,36($17)
lw	$5,48($18)
lui	$16,%hi(memcpy)
addiu	$16,$16,%lo(memcpy)
jal	$16
sll	$6,$6,1

lw	$6,36($17)
lw	$4,56($17)
lw	$5,56($18)
jal	$16
sll	$6,$6,1

lw	$6,68($17)
lw	$4,60($17)
lw	$5,60($18)
jal	$16
sll	$6,$6,1

lw	$4,8($17)
lw	$5,8($18)
jal	$16
lw	$6,12($17)

lw	$3,5780($17)
lw	$7,8($18)
lw	$2,16($18)
lw	$6,8($17)
srl	$4,$3,1
sll	$5,$3,1
subu	$2,$2,$7
addu	$5,$5,$3
sll	$4,$4,1
addu	$5,$6,$5
addu	$4,$19,$4
addu	$6,$6,$2
addiu	$3,$17,140
addiu	$7,$17,2432
addiu	$2,$17,2676
move	$8,$0
sw	$2,2856($17)
sw	$6,16($17)
sw	$4,5788($17)
sw	$5,5776($17)
sw	$3,2832($17)
j	$L73
sw	$7,2844($17)

$L75:
lui	$2,%hi(deflateEnd)
$L88:
addiu	$2,$2,%lo(deflateEnd)
$L89:
jal	$2
move	$4,$16

$L85:
j	$L73
li	$8,-4			# 0xfffffffffffffffc

.set	macro
.set	reorder
.end	deflateCopy
.size	deflateCopy, .-deflateCopy
.section	.text.fill_window,"ax",@progbits
.align	2
.align	5
.ent	fill_window
fill_window:
.frame	$sp,56,$31		# vars= 0, regs= 10/0, args= 16, gp= 0
.mask	0xc0ff0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-56
sw	$fp,48($sp)
sw	$19,28($sp)
sw	$31,52($sp)
sw	$23,44($sp)
sw	$22,40($sp)
sw	$21,36($sp)
sw	$20,32($sp)
sw	$18,24($sp)
sw	$17,20($sp)
sw	$16,16($sp)
lw	$18,36($4)
lw	$21,108($4)
move	$19,$4
move	$2,$18
j	$L91
sll	$fp,$18,1

$L128:
move	$2,$0
lw	$3,28($16)
sw	$2,4($16)
lw	$3,24($3)
li	$2,1			# 0x1
beq	$3,$2,$L125
nop

$L112:
li	$2,2			# 0x2
beq	$3,$2,$L126
lui	$2,%hi(crc32)

$L114:
addu	$4,$23,$20
lw	$5,0($16)
lui	$2,%hi(memcpy)
addu	$4,$4,$21
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$17

lw	$3,0($16)
lw	$2,8($16)
addu	$3,$3,$17
addu	$2,$2,$17
lw	$21,108($19)
sw	$2,8($16)
sw	$3,0($16)
move	$6,$17
$L111:
addu	$7,$21,$6
sltu	$2,$7,3
sw	$7,108($19)
bne	$2,$0,$L116
move	$21,$7

lw	$4,100($19)
lw	$3,48($19)
lw	$6,80($19)
addu	$3,$3,$4
lbu	$2,0($3)
lw	$5,76($19)
sw	$2,64($19)
lbu	$4,1($3)
sll	$2,$2,$6
xor	$2,$2,$4
and	$2,$2,$5
sw	$2,64($19)
$L116:
sltu	$2,$7,262
beq	$2,$0,$L130
lw	$31,52($sp)

lw	$2,0($19)
lw	$3,4($2)
beq	$3,$0,$L130
nop

lw	$2,36($19)
$L91:
addu	$3,$18,$2
lw	$20,100($19)
lw	$2,52($19)
addiu	$3,$3,-262
subu	$2,$2,$21
sltu	$3,$20,$3
beq	$3,$0,$L127
subu	$22,$2,$20

lw	$16,0($19)
lw	$17,4($16)
beq	$17,$0,$L119
nop

$L129:
sltu	$2,$22,$17
lw	$23,48($19)
beq	$2,$0,$L128
lw	$21,108($19)

bne	$22,$0,$L109
subu	$2,$17,$22

j	$L111
move	$6,$0

$L127:
lw	$4,48($19)
lui	$2,%hi(memcpy)
addu	$5,$4,$18
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$18

lw	$3,104($19)
lw	$20,100($19)
lw	$2,84($19)
lw	$6,68($19)
lw	$5,60($19)
subu	$3,$3,$18
subu	$20,$20,$18
subu	$2,$2,$18
sll	$4,$6,1
sw	$3,104($19)
sw	$2,84($19)
addu	$4,$4,$5
sw	$20,100($19)
$L94:
addiu	$4,$4,-2
lhu	$2,0($4)
subu	$3,$2,$18
sltu	$2,$2,$18
bne	$2,$0,$L97
move	$5,$0

andi	$5,$3,0xffff
$L97:
addiu	$6,$6,-1
bne	$6,$0,$L94
sh	$5,0($4)

lw	$2,56($19)
move	$4,$18
addu	$3,$fp,$2
$L99:
addiu	$3,$3,-2
lhu	$2,0($3)
subu	$5,$2,$18
sltu	$2,$2,$18
bne	$2,$0,$L102
move	$6,$0

andi	$6,$5,0xffff
$L102:
addiu	$4,$4,-1
bne	$4,$0,$L99
sh	$6,0($3)

lw	$16,0($19)
lw	$17,4($16)
bne	$17,$0,$L129
addu	$22,$22,$18

$L119:
lw	$31,52($sp)
$L130:
lw	$fp,48($sp)
lw	$23,44($sp)
lw	$22,40($sp)
lw	$21,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
j	$31
addiu	$sp,$sp,56

$L109:
lw	$3,28($16)
sw	$2,4($16)
lw	$3,24($3)
li	$2,1			# 0x1
bne	$3,$2,$L112
move	$17,$22

$L125:
lw	$4,48($16)
lw	$5,0($16)
lui	$2,%hi(adler32)
addiu	$2,$2,%lo(adler32)
jal	$2
move	$6,$17

j	$L114
sw	$2,48($16)

$L126:
lw	$4,48($16)
lw	$5,0($16)
addiu	$2,$2,%lo(crc32)
jal	$2
move	$6,$17

j	$L114
sw	$2,48($16)

.set	macro
.set	reorder
.end	fill_window
.size	fill_window, .-fill_window
.section	.text.deflateSetDictionary,"ax",@progbits
.align	2
.align	5
.globl	deflateSetDictionary
.ent	deflateSetDictionary
deflateSetDictionary:
.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
.mask	0x800f0000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
sw	$18,24($sp)
sw	$17,20($sp)
sw	$16,16($sp)
sw	$31,32($sp)
sw	$19,28($sp)
move	$16,$4
move	$17,$5
bne	$4,$0,$L150
move	$18,$6

$L132:
li	$2,-2			# 0xfffffffffffffffe
$L147:
lw	$31,32($sp)
$L153:
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
j	$31
addiu	$sp,$sp,40

$L150:
lw	$19,28($4)
beq	$19,$0,$L147
li	$2,-2			# 0xfffffffffffffffe

beq	$5,$0,$L153
lw	$31,32($sp)

lw	$3,24($19)
li	$2,2			# 0x2
beq	$3,$2,$L132
li	$2,1			# 0x1

beq	$3,$2,$L151
li	$2,42			# 0x2a

beq	$3,$0,$L154
sltu	$2,$18,3

lw	$4,48($16)
$L152:
lui	$2,%hi(adler32)
move	$5,$17
addiu	$2,$2,%lo(adler32)
jal	$2
move	$6,$18

sw	$2,48($16)
sltu	$2,$18,3
$L154:
bne	$2,$0,$L147
move	$2,$0

lw	$2,36($19)
addiu	$16,$2,-262
sltu	$3,$16,$18
bne	$3,$0,$L143
subu	$2,$18,$16

move	$16,$18
$L145:
lw	$4,48($19)
lui	$2,%hi(memcpy)
move	$5,$17
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$16

lw	$8,48($19)
sw	$16,100($19)
sw	$16,84($19)
lbu	$5,0($8)
lw	$13,80($19)
sw	$5,64($19)
lbu	$3,1($8)
sll	$2,$5,$13
lw	$12,76($19)
xor	$2,$2,$3
and	$5,$2,$12
lw	$9,44($19)
lw	$10,56($19)
lw	$11,60($19)
sw	$5,64($19)
addiu	$6,$16,-3
move	$7,$0
$L146:
addu	$3,$7,$8
lbu	$4,2($3)
sll	$2,$5,$13
xor	$2,$2,$4
and	$2,$2,$12
sll	$3,$2,1
move	$5,$2
addu	$3,$3,$11
and	$2,$7,$9
lhu	$4,0($3)
sll	$2,$2,1
addu	$2,$2,$10
sh	$4,0($2)
sh	$7,0($3)
addiu	$7,$7,1
sltu	$2,$6,$7
beq	$2,$0,$L146
sw	$5,64($19)

j	$L147
move	$2,$0

$L143:
j	$L145
addu	$17,$17,$2

$L151:
lw	$3,4($19)
bne	$3,$2,$L147
li	$2,-2			# 0xfffffffffffffffe

j	$L152
lw	$4,48($16)

.set	macro
.set	reorder
.end	deflateSetDictionary
.size	deflateSetDictionary, .-deflateSetDictionary
.section	.text.deflateBound,"ax",@progbits
.align	2
.align	5
.globl	deflateBound
.ent	deflateBound
deflateBound:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$2,$5,7
srl	$2,$2,3
addu	$2,$5,$2
addiu	$3,$5,63
srl	$3,$3,6
addiu	$2,$2,11
beq	$4,$0,$L156
addu	$3,$3,$2

lw	$4,28($4)
beq	$4,$0,$L156
nop

lw	$6,40($4)
li	$2,15			# 0xf
beq	$6,$2,$L162
nop

$L156:
j	$31
move	$2,$3

$L162:
lw	$2,72($4)
bne	$2,$6,$L156
move	$4,$5

lui	$25,%hi(compressBound)
addiu	$25,$25,%lo(compressBound)
jr	$25
nop

.set	macro
.set	reorder
.end	deflateBound
.size	deflateBound, .-deflateBound
.section	.text.deflateReset,"ax",@progbits
.align	2
.align	5
.globl	deflateReset
.ent	deflateReset
deflateReset:
.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
.mask	0x80030000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-32
sw	$16,16($sp)
sw	$31,24($sp)
sw	$17,20($sp)
bne	$4,$0,$L179
move	$16,$4

$L164:
li	$4,-2			# 0xfffffffffffffffe
$L177:
lw	$31,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,32

$L179:
lw	$17,28($4)
beq	$17,$0,$L164
nop

lw	$2,32($4)
beq	$2,$0,$L164
nop

lw	$2,36($4)
beq	$2,$0,$L164
li	$5,2			# 0x2

sw	$5,44($4)
lw	$2,8($17)
lw	$4,24($17)
sw	$2,16($17)
sw	$0,20($16)
sw	$0,8($16)
sw	$0,24($16)
bltz	$4,$L180
sw	$0,20($17)

$L169:
li	$2,42			# 0x2a
li	$3,113			# 0x71
movz	$2,$3,$4
beq	$4,$5,$L181
sw	$2,4($17)

lui	$2,%hi(adler32)
move	$4,$0
move	$5,$0
addiu	$2,$2,%lo(adler32)
jal	$2
move	$6,$0

sw	$2,48($16)
$L182:
lui	$2,%hi(_tr_init)
move	$4,$17
addiu	$2,$2,%lo(_tr_init)
jal	$2
sw	$0,32($17)

lw	$6,68($17)
lw	$2,36($17)
lw	$4,60($17)
sll	$6,$6,1
sll	$2,$2,1
addu	$3,$6,$4
sw	$2,52($17)
lui	$2,%hi(memset)
sh	$0,-2($3)
addiu	$6,$6,-2
addiu	$2,$2,%lo(memset)
jal	$2
move	$5,$0

lw	$3,124($17)
lui	$2,%hi(configuration_table)
sll	$4,$3,4
sll	$3,$3,2
subu	$4,$4,$3
addiu	$2,$2,%lo(configuration_table)
addu	$4,$4,$2
lhu	$7,6($4)
lhu	$2,2($4)
lhu	$3,0($4)
lhu	$5,4($4)
li	$6,2			# 0x2
move	$4,$0
sw	$2,120($17)
sw	$3,132($17)
sw	$5,136($17)
sw	$7,116($17)
sw	$6,88($17)
sw	$0,64($17)
sw	$0,100($17)
sw	$0,84($17)
sw	$0,108($17)
sw	$6,112($17)
j	$L177
sw	$0,96($17)

$L181:
lui	$2,%hi(crc32)
move	$4,$0
move	$5,$0
addiu	$2,$2,%lo(crc32)
jal	$2
move	$6,$0

j	$L182
sw	$2,48($16)

$L180:
subu	$4,$0,$4
j	$L169
sw	$4,24($17)

.set	macro
.set	reorder
.end	deflateReset
.size	deflateReset, .-deflateReset
.section	.text.deflateInit2_,"ax",@progbits
.align	2
.align	5
.globl	deflateInit2_
.ent	deflateInit2_
deflateInit2_:
.frame	$sp,56,$31		# vars= 0, regs= 10/0, args= 16, gp= 0
.mask	0xc0ff0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-56
lw	$2,80($sp)
sw	$22,40($sp)
sw	$21,36($sp)
sw	$20,32($sp)
sw	$19,28($sp)
sw	$18,24($sp)
sw	$17,20($sp)
move	$18,$4
sw	$31,52($sp)
sw	$fp,48($sp)
sw	$23,44($sp)
sw	$16,16($sp)
move	$21,$5
move	$22,$6
move	$17,$7
lw	$19,72($sp)
lw	$20,76($sp)
bne	$2,$0,$L221
lw	$4,84($sp)

li	$2,-6			# 0xfffffffffffffffa
$L213:
lw	$31,52($sp)
$L226:
$L227:
lw	$fp,48($sp)
lw	$23,44($sp)
lw	$22,40($sp)
lw	$21,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
j	$31
addiu	$sp,$sp,56

$L221:
lb	$3,0($2)
li	$2,49			# 0x31
bne	$3,$2,$L213
li	$2,-6			# 0xfffffffffffffffa

li	$2,56			# 0x38
bne	$4,$2,$L213
li	$2,-6			# 0xfffffffffffffffa

beq	$18,$0,$L213
li	$2,-2			# 0xfffffffffffffffe

lw	$2,32($18)
bne	$2,$0,$L190
sw	$0,24($18)

lui	$2,%hi(zcalloc)
addiu	$2,$2,%lo(zcalloc)
sw	$2,32($18)
sw	$0,40($18)
$L190:
lw	$2,36($18)
bne	$2,$0,$L224
nor	$3,$0,$21

lui	$2,%hi(zcfree)
addiu	$2,$2,%lo(zcfree)
sw	$2,36($18)
$L224:
li	$2,6			# 0x6
bltz	$17,$L223
movz	$21,$2,$3

slt	$2,$17,16
beq	$2,$0,$L199
nop

li	$fp,1			# 0x1
$L198:
addiu	$2,$19,-1
sltu	$2,$2,9
beq	$2,$0,$L213
li	$2,-2			# 0xfffffffffffffffe

li	$2,8			# 0x8
bne	$22,$2,$L213
li	$2,-2			# 0xfffffffffffffffe

slt	$2,$17,8
bne	$2,$0,$L213
li	$2,-2			# 0xfffffffffffffffe

slt	$2,$17,16
beq	$2,$0,$L213
li	$2,-2			# 0xfffffffffffffffe

bltz	$21,$L226
lw	$31,52($sp)

slt	$2,$21,10
beq	$2,$0,$L226
li	$2,-2			# 0xfffffffffffffffe

bltz	$20,$L226
nop

slt	$2,$20,4
beq	$2,$0,$L227
li	$2,-2			# 0xfffffffffffffffe

lw	$2,32($18)
lw	$4,40($18)
xori	$7,$17,0x8
li	$3,9			# 0x9
li	$5,1			# 0x1
li	$6,5816			# 0x16b8
jal	$2
movz	$17,$3,$7

beq	$2,$0,$L220
move	$23,$2

li	$2,-1431699456			# 0xffffffffaaaa0000
addiu	$3,$19,9
ori	$2,$2,0xaaab
multu	$3,$2
addiu	$8,$19,7
mfhi	$3
li	$16,1			# 0x1
sll	$9,$16,$8
sll	$6,$16,$17
addiu	$7,$9,-1
srl	$3,$3,1
addiu	$5,$6,-1
lw	$4,40($18)
lw	$2,32($18)
sw	$8,72($23)
sw	$9,68($23)
sw	$7,76($23)
sw	$5,44($23)
sw	$3,80($23)
move	$5,$6
sw	$6,36($23)
sw	$fp,24($23)
li	$6,2			# 0x2
sw	$23,28($18)
sw	$18,0($23)
jal	$2
sw	$17,40($23)

lw	$3,32($18)
lw	$4,40($18)
lw	$5,36($23)
li	$6,2			# 0x2
jal	$3
sw	$2,48($23)

lw	$4,40($18)
lw	$5,68($23)
lw	$7,32($18)
li	$6,2			# 0x2
jal	$7
sw	$2,56($23)

addiu	$3,$19,6
sll	$16,$16,$3
lw	$4,40($18)
lw	$7,32($18)
move	$5,$16
sw	$2,60($23)
sw	$16,5780($23)
jal	$7
li	$6,4			# 0x4

lw	$4,5780($23)
lw	$3,48($23)
move	$5,$2
sll	$2,$4,2
sw	$2,12($23)
beq	$3,$0,$L214
sw	$5,8($23)

lw	$2,56($23)
beq	$2,$0,$L225
lui	$2,%hi(z_errmsg+24)

lw	$2,60($23)
beq	$2,$0,$L214
nop

beq	$5,$0,$L214
sll	$3,$4,1

srl	$2,$4,1
addu	$3,$3,$4
sll	$2,$2,1
addu	$3,$5,$3
addu	$2,$5,$2
sb	$22,29($23)
sw	$2,5788($23)
sw	$3,5776($23)
sw	$21,124($23)
sw	$20,128($23)
lui	$25,%hi(deflateReset)
move	$4,$18
lw	$31,52($sp)
lw	$fp,48($sp)
lw	$23,44($sp)
lw	$22,40($sp)
lw	$21,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
addiu	$25,$25,%lo(deflateReset)
jr	$25
addiu	$sp,$sp,56

$L223:
subu	$17,$0,$17
j	$L198
move	$fp,$0

$L214:
lui	$2,%hi(z_errmsg+24)
$L225:
lw	$4,%lo(z_errmsg+24)($2)
li	$3,666			# 0x29a
lui	$2,%hi(deflateEnd)
sw	$4,24($18)
sw	$3,4($23)
addiu	$2,$2,%lo(deflateEnd)
jal	$2
move	$4,$18

$L220:
j	$L213
li	$2,-4			# 0xfffffffffffffffc

$L199:
addiu	$17,$17,-16
j	$L198
li	$fp,2			# 0x2

.set	macro
.set	reorder
.end	deflateInit2_
.size	deflateInit2_, .-deflateInit2_
.section	.text.deflateInit_,"ax",@progbits
.align	2
.align	5
.globl	deflateInit_
.ent	deflateInit_
deflateInit_:
.frame	$sp,40,$31		# vars= 0, regs= 1/0, args= 32, gp= 0
.mask	0x80000000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
li	$2,8			# 0x8
sw	$2,16($sp)
lui	$2,%hi(deflateInit2_)
sw	$31,32($sp)
sw	$6,24($sp)
sw	$7,28($sp)
sw	$0,20($sp)
li	$6,8			# 0x8
addiu	$2,$2,%lo(deflateInit2_)
jal	$2
li	$7,15			# 0xf

lw	$31,32($sp)
j	$31
addiu	$sp,$sp,40

.set	macro
.set	reorder
.end	deflateInit_
.size	deflateInit_, .-deflateInit_
.section	.text.deflate_slow,"ax",@progbits
.align	2
.align	5
.ent	deflate_slow
deflate_slow:
.frame	$sp,40,$31		# vars= 0, regs= 6/0, args= 16, gp= 0
.mask	0x801f0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
sw	$20,32($sp)
sw	$19,28($sp)
sw	$16,16($sp)
sw	$31,36($sp)
sw	$18,24($sp)
sw	$17,20($sp)
lw	$13,108($4)
move	$16,$4
move	$20,$5
move	$19,$0
$L336:
sltu	$2,$13,262
$L353:
bne	$2,$0,$L350
lui	$2,%hi(fill_window)

sltu	$2,$13,3
$L352:
bne	$2,$0,$L317
nop

lw	$12,100($16)
lw	$2,48($16)
lw	$3,64($16)
addu	$2,$2,$12
lw	$4,80($16)
lbu	$5,2($2)
sll	$3,$3,$4
lw	$2,76($16)
xor	$3,$3,$5
and	$3,$3,$2
lw	$5,60($16)
lw	$2,44($16)
sll	$4,$3,1
addu	$4,$4,$5
and	$2,$12,$2
lw	$5,56($16)
lhu	$6,0($4)
sll	$2,$2,1
addu	$2,$2,$5
sh	$6,0($2)
move	$19,$6
sh	$12,0($4)
sw	$3,64($16)
$L238:
lw	$11,88($16)
lw	$2,104($16)
li	$3,2			# 0x2
sw	$2,92($16)
sw	$3,88($16)
beq	$19,$0,$L240
sw	$11,112($16)

lw	$2,120($16)
sltu	$2,$11,$2
beq	$2,$0,$L240
nop

lw	$2,36($16)
subu	$4,$12,$19
addiu	$2,$2,-262
sltu	$2,$2,$4
bne	$2,$0,$L240
nop

lw	$3,128($16)
slt	$2,$3,2
bne	$2,$0,$L338
li	$2,3			# 0x3

beq	$3,$2,$L339
li	$2,1			# 0x1

$L240:
lw	$8,112($16)
sltu	$2,$8,3
bne	$2,$0,$L275
nop

lw	$2,88($16)
sltu	$2,$8,$2
bne	$2,$0,$L275
addiu	$8,$8,-3

lui	$2,%hi(_length_code)
andi	$8,$8,0x00ff
addiu	$2,$2,%lo(_length_code)
lw	$3,92($16)
addu	$2,$8,$2
lw	$6,5784($16)
lbu	$4,0($2)
lw	$5,5788($16)
lw	$7,5776($16)
subu	$3,$12,$3
sll	$2,$6,1
addiu	$3,$3,-1
addiu	$4,$4,257
addu	$2,$2,$5
addu	$7,$7,$6
andi	$3,$3,0xffff
addiu	$6,$6,1
sll	$4,$4,2
sh	$3,0($2)
lw	$5,108($16)
addu	$4,$4,$16
sb	$8,0($7)
sw	$6,5784($16)
addiu	$3,$3,-1
lhu	$2,140($4)
andi	$7,$3,0xffff
addu	$5,$12,$5
addiu	$2,$2,1
sltu	$3,$7,256
sh	$2,140($4)
beq	$3,$0,$L278
addiu	$15,$5,-3

lui	$2,%hi(_dist_code)
addiu	$2,$2,%lo(_dist_code)
addu	$2,$7,$2
lbu	$2,0($2)
$L280:
sll	$2,$2,2
addu	$2,$2,$16
lhu	$3,2432($2)
addiu	$3,$3,1
sh	$3,2432($2)
lw	$13,108($16)
lw	$11,112($16)
lw	$18,5784($16)
subu	$2,$13,$11
addiu	$13,$2,1
addiu	$11,$11,-2
lw	$17,5780($16)
lw	$12,100($16)
sw	$13,108($16)
sw	$11,112($16)
$L281:
addiu	$12,$12,1
addiu	$10,$11,-1
sltu	$2,$15,$12
move	$11,$10
sw	$12,100($16)
bne	$2,$0,$L282
move	$14,$12

lw	$4,48($16)
lw	$3,64($16)
addu	$4,$4,$12
lw	$6,80($16)
lbu	$5,2($4)
lw	$7,76($16)
sll	$3,$3,$6
xor	$3,$3,$5
and	$3,$3,$7
lw	$8,60($16)
lw	$2,44($16)
sll	$4,$3,1
addu	$4,$4,$8
lw	$9,56($16)
and	$2,$12,$2
lhu	$5,0($4)
sll	$2,$2,1
addu	$2,$2,$9
sh	$5,0($2)
move	$19,$5
sh	$12,0($4)
sw	$3,64($16)
$L282:
bne	$10,$0,$L281
sw	$10,112($16)

addiu	$12,$14,1
li	$2,2			# 0x2
addiu	$3,$17,-1
sw	$2,88($16)
sw	$0,96($16)
bne	$18,$3,$L336
sw	$12,100($16)

lw	$3,84($16)
bltz	$3,$L288
move	$5,$0

lw	$2,48($16)
addu	$5,$3,$2
$L288:
lui	$2,%hi(_tr_flush_block)
subu	$6,$12,$3
move	$4,$16
addiu	$2,$2,%lo(_tr_flush_block)
jal	$2
move	$7,$0

lw	$17,0($16)
lw	$12,100($16)
lw	$5,28($17)
lw	$4,16($17)
lw	$3,20($5)
move	$18,$4
sltu	$2,$4,$3
movz	$18,$3,$2
bne	$18,$0,$L341
sw	$12,84($16)

$L289:
lw	$2,16($17)
beq	$2,$0,$L351
move	$4,$0

lw	$13,108($16)
sltu	$2,$13,262
beq	$2,$0,$L352
sltu	$2,$13,3

lui	$2,%hi(fill_window)
$L350:
addiu	$2,$2,%lo(fill_window)
jal	$2
move	$4,$16

lw	$13,108($16)
sltu	$2,$13,262
beq	$2,$0,$L234
nop

beq	$20,$0,$L236
nop

$L234:
bne	$13,$0,$L352
sltu	$2,$13,3

lw	$2,96($16)
bne	$2,$0,$L342
nop

$L301:
lw	$3,84($16)
bltz	$3,$L305
move	$5,$0

lw	$2,48($16)
addu	$5,$3,$2
$L305:
lw	$6,100($16)
xori	$7,$20,0x4
lui	$2,%hi(_tr_flush_block)
subu	$6,$6,$3
move	$4,$16
addiu	$2,$2,%lo(_tr_flush_block)
jal	$2
sltu	$7,$7,1

lw	$17,0($16)
lw	$5,100($16)
lw	$6,28($17)
lw	$4,16($17)
lw	$3,20($6)
move	$18,$4
sltu	$2,$4,$3
movz	$18,$3,$2
bne	$18,$0,$L344
sw	$5,84($16)

$L306:
lw	$2,16($17)
bne	$2,$0,$L309
xori	$3,$20,0x4

li	$2,4			# 0x4
bne	$20,$2,$L236
li	$4,2			# 0x2

lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L275:
lw	$2,96($16)
beq	$2,$0,$L292
addiu	$2,$12,1

lw	$2,48($16)
lw	$5,5784($16)
addu	$2,$2,$12
lbu	$6,-1($2)
lw	$4,5788($16)
lw	$3,5776($16)
sll	$2,$5,1
addu	$2,$2,$4
addu	$3,$3,$5
sll	$4,$6,2
addiu	$5,$5,1
sh	$0,0($2)
addu	$4,$4,$16
sb	$6,0($3)
sw	$5,5784($16)
lhu	$2,140($4)
addiu	$2,$2,1
sh	$2,140($4)
lw	$3,5780($16)
lw	$2,5784($16)
addiu	$3,$3,-1
beq	$2,$3,$L345
nop

$L320:
lw	$12,100($16)
lw	$17,0($16)
$L294:
lw	$13,108($16)
addiu	$12,$12,1
addiu	$13,$13,-1
sw	$12,100($16)
sw	$13,108($16)
lw	$2,16($17)
bne	$2,$0,$L353
sltu	$2,$13,262

$L236:
move	$4,$0
$L351:
lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L345:
lw	$3,84($16)
bltz	$3,$L298
move	$5,$0

lw	$2,48($16)
addu	$5,$3,$2
$L298:
lw	$6,100($16)
lui	$2,%hi(_tr_flush_block)
subu	$6,$6,$3
move	$4,$16
addiu	$2,$2,%lo(_tr_flush_block)
jal	$2
move	$7,$0

lw	$17,0($16)
lw	$12,100($16)
lw	$5,28($17)
lw	$4,16($17)
lw	$3,20($5)
move	$18,$4
sltu	$2,$4,$3
movz	$18,$3,$2
beq	$18,$0,$L294
sw	$12,84($16)

lw	$5,16($5)
lw	$4,12($17)
lui	$2,%hi(memcpy)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$18

lw	$3,12($17)
lw	$2,16($17)
lw	$5,28($17)
addu	$3,$3,$18
subu	$2,$2,$18
sw	$3,12($17)
sw	$2,16($17)
lw	$4,16($5)
lw	$3,20($17)
lw	$2,20($5)
addu	$4,$4,$18
subu	$2,$2,$18
addu	$3,$3,$18
sw	$4,16($5)
sw	$3,20($17)
bne	$2,$0,$L320
sw	$2,20($5)

lw	$2,8($5)
lw	$12,100($16)
lw	$17,0($16)
j	$L294
sw	$2,16($5)

$L317:
j	$L238
lw	$12,100($16)

$L292:
lw	$13,108($16)
addiu	$3,$13,-1
li	$4,1			# 0x1
move	$13,$3
sw	$4,96($16)
sw	$2,100($16)
j	$L336
sw	$3,108($16)

$L278:
lui	$2,%hi(_dist_code)
addiu	$2,$2,%lo(_dist_code)
srl	$3,$7,7
addu	$3,$3,$2
j	$L280
lbu	$2,256($3)

$L339:
bne	$4,$2,$L240
nop

lw	$2,48($16)
addu	$5,$2,$19
addu	$4,$2,$12
lbu	$3,0($5)
lbu	$2,0($4)
beq	$3,$2,$L347
addiu	$11,$4,258

$L249:
li	$5,2			# 0x2
move	$4,$5
j	$L246
sw	$5,88($16)

$L341:
lw	$5,16($5)
lw	$4,12($17)
lui	$2,%hi(memcpy)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$18

lw	$3,12($17)
lw	$2,16($17)
lw	$5,28($17)
addu	$3,$3,$18
subu	$2,$2,$18
sw	$3,12($17)
sw	$2,16($17)
lw	$4,16($5)
lw	$3,20($17)
lw	$2,20($5)
addu	$4,$4,$18
subu	$2,$2,$18
addu	$3,$3,$18
sw	$4,16($5)
sw	$3,20($17)
beq	$2,$0,$L348
sw	$2,20($5)

j	$L289
lw	$17,0($16)

$L338:
lui	$2,%hi(longest_match)
move	$4,$16
addiu	$2,$2,%lo(longest_match)
jal	$2
move	$5,$19

lw	$12,100($16)
sw	$2,88($16)
move	$4,$2
$L246:
sltu	$2,$4,6
beq	$2,$0,$L240
li	$2,1			# 0x1

lw	$3,128($16)
bne	$3,$2,$L349
li	$2,2			# 0x2

j	$L240
sw	$2,88($16)

$L348:
lw	$2,8($5)
lw	$17,0($16)
j	$L289
sw	$2,16($5)

$L309:
li	$2,1			# 0x1
li	$4,3			# 0x3
movn	$4,$2,$3
lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L342:
lw	$3,100($16)
lw	$2,48($16)
lw	$4,5784($16)
addu	$2,$2,$3
lbu	$6,-1($2)
lw	$3,5788($16)
lw	$5,5776($16)
sll	$2,$4,1
addu	$2,$2,$3
addu	$5,$5,$4
sll	$3,$6,2
addiu	$4,$4,1
sh	$0,0($2)
addu	$3,$3,$16
sb	$6,0($5)
sw	$4,5784($16)
lhu	$2,140($3)
addiu	$2,$2,1
sh	$2,140($3)
j	$L301
sw	$0,96($16)

$L347:
lbu	$3,1($5)
lbu	$2,1($4)
bne	$3,$2,$L249
nop

addiu	$4,$4,2
addiu	$5,$5,2
$L252:
lbu	$3,1($4)
lbu	$2,1($5)
bne	$3,$2,$L327
addiu	$6,$4,1

lbu	$3,2($4)
lbu	$2,2($5)
addiu	$6,$4,2
addiu	$7,$4,3
bne	$3,$2,$L327
addiu	$9,$4,4

lbu	$3,3($4)
lbu	$2,3($5)
addiu	$8,$4,5
addiu	$6,$4,6
bne	$3,$2,$L324
addiu	$10,$4,7

lbu	$3,4($4)
lbu	$2,4($5)
bne	$3,$2,$L325
nop

lbu	$3,5($4)
lbu	$2,5($5)
bne	$3,$2,$L326
nop

lbu	$3,6($4)
lbu	$2,6($5)
bne	$3,$2,$L327
nop

lbu	$3,7($4)
lbu	$2,7($5)
addiu	$4,$4,8
addiu	$5,$5,8
bne	$3,$2,$L328
sltu	$6,$4,$11

lbu	$3,0($4)
lbu	$2,0($5)
bne	$3,$2,$L354
subu	$2,$11,$4

bne	$6,$0,$L252
nop

$L255:
subu	$2,$11,$4
$L354:
li	$3,258			# 0x102
subu	$2,$3,$2
slt	$4,$2,3
bne	$4,$0,$L249
nop

move	$5,$2
sltu	$2,$2,$13
movz	$5,$13,$2
move	$4,$5
sw	$19,104($16)
j	$L246
sw	$5,88($16)

$L344:
lw	$5,16($6)
lw	$4,12($17)
lui	$2,%hi(memcpy)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$18

lw	$3,12($17)
lw	$2,16($17)
lw	$5,28($17)
addu	$3,$3,$18
subu	$2,$2,$18
sw	$3,12($17)
sw	$2,16($17)
lw	$4,16($5)
lw	$3,20($17)
lw	$2,20($5)
addu	$4,$4,$18
subu	$2,$2,$18
addu	$3,$3,$18
sw	$4,16($5)
sw	$3,20($17)
bne	$2,$0,$L321
sw	$2,20($5)

lw	$2,8($5)
lw	$17,0($16)
j	$L306
sw	$2,16($5)

$L321:
j	$L306
lw	$17,0($16)

$L328:
j	$L255
move	$4,$10

$L326:
j	$L255
move	$4,$8

$L325:
j	$L255
move	$4,$9

$L324:
j	$L255
move	$4,$7

$L327:
j	$L255
move	$4,$6

$L349:
li	$2,3			# 0x3
bne	$4,$2,$L240
nop

lw	$2,104($16)
subu	$2,$12,$2
sltu	$2,$2,4097
bne	$2,$0,$L240
li	$2,2			# 0x2

j	$L240
sw	$2,88($16)

.set	macro
.set	reorder
.end	deflate_slow
.size	deflate_slow, .-deflate_slow
.section	.text.deflate_fast,"ax",@progbits
.align	2
.align	5
.ent	deflate_fast
deflate_fast:
.frame	$sp,40,$31		# vars= 0, regs= 6/0, args= 16, gp= 0
.mask	0x801f0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
sw	$20,32($sp)
sw	$19,28($sp)
sw	$17,20($sp)
sw	$31,36($sp)
sw	$18,24($sp)
sw	$16,16($sp)
lw	$7,108($4)
move	$17,$4
move	$20,$5
move	$19,$0
sltu	$2,$7,262
$L439:
bne	$2,$0,$L438
lui	$2,%hi(fill_window)

sltu	$2,$7,3
$L441:
bne	$2,$0,$L420
nop

lw	$9,100($17)
lw	$2,48($17)
lw	$3,64($17)
addu	$2,$2,$9
lw	$4,80($17)
lbu	$5,2($2)
sll	$3,$3,$4
lw	$2,76($17)
xor	$3,$3,$5
and	$6,$3,$2
lw	$4,60($17)
lw	$2,44($17)
sll	$3,$6,1
addu	$3,$3,$4
and	$2,$9,$2
lw	$4,56($17)
lhu	$5,0($3)
sll	$2,$2,1
addu	$2,$2,$4
sh	$5,0($2)
move	$19,$5
sh	$9,0($3)
sw	$6,64($17)
$L363:
beq	$19,$0,$L365
subu	$4,$9,$19

lw	$2,36($17)
addiu	$2,$2,-262
sltu	$2,$2,$4
bne	$2,$0,$L365
nop

lw	$3,128($17)
slt	$2,$3,2
bne	$2,$0,$L431
li	$2,3			# 0x3

beq	$3,$2,$L432
li	$2,1			# 0x1

$L365:
lw	$7,88($17)
sltu	$2,$7,3
bne	$2,$0,$L388
lui	$2,%hi(_length_code)

addiu	$7,$7,-3
andi	$7,$7,0x00ff
addiu	$2,$2,%lo(_length_code)
addu	$2,$7,$2
lw	$5,5784($17)
lw	$3,104($17)
lbu	$4,0($2)
lw	$8,5788($17)
lw	$6,5776($17)
sll	$2,$5,1
subu	$3,$9,$3
addiu	$4,$4,257
addu	$2,$2,$8
andi	$3,$3,0xffff
addu	$6,$6,$5
sll	$4,$4,2
addiu	$5,$5,1
sh	$3,0($2)
addu	$4,$4,$17
sb	$7,0($6)
sw	$5,5784($17)
addiu	$3,$3,-1
lhu	$2,140($4)
andi	$5,$3,0xffff
addiu	$2,$2,1
sltu	$3,$5,256
beq	$3,$0,$L390
sh	$2,140($4)

lui	$2,%hi(_dist_code)
addiu	$2,$2,%lo(_dist_code)
addu	$2,$5,$2
lbu	$2,0($2)
$L392:
sll	$3,$2,2
addu	$3,$3,$17
lhu	$2,2432($3)
addiu	$2,$2,1
sh	$2,2432($3)
lw	$4,5780($17)
lw	$7,108($17)
lw	$5,88($17)
lw	$2,5784($17)
lw	$3,120($17)
addiu	$4,$4,-1
xor	$2,$2,$4
subu	$7,$7,$5
sltu	$3,$3,$5
sltu	$16,$2,1
bne	$3,$0,$L393
sw	$7,108($17)

sltu	$2,$7,3
bne	$2,$0,$L393
nop

addiu	$5,$5,-1
lw	$6,64($17)
lw	$15,80($17)
lw	$14,48($17)
lw	$9,100($17)
lw	$13,76($17)
lw	$12,60($17)
lw	$11,44($17)
lw	$10,56($17)
sw	$5,88($17)
$L396:
move	$8,$9
addiu	$9,$9,1
addu	$3,$14,$9
sw	$9,100($17)
lbu	$4,2($3)
sll	$2,$6,$15
xor	$2,$2,$4
and	$6,$2,$13
sll	$3,$6,1
addu	$3,$3,$12
and	$2,$9,$11
lhu	$4,0($3)
sll	$2,$2,1
addu	$2,$2,$10
addiu	$5,$5,-1
sh	$4,0($2)
sw	$6,64($17)
sh	$9,0($3)
move	$19,$4
bne	$5,$0,$L396
sw	$5,88($17)

addiu	$9,$8,2
sw	$9,100($17)
$L398:
beq	$16,$0,$L439
sltu	$2,$7,262

lw	$3,84($17)
bltz	$3,$L402
move	$5,$0

lw	$2,48($17)
addu	$5,$3,$2
$L402:
lw	$6,100($17)
lui	$2,%hi(_tr_flush_block)
subu	$6,$6,$3
move	$4,$17
addiu	$2,$2,%lo(_tr_flush_block)
jal	$2
move	$7,$0

lw	$18,0($17)
lw	$9,100($17)
lw	$5,28($18)
lw	$4,16($18)
lw	$3,20($5)
move	$16,$4
sltu	$2,$4,$3
movz	$16,$3,$2
bne	$16,$0,$L434
sw	$9,84($17)

$L403:
lw	$2,16($18)
beq	$2,$0,$L440
move	$4,$0

lw	$7,108($17)
sltu	$2,$7,262
beq	$2,$0,$L441
sltu	$2,$7,3

lui	$2,%hi(fill_window)
$L438:
addiu	$2,$2,%lo(fill_window)
jal	$2
move	$4,$17

lw	$7,108($17)
sltu	$2,$7,262
beq	$2,$0,$L359
nop

beq	$20,$0,$L361
nop

$L359:
bne	$7,$0,$L441
sltu	$2,$7,3

lw	$3,84($17)
bltz	$3,$L408
move	$5,$0

lw	$2,48($17)
addu	$5,$3,$2
$L408:
lw	$6,100($17)
xori	$7,$20,0x4
lui	$2,%hi(_tr_flush_block)
subu	$6,$6,$3
move	$4,$17
addiu	$2,$2,%lo(_tr_flush_block)
jal	$2
sltu	$7,$7,1

lw	$18,0($17)
lw	$5,100($17)
lw	$6,28($18)
lw	$4,16($18)
lw	$3,20($6)
move	$16,$4
sltu	$2,$4,$3
movz	$16,$3,$2
bne	$16,$0,$L436
sw	$5,84($17)

$L409:
lw	$2,16($18)
bne	$2,$0,$L412
xori	$3,$20,0x4

li	$2,4			# 0x4
bne	$20,$2,$L361
li	$4,2			# 0x2

lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L388:
lw	$2,48($17)
lw	$5,5784($17)
addu	$2,$2,$9
lbu	$6,0($2)
lw	$4,5788($17)
lw	$3,5776($17)
sll	$2,$5,1
addu	$2,$2,$4
addu	$3,$3,$5
sll	$4,$6,2
addiu	$5,$5,1
sh	$0,0($2)
addu	$4,$4,$17
sb	$6,0($3)
sw	$5,5784($17)
lhu	$2,140($4)
addiu	$2,$2,1
sh	$2,140($4)
lw	$3,5780($17)
lw	$2,5784($17)
lw	$7,108($17)
lw	$9,100($17)
addiu	$3,$3,-1
xor	$2,$2,$3
addiu	$7,$7,-1
addiu	$9,$9,1
sltu	$16,$2,1
sw	$9,100($17)
j	$L398
sw	$7,108($17)

$L420:
j	$L363
lw	$9,100($17)

$L393:
lw	$9,100($17)
lw	$3,48($17)
addu	$9,$9,$5
addu	$3,$3,$9
sw	$9,100($17)
sw	$0,88($17)
lbu	$6,0($3)
lw	$2,80($17)
sw	$6,64($17)
lbu	$4,1($3)
sll	$2,$6,$2
lw	$3,76($17)
xor	$2,$2,$4
and	$2,$2,$3
j	$L398
sw	$2,64($17)

$L361:
move	$4,$0
$L440:
lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L390:
lui	$2,%hi(_dist_code)
addiu	$2,$2,%lo(_dist_code)
srl	$3,$5,7
addu	$3,$3,$2
j	$L392
lbu	$2,256($3)

$L434:
lw	$5,16($5)
lw	$4,12($18)
lui	$2,%hi(memcpy)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$16

lw	$3,12($18)
lw	$2,16($18)
lw	$5,28($18)
addu	$3,$3,$16
subu	$2,$2,$16
sw	$3,12($18)
sw	$2,16($18)
lw	$4,16($5)
lw	$3,20($18)
lw	$2,20($5)
addu	$4,$4,$16
subu	$2,$2,$16
addu	$3,$3,$16
sw	$4,16($5)
sw	$3,20($18)
bne	$2,$0,$L421
sw	$2,20($5)

lw	$2,8($5)
lw	$18,0($17)
j	$L403
sw	$2,16($5)

$L421:
j	$L403
lw	$18,0($17)

$L432:
bne	$4,$2,$L365
nop

lw	$2,48($17)
addu	$5,$2,$19
addu	$4,$2,$9
lbu	$3,0($5)
lbu	$2,0($4)
beq	$3,$2,$L437
addiu	$10,$4,258

$L372:
li	$5,2			# 0x2
$L442:
j	$L365
sw	$5,88($17)

$L431:
lui	$2,%hi(longest_match)
move	$4,$17
addiu	$2,$2,%lo(longest_match)
jal	$2
move	$5,$19

lw	$9,100($17)
j	$L365
sw	$2,88($17)

$L437:
lbu	$3,1($5)
lbu	$2,1($4)
bne	$3,$2,$L372
addiu	$4,$4,2

addiu	$6,$5,2
$L375:
lbu	$3,1($4)
lbu	$2,1($6)
bne	$3,$2,$L376
addiu	$5,$4,1

lbu	$3,2($4)
lbu	$2,2($6)
bne	$3,$2,$L376
addiu	$5,$4,2

lbu	$3,3($4)
lbu	$2,3($6)
bne	$3,$2,$L376
addiu	$5,$4,3

lbu	$3,4($4)
lbu	$2,4($6)
bne	$3,$2,$L376
addiu	$5,$4,4

lbu	$3,5($4)
lbu	$2,5($6)
bne	$3,$2,$L376
addiu	$5,$4,5

lbu	$3,6($4)
lbu	$2,6($6)
bne	$3,$2,$L376
addiu	$5,$4,6

lbu	$2,7($6)
lbu	$3,7($4)
addiu	$5,$4,7
bne	$3,$2,$L376
addiu	$6,$6,8

lbu	$3,8($4)
lbu	$2,0($6)
addiu	$5,$4,8
sltu	$8,$5,$10
bne	$3,$2,$L376
move	$4,$5

bne	$8,$0,$L375
nop

$L376:
subu	$2,$10,$5
li	$3,258			# 0x102
subu	$2,$3,$2
slt	$4,$2,3
bne	$4,$0,$L442
li	$5,2			# 0x2

move	$5,$2
sltu	$2,$2,$7
movz	$5,$7,$2
sw	$19,104($17)
j	$L365
sw	$5,88($17)

$L412:
li	$2,1			# 0x1
li	$4,3			# 0x3
movn	$4,$2,$3
lw	$31,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L436:
lw	$5,16($6)
lw	$4,12($18)
lui	$2,%hi(memcpy)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$16

lw	$3,12($18)
lw	$2,16($18)
lw	$5,28($18)
addu	$3,$3,$16
subu	$2,$2,$16
sw	$3,12($18)
sw	$2,16($18)
lw	$4,16($5)
lw	$3,20($18)
lw	$2,20($5)
addu	$4,$4,$16
subu	$2,$2,$16
addu	$3,$3,$16
sw	$4,16($5)
sw	$3,20($18)
bne	$2,$0,$L422
sw	$2,20($5)

lw	$2,8($5)
lw	$18,0($17)
j	$L409
sw	$2,16($5)

$L422:
j	$L409
lw	$18,0($17)

.set	macro
.set	reorder
.end	deflate_fast
.size	deflate_fast, .-deflate_fast
.section	.text.deflate_stored,"ax",@progbits
.align	2
.align	5
.ent	deflate_stored
deflate_stored:
.frame	$sp,40,$31		# vars= 0, regs= 6/0, args= 16, gp= 0
.mask	0x801f0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
sw	$20,32($sp)
sw	$19,28($sp)
sw	$18,24($sp)
sw	$31,36($sp)
sw	$17,20($sp)
sw	$16,16($sp)
lw	$3,12($4)
li	$2,65535			# 0xffff
addiu	$19,$3,-5
sltu	$2,$19,$2
move	$18,$4
bne	$2,$0,$L490
move	$20,$5

li	$19,65535			# 0xffff
$L490:
lw	$3,108($18)
sltu	$2,$3,2
bne	$2,$0,$L491
lui	$2,%hi(fill_window)

$L446:
lw	$6,100($18)
lw	$5,84($18)
addu	$6,$6,$3
sw	$6,100($18)
sw	$0,108($18)
beq	$6,$0,$L451
addu	$3,$19,$5

sltu	$2,$6,$3
bne	$2,$0,$L453
nop

$L451:
subu	$2,$6,$3
sw	$2,108($18)
bltz	$5,$L492
sw	$3,100($18)

lw	$2,48($18)
addu	$2,$5,$2
$L456:
subu	$6,$3,$5
move	$5,$2
lui	$2,%hi(_tr_flush_block)
move	$4,$18
addiu	$2,$2,%lo(_tr_flush_block)
jal	$2
move	$7,$0

lw	$17,0($18)
lw	$5,100($18)
lw	$6,28($17)
lw	$4,16($17)
lw	$3,20($6)
move	$16,$4
sltu	$2,$4,$3
movz	$16,$3,$2
bne	$16,$0,$L493
sw	$5,84($18)

$L457:
lw	$2,16($17)
beq	$2,$0,$L476
move	$4,$0

lw	$6,100($18)
lw	$5,84($18)
$L453:
lw	$2,36($18)
subu	$6,$6,$5
addiu	$2,$2,-262
sltu	$2,$6,$2
bne	$2,$0,$L490
nop

bltz	$5,$L494
nop

lw	$2,48($18)
addu	$5,$5,$2
$L463:
lui	$2,%hi(_tr_flush_block)
move	$4,$18
addiu	$2,$2,%lo(_tr_flush_block)
jal	$2
move	$7,$0

lw	$17,0($18)
lw	$5,100($18)
lw	$6,28($17)
lw	$4,16($17)
lw	$3,20($6)
move	$16,$4
sltu	$2,$4,$3
movz	$16,$3,$2
bne	$16,$0,$L495
sw	$5,84($18)

$L464:
lw	$2,16($17)
bne	$2,$0,$L490
nop

$L449:
move	$4,$0
$L476:
lw	$31,36($sp)
$L498:
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L493:
lw	$5,16($6)
lw	$4,12($17)
lui	$2,%hi(memcpy)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$16

lw	$3,12($17)
lw	$2,16($17)
lw	$5,28($17)
addu	$3,$3,$16
subu	$2,$2,$16
sw	$3,12($17)
sw	$2,16($17)
lw	$4,16($5)
lw	$3,20($17)
lw	$2,20($5)
addu	$4,$4,$16
subu	$2,$2,$16
addu	$3,$3,$16
sw	$4,16($5)
sw	$3,20($17)
bne	$2,$0,$L481
sw	$2,20($5)

lw	$2,8($5)
lw	$17,0($18)
j	$L457
sw	$2,16($5)

$L492:
j	$L456
move	$2,$0

$L495:
lw	$5,16($6)
lw	$4,12($17)
lui	$2,%hi(memcpy)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$16

lw	$3,12($17)
lw	$2,16($17)
lw	$5,28($17)
addu	$3,$3,$16
subu	$2,$2,$16
sw	$3,12($17)
sw	$2,16($17)
lw	$4,16($5)
lw	$3,20($17)
lw	$2,20($5)
addu	$4,$4,$16
subu	$2,$2,$16
addu	$3,$3,$16
sw	$4,16($5)
sw	$3,20($17)
bne	$2,$0,$L482
sw	$2,20($5)

lw	$2,8($5)
lw	$17,0($18)
j	$L464
sw	$2,16($5)

$L481:
j	$L457
lw	$17,0($18)

$L491:
addiu	$2,$2,%lo(fill_window)
jal	$2
move	$4,$18

lw	$3,108($18)
bne	$3,$0,$L446
nop

beq	$20,$0,$L476
move	$4,$0

lw	$3,84($18)
bltz	$3,$L469
move	$5,$0

lw	$2,48($18)
addu	$5,$3,$2
$L469:
lw	$6,100($18)
xori	$7,$20,0x4
lui	$2,%hi(_tr_flush_block)
subu	$6,$6,$3
sltu	$7,$7,1
addiu	$2,$2,%lo(_tr_flush_block)
jal	$2
move	$4,$18

lw	$17,0($18)
lw	$5,100($18)
lw	$7,28($17)
lw	$4,16($17)
lw	$3,20($7)
move	$16,$4
sltu	$2,$4,$3
movz	$16,$3,$2
bne	$16,$0,$L497
sw	$5,84($18)

$L470:
lw	$2,16($17)
bne	$2,$0,$L473
xori	$3,$20,0x4

li	$2,4			# 0x4
bne	$20,$2,$L449
li	$4,2			# 0x2

j	$L498
lw	$31,36($sp)

$L494:
j	$L463
move	$5,$0

$L482:
j	$L464
lw	$17,0($18)

$L473:
li	$4,3			# 0x3
li	$2,1			# 0x1
j	$L476
movn	$4,$2,$3

$L497:
lw	$5,16($7)
lw	$4,12($17)
lui	$2,%hi(memcpy)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$16

lw	$3,12($17)
lw	$2,16($17)
lw	$5,28($17)
addu	$3,$3,$16
subu	$2,$2,$16
sw	$3,12($17)
sw	$2,16($17)
lw	$4,16($5)
lw	$3,20($17)
lw	$2,20($5)
addu	$4,$4,$16
subu	$2,$2,$16
addu	$3,$3,$16
sw	$4,16($5)
sw	$3,20($17)
bne	$2,$0,$L483
sw	$2,20($5)

lw	$2,8($5)
lw	$17,0($18)
j	$L470
sw	$2,16($5)

$L483:
j	$L470
lw	$17,0($18)

.set	macro
.set	reorder
.end	deflate_stored
.size	deflate_stored, .-deflate_stored
.section	.text.deflate,"ax",@progbits
.align	2
.align	5
.globl	deflate
.ent	deflate
deflate:
.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
.mask	0x800f0000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-40
sw	$18,24($sp)
sw	$16,16($sp)
sw	$31,32($sp)
sw	$19,28($sp)
sw	$17,20($sp)
move	$16,$4
bne	$4,$0,$L589
move	$18,$5

$L500:
li	$4,-2			# 0xfffffffffffffffe
$L511:
lw	$31,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,40

$L589:
lw	$17,28($4)
beq	$17,$0,$L500
slt	$2,$5,5

beq	$2,$0,$L500
nop

bltz	$5,$L500
nop

lw	$2,12($4)
beq	$2,$0,$L610
lui	$2,%hi(z_errmsg+16)

lw	$2,0($4)
beq	$2,$0,$L590
nop

lw	$6,4($17)
$L607:
li	$2,666			# 0x29a
beq	$6,$2,$L591
li	$2,4			# 0x4

$L509:
lw	$2,16($16)
beq	$2,$0,$L587
li	$2,42			# 0x2a

lw	$19,32($17)
sw	$16,0($17)
beq	$6,$2,$L592
sw	$18,32($17)

$L514:
lw	$2,20($17)
beq	$2,$0,$L536
nop

lw	$5,28($16)
lw	$4,16($16)
lw	$3,20($5)
move	$19,$4
sltu	$2,$4,$3
movz	$19,$3,$2
bne	$19,$0,$L593
lui	$2,%hi(memcpy)

$L538:
beq	$4,$0,$L611
li	$2,-1			# 0xffffffffffffffff

lw	$3,4($16)
$L541:
lw	$4,4($17)
li	$2,666			# 0x29a
beq	$4,$2,$L594
nop

bne	$3,$0,$L550
nop

$L548:
lw	$2,108($17)
bne	$2,$0,$L550
nop

beq	$18,$0,$L552
li	$2,666			# 0x29a

beq	$4,$2,$L612
li	$2,4			# 0x4

$L550:
lw	$3,124($17)
lui	$2,%hi(configuration_table)
sll	$4,$3,4
sll	$3,$3,2
subu	$4,$4,$3
addiu	$2,$2,%lo(configuration_table)
addu	$4,$4,$2
lw	$2,8($4)
move	$5,$18
jal	$2
move	$4,$17

move	$3,$2
addiu	$2,$2,-2
sltu	$2,$2,2
bne	$2,$0,$L595
li	$2,666			# 0x29a

$L555:
beq	$3,$0,$L557
li	$2,2			# 0x2

beq	$3,$2,$L557
li	$2,1			# 0x1

beq	$3,$2,$L596
li	$2,4			# 0x4

$L612:
beq	$18,$2,$L597
nop

$L552:
j	$L511
move	$4,$0

$L595:
j	$L555
sw	$2,4($17)

$L593:
lw	$5,16($5)
lw	$4,12($16)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$19

lw	$2,16($16)
lw	$3,12($16)
lw	$5,28($16)
subu	$4,$2,$19
addu	$3,$3,$19
sw	$4,16($16)
sw	$3,12($16)
lw	$3,16($5)
lw	$4,20($16)
lw	$2,20($5)
addu	$3,$3,$19
subu	$2,$2,$19
addu	$4,$4,$19
sw	$3,16($5)
sw	$4,20($16)
bne	$2,$0,$L582
sw	$2,20($5)

lw	$2,8($5)
lw	$4,16($16)
j	$L538
sw	$2,16($5)

$L536:
lw	$3,4($16)
bne	$3,$0,$L541
slt	$2,$19,$18

bne	$2,$0,$L541
li	$2,4			# 0x4

beq	$18,$2,$L541
nop

$L587:
lui	$2,%hi(z_errmsg+28)
lw	$3,%lo(z_errmsg+28)($2)
$L613:
li	$4,-5			# 0xfffffffffffffffb
j	$L511
sw	$3,24($16)

$L591:
beq	$18,$2,$L509
lui	$2,%hi(z_errmsg+16)

$L610:
lw	$3,%lo(z_errmsg+16)($2)
li	$4,-2			# 0xfffffffffffffffe
j	$L511
sw	$3,24($16)

$L557:
lw	$2,16($16)
bne	$2,$0,$L552
li	$2,-1			# 0xffffffffffffffff

move	$4,$0
$L611:
j	$L511
sw	$2,32($17)

$L592:
lw	$3,24($17)
li	$2,2			# 0x2
beq	$3,$2,$L598
li	$4,31

lw	$2,128($17)
slt	$2,$2,2
bne	$2,$0,$L599
lw	$5,40($17)

$L524:
move	$4,$0
$L529:
sll	$3,$5,12
addiu	$3,$3,-30720
lw	$5,100($17)
or	$3,$4,$3
ori	$4,$3,0x20
li	$2,138543104			# 0x8420000
movn	$3,$4,$5
ori	$2,$2,0x1085
multu	$3,$2
lw	$7,20($17)
mfhi	$2
li	$6,113			# 0x71
subu	$3,$3,$2
srl	$3,$3,1
addu	$2,$2,$3
srl	$2,$2,4
sll	$4,$2,5
lw	$3,8($17)
subu	$4,$4,$2
addiu	$4,$4,31
addu	$3,$3,$7
srl	$5,$4,8
sw	$6,4($17)
sb	$5,0($3)
lw	$2,8($17)
addiu	$6,$7,2
addu	$2,$7,$2
sb	$4,1($2)
lw	$3,100($17)
beq	$3,$0,$L534
sw	$6,20($17)

lw	$4,48($16)
lw	$2,8($17)
srl	$5,$4,24
addu	$2,$2,$6
sb	$5,0($2)
lw	$3,8($17)
srl	$4,$4,16
addu	$3,$7,$3
sb	$4,3($3)
lhu	$5,48($16)
lw	$2,8($17)
srl	$4,$5,8
addu	$2,$7,$2
sb	$4,4($2)
lw	$3,8($17)
addiu	$2,$7,6
addu	$3,$7,$3
sb	$5,5($3)
sw	$2,20($17)
$L534:
lui	$2,%hi(adler32)
move	$4,$0
move	$5,$0
addiu	$2,$2,%lo(adler32)
jal	$2
move	$6,$0

j	$L514
sw	$2,48($16)

$L590:
lw	$2,4($4)
bne	$2,$0,$L610
lui	$2,%hi(z_errmsg+16)

j	$L607
lw	$6,4($17)

$L597:
lw	$3,24($17)
blez	$3,$L600
li	$2,2			# 0x2

beq	$3,$2,$L601
nop

lw	$4,20($17)
lw	$5,48($16)
lw	$2,8($17)
srl	$6,$5,24
addu	$2,$2,$4
sb	$6,0($2)
lw	$3,8($17)
srl	$5,$5,16
addu	$3,$4,$3
sb	$5,1($3)
lhu	$6,48($16)
lw	$2,8($17)
srl	$3,$6,8
addu	$2,$4,$2
sb	$3,2($2)
lw	$5,8($17)
addiu	$2,$4,4
addu	$4,$4,$5
sb	$6,3($4)
sw	$2,20($17)
$L575:
lw	$5,28($16)
lw	$4,16($16)
lw	$3,20($5)
move	$18,$4
sltu	$2,$4,$3
movz	$18,$3,$2
bne	$18,$0,$L602
lui	$2,%hi(memcpy)

$L576:
lw	$2,24($17)
blez	$2,$L579
subu	$2,$0,$2

sw	$2,24($17)
$L579:
lw	$2,20($17)
j	$L511
sltu	$4,$2,1

$L594:
beq	$3,$0,$L548
lui	$2,%hi(z_errmsg+28)

j	$L613
lw	$3,%lo(z_errmsg+28)($2)

$L600:
j	$L511
li	$4,1			# 0x1

$L602:
lw	$5,16($5)
lw	$4,12($16)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$18

lw	$3,12($16)
lw	$2,16($16)
lw	$5,28($16)
addu	$3,$3,$18
subu	$2,$2,$18
sw	$3,12($16)
sw	$2,16($16)
lw	$4,16($5)
lw	$3,20($16)
lw	$2,20($5)
addu	$4,$4,$18
subu	$2,$2,$18
addu	$3,$3,$18
sw	$4,16($5)
sw	$3,20($16)
bne	$2,$0,$L576
sw	$2,20($5)

lw	$2,8($5)
j	$L576
sw	$2,16($5)

$L601:
lw	$5,20($17)
lw	$2,8($17)
lw	$3,48($16)
addu	$2,$2,$5
sb	$3,0($2)
lw	$4,8($17)
lw	$2,48($16)
addu	$4,$5,$4
srl	$2,$2,8
sb	$2,1($4)
lw	$3,8($17)
lhu	$4,50($16)
addu	$3,$5,$3
sb	$4,2($3)
lw	$2,8($17)
lbu	$4,51($16)
addu	$2,$5,$2
sb	$4,3($2)
lw	$3,8($17)
lw	$2,8($16)
addu	$3,$5,$3
sb	$2,4($3)
lw	$4,8($17)
lw	$2,8($16)
addu	$4,$5,$4
srl	$2,$2,8
sb	$2,5($4)
lw	$3,8($17)
lhu	$4,10($16)
addu	$3,$5,$3
sb	$4,6($3)
lw	$2,8($17)
lbu	$3,11($16)
addu	$2,$5,$2
addiu	$5,$5,8
sb	$3,7($2)
j	$L575
sw	$5,20($17)

$L598:
lw	$5,20($17)
lw	$2,8($17)
addu	$2,$2,$5
sb	$4,0($2)
lw	$3,8($17)
li	$4,-117
addu	$3,$5,$3
sb	$4,1($3)
lw	$2,8($17)
li	$4,8
addu	$2,$5,$2
sb	$4,2($2)
lw	$3,8($17)
addiu	$6,$5,8
addu	$3,$5,$3
sb	$0,3($3)
lw	$2,8($17)
addu	$2,$5,$2
sb	$0,4($2)
lw	$3,8($17)
addu	$3,$5,$3
sb	$0,5($3)
lw	$2,8($17)
addu	$2,$5,$2
sb	$0,6($2)
lw	$3,8($17)
li	$2,9			# 0x9
addu	$5,$5,$3
sb	$0,7($5)
lw	$3,124($17)
sw	$6,20($17)
beq	$3,$2,$L603
lw	$4,8($17)

lw	$2,128($17)
slt	$2,$2,2
bne	$2,$0,$L604
slt	$2,$3,2

$L521:
li	$3,4			# 0x4
$L520:
addu	$2,$4,$6
$L614:
sb	$3,0($2)
lw	$3,8($17)
li	$2,-1
addu	$3,$6,$3
sb	$2,1($3)
addiu	$4,$6,2
lui	$2,%hi(crc32)
li	$6,113			# 0x71
sw	$4,20($17)
sw	$6,4($17)
move	$4,$0
move	$5,$0
addiu	$2,$2,%lo(crc32)
jal	$2
move	$6,$0

j	$L514
sw	$2,48($16)

$L599:
lw	$3,124($17)
slt	$2,$3,2
bne	$2,$0,$L524
slt	$2,$3,6

beq	$2,$0,$L527
xori	$3,$3,0x6

j	$L529
li	$4,64			# 0x40

$L604:
bne	$2,$0,$L521
move	$3,$0

j	$L614
addu	$2,$4,$6

$L596:
beq	$18,$3,$L605
lui	$2,%hi(_tr_stored_block)

move	$4,$17
move	$5,$0
move	$6,$0
addiu	$2,$2,%lo(_tr_stored_block)
jal	$2
move	$7,$0

li	$3,3			# 0x3
bne	$18,$3,$L564
nop

lw	$6,68($17)
lw	$4,60($17)
sll	$6,$6,1
addu	$2,$6,$4
sh	$0,-2($2)
lui	$2,%hi(memset)
addiu	$6,$6,-2
addiu	$2,$2,%lo(memset)
jal	$2
move	$5,$0

$L564:
lw	$5,28($16)
$L609:
lw	$4,16($16)
lw	$3,20($5)
move	$19,$4
sltu	$2,$4,$3
movz	$19,$3,$2
bne	$19,$0,$L606
lui	$2,%hi(memcpy)

$L566:
bne	$4,$0,$L612
li	$2,4			# 0x4

li	$2,-1			# 0xffffffffffffffff
j	$L511
sw	$2,32($17)

$L605:
lui	$2,%hi(_tr_align)
addiu	$2,$2,%lo(_tr_align)
jal	$2
move	$4,$17

j	$L609
lw	$5,28($16)

$L603:
j	$L520
li	$3,2			# 0x2

$L527:
li	$4,128			# 0x80
li	$2,192			# 0xc0
j	$L529
movn	$4,$2,$3

$L582:
j	$L538
lw	$4,16($16)

$L606:
lw	$5,16($5)
lw	$4,12($16)
addiu	$2,$2,%lo(memcpy)
jal	$2
move	$6,$19

lw	$2,16($16)
lw	$3,12($16)
lw	$5,28($16)
subu	$4,$2,$19
addu	$3,$3,$19
sw	$4,16($16)
sw	$3,12($16)
lw	$3,16($5)
lw	$4,20($16)
lw	$2,20($5)
addu	$3,$3,$19
subu	$2,$2,$19
addu	$4,$4,$19
sw	$3,16($5)
sw	$4,20($16)
bne	$2,$0,$L584
sw	$2,20($5)

lw	$2,8($5)
lw	$4,16($16)
j	$L566
sw	$2,16($5)

$L584:
j	$L566
lw	$4,16($16)

.set	macro
.set	reorder
.end	deflate
.size	deflate, .-deflate
.section	.text.deflateParams,"ax",@progbits
.align	2
.align	5
.globl	deflateParams
.ent	deflateParams
deflateParams:
.frame	$sp,48,$31		# vars= 0, regs= 7/0, args= 16, gp= 0
.mask	0x803f0000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-48
sw	$17,20($sp)
sw	$16,16($sp)
sw	$31,40($sp)
sw	$21,36($sp)
sw	$20,32($sp)
sw	$19,28($sp)
sw	$18,24($sp)
move	$7,$4
move	$16,$5
bne	$4,$0,$L632
move	$17,$6

$L616:
li	$7,-2			# 0xfffffffffffffffe
$L630:
lw	$31,40($sp)
lw	$21,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
move	$2,$7
j	$31
addiu	$sp,$sp,48

$L632:
lw	$18,28($4)
beq	$18,$0,$L616
li	$2,-1			# 0xffffffffffffffff

beq	$5,$2,$L633
sltu	$2,$5,10

beq	$2,$0,$L616
nop

bltz	$17,$L616
slt	$2,$17,4

$L636:
beq	$2,$0,$L616
lui	$21,%hi(configuration_table)

lw	$6,124($18)
sll	$2,$6,2
sll	$4,$6,4
sll	$19,$16,2
sll	$20,$16,4
addiu	$3,$21,%lo(configuration_table)
subu	$4,$4,$2
subu	$2,$20,$19
addu	$4,$4,$3
addu	$2,$2,$3
lw	$5,8($4)
lw	$3,8($2)
beq	$5,$3,$L624
nop

lw	$2,8($7)
bne	$2,$0,$L634
lui	$2,%hi(deflate)

$L624:
move	$7,$0
$L627:
beq	$6,$16,$L628
addiu	$3,$21,%lo(configuration_table)

subu	$2,$20,$19
addu	$2,$2,$3
lhu	$6,6($2)
lhu	$3,2($2)
lhu	$4,0($2)
lhu	$5,4($2)
sw	$16,124($18)
sw	$3,120($18)
sw	$4,132($18)
sw	$5,136($18)
sw	$6,116($18)
$L628:
j	$L630
sw	$17,128($18)

$L634:
move	$4,$7
addiu	$2,$2,%lo(deflate)
jal	$2
li	$5,1			# 0x1

lw	$6,124($18)
j	$L627
move	$7,$2

$L633:
bltz	$17,$L616
li	$16,6			# 0x6

j	$L636
slt	$2,$17,4

.set	macro
.set	reorder
.end	deflateParams
.size	deflateParams, .-deflateParams
.globl	deflate_copyright
.rdata
.align	2
.type	deflate_copyright, @object
.size	deflate_copyright, 53
deflate_copyright:
.ascii	" deflate 1.2.1 Copyright 1995-2003 Jean-loup Gailly \000"
.align	2
.type	configuration_table, @object
.size	configuration_table, 120
configuration_table:
.half	0
.half	0
.half	0
.half	0
.word	deflate_stored
.half	4
.half	4
.half	8
.half	4
.word	deflate_fast
.half	4
.half	5
.half	16
.half	8
.word	deflate_fast
.half	4
.half	6
.half	32
.half	32
.word	deflate_fast
.half	4
.half	4
.half	16
.half	16
.word	deflate_slow
.half	8
.half	16
.half	32
.half	32
.word	deflate_slow
.half	8
.half	16
.half	128
.half	128
.word	deflate_slow
.half	8
.half	32
.half	128
.half	256
.word	deflate_slow
.half	32
.half	128
.half	258
.half	1024
.word	deflate_slow
.half	32
.half	258
.half	258
.half	4096
.word	deflate_slow
.ident	"GCC: (GNU) 4.1.2"
