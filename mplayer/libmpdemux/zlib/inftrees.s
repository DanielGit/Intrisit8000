.file	1 "inftrees.c"
.section .mdebug.abi32
.previous
.section	.text.inflate_table,"ax",@progbits
.align	2
.align	5
.globl	inflate_table
.ent	inflate_table
inflate_table:
.frame	$sp,104,$31		# vars= 64, regs= 9/0, args= 0, gp= 0
.mask	0x40ff0000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-104
sw	$16,64($sp)
sw	$fp,96($sp)
sw	$23,92($sp)
sw	$22,88($sp)
sw	$21,84($sp)
sw	$20,80($sp)
sw	$19,76($sp)
sw	$18,72($sp)
sw	$17,68($sp)
move	$16,$6
sw	$4,104($sp)
sw	$5,108($sp)
sw	$7,116($sp)
sh	$0,32($sp)
sh	$0,34($sp)
sh	$0,36($sp)
sh	$0,38($sp)
sh	$0,40($sp)
sh	$0,42($sp)
sh	$0,44($sp)
sh	$0,46($sp)
sh	$0,48($sp)
sh	$0,50($sp)
sh	$0,52($sp)
sh	$0,54($sp)
sh	$0,56($sp)
sh	$0,58($sp)
sh	$0,60($sp)
sh	$0,62($sp)
beq	$6,$0,$L2
lw	$25,124($sp)

move	$4,$5
move	$5,$0
$L4:
lhu	$3,0($4)
addiu	$5,$5,1
sll	$3,$3,1
addu	$3,$3,$sp
lhu	$2,32($3)
addiu	$4,$4,2
addiu	$2,$2,1
bne	$16,$5,$L4
sh	$2,32($3)

lw	$2,120($sp)
lhu	$3,62($sp)
bne	$3,$0,$L158
lw	$20,0($2)

$L6:
lhu	$2,60($sp)
bne	$2,$0,$L159
lhu	$2,58($sp)

bne	$2,$0,$L160
lhu	$2,56($sp)

bne	$2,$0,$L161
lhu	$2,54($sp)

bne	$2,$0,$L162
lhu	$2,52($sp)

bne	$2,$0,$L163
lhu	$2,50($sp)

bne	$2,$0,$L164
lhu	$2,48($sp)

bne	$2,$0,$L165
lhu	$2,46($sp)

bne	$2,$0,$L166
lhu	$2,44($sp)

bne	$2,$0,$L167
lhu	$2,42($sp)

bne	$2,$0,$L168
lhu	$2,40($sp)

bne	$2,$0,$L169
lhu	$2,38($sp)

bne	$2,$0,$L170
lhu	$12,36($sp)

bne	$12,$0,$L171
lhu	$15,34($sp)

bne	$15,$0,$L8
li	$17,1			# 0x1

$L35:
li	$4,-1			# 0xffffffffffffffff
$L142:
lw	$fp,96($sp)
lw	$23,92($sp)
lw	$22,88($sp)
lw	$21,84($sp)
lw	$20,80($sp)
lw	$19,76($sp)
lw	$18,72($sp)
lw	$17,68($sp)
lw	$16,64($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,104

$L2:
lw	$3,120($sp)
lw	$20,0($3)
j	$L6
move	$3,$0

$L158:
lhu	$15,34($sp)
li	$17,15			# 0xf
$L8:
bne	$15,$0,$L173
lhu	$12,36($sp)

bne	$12,$0,$L174
lhu	$2,38($sp)

bne	$2,$0,$L175
lhu	$2,40($sp)

bne	$2,$0,$L176
lhu	$2,42($sp)

bne	$2,$0,$L177
lhu	$2,44($sp)

bne	$2,$0,$L178
lhu	$2,46($sp)

bne	$2,$0,$L179
lhu	$2,48($sp)

bne	$2,$0,$L180
lhu	$2,50($sp)

bne	$2,$0,$L181
lhu	$2,52($sp)

bne	$2,$0,$L182
lhu	$2,54($sp)

bne	$2,$0,$L183
lhu	$2,56($sp)

bne	$2,$0,$L184
lhu	$2,58($sp)

bne	$2,$0,$L185
lhu	$2,60($sp)

bne	$2,$0,$L39
li	$21,14			# 0xe

bne	$3,$0,$L187
li	$2,2			# 0x2

li	$21,16			# 0x10
sll	$2,$2,1
$L196:
subu	$2,$2,$12
bltz	$2,$L35
lhu	$13,38($sp)

sll	$2,$2,1
subu	$2,$2,$13
bltz	$2,$L35
lhu	$14,40($sp)

sll	$2,$2,1
subu	$2,$2,$14
bltz	$2,$L35
lhu	$8,42($sp)

sll	$2,$2,1
subu	$2,$2,$8
bltz	$2,$L35
lhu	$9,44($sp)

sll	$2,$2,1
subu	$2,$2,$9
bltz	$2,$L35
lhu	$10,46($sp)

sll	$2,$2,1
subu	$2,$2,$10
bltz	$2,$L35
lhu	$11,48($sp)

sll	$2,$2,1
subu	$2,$2,$11
bltz	$2,$L35
sll	$2,$2,1

lhu	$5,50($sp)
subu	$2,$2,$5
bltz	$2,$L35
lhu	$6,52($sp)

sll	$2,$2,1
subu	$2,$2,$6
bltz	$2,$L35
lhu	$7,54($sp)

sll	$2,$2,1
subu	$2,$2,$7
bltz	$2,$L35
lhu	$18,56($sp)

sll	$2,$2,1
subu	$2,$2,$18
bltz	$2,$L35
lhu	$4,58($sp)

sll	$2,$2,1
subu	$2,$2,$4
bltz	$2,$L35
lhu	$19,60($sp)

sll	$2,$2,1
subu	$2,$2,$19
bltz	$2,$L35
sll	$2,$2,1

subu	$2,$2,$3
bltz	$2,$L35
nop

bgtz	$2,$L188
lw	$3,104($sp)

andi	$15,$15,0xffff
$L193:
addu	$12,$12,$15
andi	$12,$12,0xffff
addu	$13,$13,$12
andi	$13,$13,0xffff
addu	$14,$14,$13
andi	$14,$14,0xffff
addu	$8,$8,$14
andi	$8,$8,0xffff
addu	$9,$9,$8
andi	$9,$9,0xffff
addu	$10,$10,$9
andi	$10,$10,0xffff
addu	$11,$11,$10
andi	$11,$11,0xffff
addu	$5,$5,$11
andi	$5,$5,0xffff
addu	$6,$6,$5
andi	$6,$6,0xffff
addu	$7,$7,$6
andi	$7,$7,0xffff
addu	$3,$18,$7
andi	$3,$3,0xffff
addu	$4,$4,$3
andi	$4,$4,0xffff
addu	$2,$19,$4
sh	$2,30($sp)
sh	$0,2($sp)
sh	$15,4($sp)
sh	$12,6($sp)
sh	$13,8($sp)
sh	$14,10($sp)
sh	$8,12($sp)
sh	$9,14($sp)
sh	$10,16($sp)
sh	$11,18($sp)
sh	$5,20($sp)
sh	$6,22($sp)
sh	$7,24($sp)
sh	$3,26($sp)
beq	$16,$0,$L86
sh	$4,28($sp)

lw	$4,108($sp)
move	$6,$0
$L88:
lhu	$3,0($4)
addiu	$4,$4,2
sll	$2,$3,1
beq	$3,$0,$L89
addu	$5,$2,$sp

lhu	$2,0($5)
sll	$3,$2,1
addu	$3,$3,$25
addiu	$2,$2,1
sh	$2,0($5)
sh	$6,0($3)
$L89:
addiu	$6,$6,1
bne	$16,$6,$L88
nop

$L86:
sltu	$2,$17,$20
move	$4,$17
movz	$4,$20,$2
lw	$2,104($sp)
sltu	$3,$4,$21
move	$16,$21
beq	$2,$0,$L92
movz	$16,$4,$3

move	$3,$2
li	$2,1			# 0x1
beq	$3,$2,$L189
lw	$3,116($sp)

sll	$19,$2,$16
lw	$24,0($3)
lui	$2,%hi(dbase.1476)
lui	$3,%hi(dext.1477)
addiu	$fp,$2,%lo(dbase.1476)
addiu	$23,$3,%lo(dext.1477)
addiu	$22,$19,-1
li	$20,-1			# 0xffffffffffffffff
$L94:
move	$14,$25
lhu	$8,0($14)
move	$12,$21
move	$15,$0
subu	$4,$12,$15
slt	$2,$8,$20
move	$10,$16
move	$13,$0
li	$21,-1			# 0xffffffffffffffff
li	$25,1			# 0x1
beq	$2,$0,$L101
andi	$11,$4,0x00ff

$L191:
move	$9,$0
$L103:
sll	$5,$25,$4
sll	$2,$25,$10
subu	$4,$2,$5
srl	$3,$13,$15
addu	$3,$3,$4
sll	$3,$3,2
addu	$3,$24,$3
sll	$2,$5,2
subu	$7,$0,$2
addiu	$3,$3,2
move	$18,$10
subu	$6,$0,$5
$L106:
addu	$4,$4,$6
addu	$2,$5,$4
sh	$8,0($3)
sb	$9,-2($3)
sb	$11,-1($3)
bne	$2,$0,$L106
addu	$3,$3,$7

addiu	$2,$12,-1
sll	$4,$25,$2
and	$3,$13,$4
beq	$3,$0,$L108
nop

$L144:
srl	$4,$4,1
and	$2,$13,$4
bne	$2,$0,$L144
nop

$L108:
bne	$4,$0,$L110
addiu	$2,$4,-1

move	$13,$0
$L112:
sll	$5,$12,1
addu	$3,$5,$sp
lhu	$2,32($3)
addiu	$2,$2,-1
andi	$4,$2,0xffff
beq	$4,$0,$L113
move	$7,$12

$L115:
addu	$2,$5,$sp
sltu	$3,$16,$7
beq	$3,$0,$L100
sh	$4,32($2)

and	$8,$13,$22
beq	$21,$8,$L100
nop

movz	$15,$16,$15
subu	$10,$7,$15
addu	$5,$15,$10
sltu	$2,$5,$17
beq	$2,$0,$L122
sll	$2,$5,1

addu	$2,$2,$sp
lhu	$4,32($2)
sll	$3,$25,$10
subu	$4,$3,$4
blez	$4,$L122
addiu	$2,$sp,32

addiu	$6,$5,1
sll	$3,$6,1
j	$L125
addu	$5,$2,$3

$L126:
lhu	$2,0($5)
subu	$4,$3,$2
blez	$4,$L122
addiu	$5,$5,2

$L125:
sltu	$2,$6,$17
sll	$3,$4,1
addiu	$6,$6,1
bne	$2,$0,$L126
addiu	$10,$10,1

$L122:
sll	$2,$25,$10
addu	$19,$19,$2
lw	$2,104($sp)
beq	$2,$25,$L190
sltu	$2,$19,1286

lw	$2,116($sp)
$L194:
sll	$5,$8,2
lw	$3,0($2)
move	$21,$8
addu	$3,$3,$5
sb	$10,0($3)
lw	$3,116($sp)
lw	$2,0($3)
li	$3,4			# 0x4
addu	$2,$5,$2
sb	$16,1($2)
lw	$2,116($sp)
sll	$3,$3,$18
lw	$4,0($2)
addu	$24,$24,$3
subu	$2,$24,$4
addu	$5,$5,$4
sra	$2,$2,2
sh	$2,2($5)
$L100:
addiu	$14,$14,2
lhu	$8,0($14)
move	$12,$7
subu	$4,$12,$15
slt	$2,$8,$20
bne	$2,$0,$L191
andi	$11,$4,0x00ff

$L101:
slt	$2,$20,$8
bne	$2,$0,$L104
sll	$2,$8,1

move	$8,$0
j	$L103
li	$9,96			# 0x60

$L113:
beq	$12,$17,$L116
lw	$3,108($sp)

lhu	$2,2($14)
sll	$2,$2,1
addu	$2,$2,$3
j	$L115
lhu	$7,0($2)

$L110:
and	$2,$13,$2
j	$L112
addu	$13,$2,$4

$L104:
addu	$3,$2,$fp
addu	$2,$2,$23
lbu	$9,0($2)
j	$L103
lhu	$8,0($3)

$L190:
bne	$2,$0,$L194
lw	$2,116($sp)

j	$L142
li	$4,1			# 0x1

$L116:
beq	$13,$0,$L195
lw	$3,116($sp)

move	$5,$17
andi	$8,$16,0x00ff
li	$6,64
li	$7,1			# 0x1
$L133:
beq	$15,$0,$L136
move	$2,$0

and	$2,$22,$13
beq	$21,$2,$L192
lw	$3,116($sp)

move	$11,$8
lw	$24,0($3)
move	$5,$16
move	$15,$0
move	$2,$0
$L136:
addiu	$3,$5,-1
srl	$2,$13,$2
sll	$2,$2,2
sll	$4,$7,$3
addu	$2,$24,$2
and	$3,$4,$13
sb	$11,1($2)
sh	$0,2($2)
beq	$3,$0,$L139
sb	$6,0($2)

$L145:
srl	$4,$4,1
and	$2,$4,$13
bne	$2,$0,$L145
nop

$L139:
beq	$4,$0,$L195
lw	$3,116($sp)

addiu	$2,$4,-1
and	$2,$2,$13
addu	$13,$2,$4
bne	$13,$0,$L133
nop

$L195:
move	$4,$0
lw	$2,0($3)
sll	$3,$19,2
addu	$2,$2,$3
lw	$3,116($sp)
sw	$2,0($3)
lw	$2,120($sp)
sw	$16,0($2)
lw	$fp,96($sp)
lw	$23,92($sp)
lw	$22,88($sp)
lw	$21,84($sp)
lw	$20,80($sp)
lw	$19,76($sp)
lw	$18,72($sp)
lw	$17,68($sp)
lw	$16,64($sp)
move	$2,$4
j	$31
addiu	$sp,$sp,104

$L92:
li	$2,1			# 0x1
sll	$19,$2,$16
lw	$2,116($sp)
move	$23,$25
lw	$24,0($2)
addiu	$22,$19,-1
move	$fp,$25
j	$L94
li	$20,19			# 0x13

$L159:
lhu	$15,34($sp)
j	$L8
li	$17,14			# 0xe

$L171:
j	$L8
li	$17,2			# 0x2

$L162:
lhu	$15,34($sp)
j	$L8
li	$17,11			# 0xb

$L161:
lhu	$15,34($sp)
j	$L8
li	$17,12			# 0xc

$L160:
lhu	$15,34($sp)
j	$L8
li	$17,13			# 0xd

$L192:
j	$L136
move	$2,$15

$L164:
lhu	$15,34($sp)
j	$L8
li	$17,9			# 0x9

$L163:
lhu	$15,34($sp)
j	$L8
li	$17,10			# 0xa

$L188:
beq	$3,$0,$L35
addiu	$2,$16,-1

lhu	$3,32($sp)
bne	$3,$2,$L35
nop

j	$L193
andi	$15,$15,0xffff

$L166:
lhu	$15,34($sp)
j	$L8
li	$17,7			# 0x7

$L165:
lhu	$15,34($sp)
j	$L8
li	$17,8			# 0x8

$L189:
lw	$2,104($sp)
lui	$3,%hi(lbase.1474)
addiu	$3,$3,%lo(lbase.1474)
sll	$19,$2,$16
addiu	$fp,$3,-514
lui	$4,%hi(lext.1475)
lw	$3,116($sp)
addiu	$4,$4,%lo(lext.1475)
sltu	$2,$19,1286
addiu	$23,$4,-514
lw	$24,0($3)
addiu	$22,$19,-1
bne	$2,$0,$L94
li	$20,256			# 0x100

j	$L142
li	$4,1			# 0x1

$L168:
lhu	$15,34($sp)
j	$L8
li	$17,5			# 0x5

$L167:
lhu	$15,34($sp)
j	$L8
li	$17,6			# 0x6

$L169:
lhu	$15,34($sp)
j	$L8
li	$17,4			# 0x4

$L173:
li	$21,1			# 0x1
$L39:
li	$2,2			# 0x2
subu	$2,$2,$15
bltz	$2,$L35
lhu	$12,36($sp)

j	$L196
sll	$2,$2,1

$L170:
lhu	$15,34($sp)
j	$L8
li	$17,3			# 0x3

$L175:
j	$L39
li	$21,3			# 0x3

$L174:
j	$L39
li	$21,2			# 0x2

$L177:
j	$L39
li	$21,5			# 0x5

$L176:
j	$L39
li	$21,4			# 0x4

$L179:
j	$L39
li	$21,7			# 0x7

$L178:
j	$L39
li	$21,6			# 0x6

$L184:
j	$L39
li	$21,12			# 0xc

$L183:
j	$L39
li	$21,11			# 0xb

$L182:
j	$L39
li	$21,10			# 0xa

$L181:
j	$L39
li	$21,9			# 0x9

$L180:
j	$L39
li	$21,8			# 0x8

$L185:
j	$L39
li	$21,13			# 0xd

$L187:
j	$L39
li	$21,15			# 0xf

.set	macro
.set	reorder
.end	inflate_table
.size	inflate_table, .-inflate_table
.globl	inflate_copyright
.rdata
.align	2
.type	inflate_copyright, @object
.size	inflate_copyright, 47
inflate_copyright:
.ascii	" inflate 1.2.1 Copyright 1995-2003 Mark Adler \000"
.align	2
.type	dext.1477, @object
.size	dext.1477, 64
dext.1477:
.half	16
.half	16
.half	16
.half	16
.half	17
.half	17
.half	18
.half	18
.half	19
.half	19
.half	20
.half	20
.half	21
.half	21
.half	22
.half	22
.half	23
.half	23
.half	24
.half	24
.half	25
.half	25
.half	26
.half	26
.half	27
.half	27
.half	28
.half	28
.half	29
.half	29
.half	64
.half	64
.align	2
.type	dbase.1476, @object
.size	dbase.1476, 64
dbase.1476:
.half	1
.half	2
.half	3
.half	4
.half	5
.half	7
.half	9
.half	13
.half	17
.half	25
.half	33
.half	49
.half	65
.half	97
.half	129
.half	193
.half	257
.half	385
.half	513
.half	769
.half	1025
.half	1537
.half	2049
.half	3073
.half	4097
.half	6145
.half	8193
.half	12289
.half	16385
.half	24577
.half	0
.half	0
.align	2
.type	lext.1475, @object
.size	lext.1475, 62
lext.1475:
.half	16
.half	16
.half	16
.half	16
.half	16
.half	16
.half	16
.half	16
.half	17
.half	17
.half	17
.half	17
.half	18
.half	18
.half	18
.half	18
.half	19
.half	19
.half	19
.half	19
.half	20
.half	20
.half	20
.half	20
.half	21
.half	21
.half	21
.half	21
.half	16
.half	76
.half	66
.align	2
.type	lbase.1474, @object
.size	lbase.1474, 62
lbase.1474:
.half	3
.half	4
.half	5
.half	6
.half	7
.half	8
.half	9
.half	10
.half	11
.half	13
.half	15
.half	17
.half	19
.half	23
.half	27
.half	31
.half	35
.half	43
.half	51
.half	59
.half	67
.half	83
.half	99
.half	115
.half	131
.half	163
.half	195
.half	227
.half	258
.half	0
.half	0
.ident	"GCC: (GNU) 4.1.2"
