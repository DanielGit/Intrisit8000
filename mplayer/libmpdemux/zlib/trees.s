.file	1 "trees.c"
.section .mdebug.abi32
.previous
.section	.text._tr_init,"ax",@progbits
.align	2
.align	5
.globl	_tr_init
.ent	_tr_init
_tr_init:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

lui	$2,%hi(static_l_desc)
move	$6,$4
addiu	$2,$2,%lo(static_l_desc)
sw	$2,2840($6)
lui	$2,%hi(static_d_desc)
addiu	$2,$2,%lo(static_d_desc)
addiu	$3,$4,140
sw	$2,2852($6)
lui	$2,%hi(static_bl_desc)
addiu	$4,$4,2432
addiu	$5,$6,2676
sw	$3,2832($6)
addiu	$2,$2,%lo(static_bl_desc)
li	$3,8			# 0x8
sw	$4,2844($6)
sw	$3,5804($6)
sw	$5,2856($6)
sw	$2,2864($6)
sh	$0,5808($6)
sw	$0,5812($6)
move	$3,$0
li	$4,1144			# 0x478
$L2:
addu	$2,$6,$3
addiu	$3,$3,4
bne	$3,$4,$L2
sh	$0,140($2)

move	$3,$0
li	$4,120			# 0x78
$L4:
addu	$2,$6,$3
addiu	$3,$3,4
bne	$3,$4,$L4
sh	$0,2432($2)

move	$3,$0
li	$4,76			# 0x4c
$L6:
addu	$2,$6,$3
addiu	$3,$3,4
bne	$3,$4,$L6
sh	$0,2676($2)

li	$2,1
sh	$2,1164($6)
sw	$0,5784($6)
sw	$0,5796($6)
sw	$0,5792($6)
j	$31
sw	$0,5800($6)

.set	macro
.set	reorder
.end	_tr_init
.size	_tr_init, .-_tr_init
.section	.text.send_tree,"ax",@progbits
.align	2
.align	5
.ent	send_tree
send_tree:
.frame	$sp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
.mask	0x00010000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-8
sw	$16,0($sp)
lhu	$2,2($5)
beq	$2,$0,$L17
move	$10,$4

li	$4,7			# 0x7
li	$3,4			# 0x4
$L19:
bltz	$6,$L61
move	$13,$2

move	$15,$5
addiu	$25,$6,1
move	$24,$0
li	$5,-1			# 0xffffffffffffffff
move	$11,$0
li	$16,16			# 0x10
$L22:
addiu	$11,$11,1
slt	$2,$11,$4
beq	$2,$0,$L23
lhu	$14,6($15)

beq	$14,$13,$L25
nop

$L23:
slt	$2,$11,$3
beq	$2,$0,$L26
sll	$2,$13,2

addu	$9,$2,$10
j	$L27
li	$12,16			# 0x10

$L64:
lhu	$6,2676($9)
lhu	$3,5808($10)
lw	$5,20($10)
lw	$4,8($10)
sll	$2,$6,$7
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($10)
sb	$2,0($4)
lhu	$3,5808($10)
lw	$2,8($10)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($10)
addiu	$5,$5,2
subu	$2,$12,$4
addu	$3,$8,$4
sra	$6,$6,$2
addiu	$4,$3,-16
addiu	$11,$11,-1
sw	$5,20($10)
sh	$6,5808($10)
beq	$11,$0,$L31
sw	$4,5812($10)

$L27:
lhu	$8,2678($9)
lw	$7,5812($10)
subu	$2,$12,$8
slt	$2,$2,$7
bne	$2,$0,$L64
addu	$4,$7,$8

lhu	$2,2676($9)
lhu	$3,5808($10)
sll	$2,$2,$7
or	$2,$2,$3
addiu	$11,$11,-1
sw	$4,5812($10)
bne	$11,$0,$L27
sh	$2,5808($10)

$L31:
bne	$14,$0,$L56
nop

$L65:
move	$5,$13
move	$11,$0
li	$4,138			# 0x8a
li	$3,3			# 0x3
$L25:
addiu	$24,$24,1
beq	$24,$25,$L61
addiu	$15,$15,4

$L67:
j	$L22
move	$13,$14

$L26:
beq	$13,$0,$L32
slt	$2,$11,11

beq	$5,$13,$L62
sll	$2,$13,2

addu	$3,$2,$10
lhu	$7,2678($3)
lw	$4,5812($10)
li	$8,16			# 0x10
subu	$2,$8,$7
slt	$2,$2,$4
beq	$2,$0,$L36
nop

lhu	$6,2676($3)
lhu	$3,5808($10)
lw	$5,20($10)
sll	$2,$6,$4
lw	$4,8($10)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($10)
sb	$2,0($4)
lhu	$3,5808($10)
lw	$2,8($10)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($10)
addiu	$5,$5,2
subu	$2,$8,$4
addu	$3,$7,$4
sra	$6,$6,$2
addiu	$4,$3,-16
sw	$5,20($10)
sh	$6,5808($10)
sw	$4,5812($10)
$L38:
addiu	$11,$11,-1
$L34:
lhu	$7,2742($10)
li	$8,16			# 0x10
subu	$2,$8,$7
slt	$2,$2,$4
beq	$2,$0,$L39
nop

lhu	$6,2740($10)
lhu	$3,5808($10)
lw	$5,20($10)
sll	$2,$6,$4
lw	$4,8($10)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($10)
sb	$2,0($4)
lhu	$3,5808($10)
lw	$2,8($10)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($10)
addiu	$5,$5,2
subu	$2,$8,$4
addu	$3,$7,$4
sra	$6,$6,$2
addiu	$4,$3,-16
sw	$5,20($10)
sh	$6,5808($10)
sw	$4,5812($10)
$L41:
slt	$2,$4,15
bne	$2,$0,$L42
addiu	$2,$11,-3

lhu	$3,5808($10)
addiu	$6,$11,-3
lw	$5,20($10)
sll	$2,$6,$4
lw	$4,8($10)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($10)
sb	$2,0($4)
lhu	$3,5808($10)
lw	$2,8($10)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($10)
andi	$6,$6,0xffff
subu	$2,$16,$4
sra	$6,$6,$2
addiu	$5,$5,2
addiu	$4,$4,-14
sw	$5,20($10)
sh	$6,5808($10)
beq	$14,$0,$L65
sw	$4,5812($10)

$L56:
beq	$14,$13,$L66
move	$5,$14

addiu	$24,$24,1
move	$5,$13
move	$11,$0
li	$4,7			# 0x7
li	$3,4			# 0x4
bne	$24,$25,$L67
addiu	$15,$15,4

$L61:
lw	$16,0($sp)
j	$31
addiu	$sp,$sp,8

$L32:
beq	$2,$0,$L44
li	$8,16			# 0x10

lhu	$7,2746($10)
lw	$4,5812($10)
subu	$2,$8,$7
slt	$2,$2,$4
beq	$2,$0,$L46
nop

lhu	$6,2744($10)
lhu	$3,5808($10)
lw	$5,20($10)
sll	$2,$6,$4
lw	$4,8($10)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($10)
sb	$2,0($4)
lhu	$3,5808($10)
lw	$2,8($10)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($10)
addiu	$5,$5,2
addu	$3,$7,$4
subu	$2,$8,$4
addiu	$4,$3,-16
sra	$6,$6,$2
slt	$2,$4,14
sw	$5,20($10)
sh	$6,5808($10)
bne	$2,$0,$L49
sw	$4,5812($10)

$L69:
lhu	$3,5808($10)
addiu	$6,$11,-3
lw	$5,20($10)
sll	$2,$6,$4
lw	$4,8($10)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($10)
sb	$2,0($4)
lhu	$3,5808($10)
lw	$2,8($10)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($10)
andi	$6,$6,0xffff
subu	$2,$16,$4
sra	$6,$6,$2
addiu	$5,$5,2
addiu	$4,$4,-13
sw	$5,20($10)
sh	$6,5808($10)
j	$L31
sw	$4,5812($10)

$L66:
move	$11,$0
li	$4,6			# 0x6
j	$L25
li	$3,3			# 0x3

$L44:
lhu	$7,2750($10)
lw	$4,5812($10)
subu	$2,$8,$7
slt	$2,$2,$4
beq	$2,$0,$L51
nop

lhu	$6,2748($10)
lhu	$3,5808($10)
lw	$5,20($10)
sll	$2,$6,$4
lw	$4,8($10)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($10)
sb	$2,0($4)
lhu	$3,5808($10)
lw	$2,8($10)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($10)
addiu	$5,$5,2
addu	$3,$7,$4
subu	$2,$8,$4
addiu	$4,$3,-16
sra	$6,$6,$2
slt	$2,$4,10
sw	$5,20($10)
sh	$6,5808($10)
bne	$2,$0,$L54
sw	$4,5812($10)

$L68:
lhu	$3,5808($10)
addiu	$6,$11,-11
lw	$5,20($10)
sll	$2,$6,$4
lw	$4,8($10)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($10)
sb	$2,0($4)
lhu	$3,5808($10)
lw	$2,8($10)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($10)
andi	$6,$6,0xffff
subu	$2,$16,$4
sra	$6,$6,$2
addiu	$5,$5,2
addiu	$4,$4,-9
sw	$5,20($10)
sh	$6,5808($10)
j	$L31
sw	$4,5812($10)

$L42:
lhu	$3,5808($10)
sll	$2,$2,$4
or	$2,$2,$3
addiu	$4,$4,2
sh	$2,5808($10)
j	$L31
sw	$4,5812($10)

$L39:
lhu	$2,2740($10)
lhu	$3,5808($10)
sll	$2,$2,$4
or	$2,$2,$3
addu	$4,$4,$7
sh	$2,5808($10)
j	$L41
sw	$4,5812($10)

$L36:
lhu	$2,2676($3)
lhu	$3,5808($10)
sll	$2,$2,$4
or	$2,$2,$3
addu	$4,$4,$7
sh	$2,5808($10)
j	$L38
sw	$4,5812($10)

$L62:
j	$L34
lw	$4,5812($10)

$L17:
li	$4,138			# 0x8a
j	$L19
li	$3,3			# 0x3

$L51:
lhu	$2,2748($10)
lhu	$3,5808($10)
sll	$2,$2,$4
or	$2,$2,$3
addu	$4,$4,$7
sh	$2,5808($10)
slt	$2,$4,10
beq	$2,$0,$L68
sw	$4,5812($10)

$L54:
lhu	$3,5808($10)
addiu	$2,$11,-11
sll	$2,$2,$4
or	$2,$2,$3
addiu	$4,$4,7
sh	$2,5808($10)
j	$L31
sw	$4,5812($10)

$L46:
lhu	$2,2744($10)
lhu	$3,5808($10)
sll	$2,$2,$4
or	$2,$2,$3
addu	$4,$4,$7
sh	$2,5808($10)
slt	$2,$4,14
beq	$2,$0,$L69
sw	$4,5812($10)

$L49:
lhu	$3,5808($10)
addiu	$2,$11,-3
sll	$2,$2,$4
or	$2,$2,$3
addiu	$4,$4,3
sh	$2,5808($10)
j	$L31
sw	$4,5812($10)

.set	macro
.set	reorder
.end	send_tree
.size	send_tree, .-send_tree
.section	.text._tr_tally,"ax",@progbits
.align	2
.align	5
.globl	_tr_tally
.ent	_tr_tally
_tr_tally:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

move	$9,$4
lw	$4,5784($4)
lw	$2,5788($9)
lw	$8,5776($9)
sll	$7,$4,1
addu	$7,$7,$2
lui	$2,%hi(_length_code)
addu	$8,$8,$4
addiu	$2,$2,%lo(_length_code)
sll	$3,$6,2
addiu	$10,$5,-1
addiu	$4,$4,1
sh	$5,0($7)
addu	$2,$6,$2
sb	$6,0($8)
addu	$3,$3,$9
sltu	$11,$10,256
bne	$5,$0,$L71
sw	$4,5784($9)

lhu	$2,140($3)
addiu	$2,$2,1
sh	$2,140($3)
lw	$3,5780($9)
lw	$2,5784($9)
addiu	$3,$3,-1
xor	$2,$2,$3
j	$31
sltu	$2,$2,1

$L71:
lbu	$3,0($2)
lw	$2,5800($9)
addiu	$3,$3,257
addiu	$2,$2,1
sll	$3,$3,2
sw	$2,5800($9)
addu	$3,$3,$9
lui	$2,%hi(_dist_code)
addiu	$2,$2,%lo(_dist_code)
lhu	$4,140($3)
addu	$6,$10,$2
lui	$2,%hi(_dist_code)
srl	$5,$10,7
addiu	$2,$2,%lo(_dist_code)
addiu	$4,$4,1
addu	$5,$5,$2
beq	$11,$0,$L74
sh	$4,140($3)

lbu	$3,0($6)
$L76:
sll	$3,$3,2
addu	$3,$3,$9
lhu	$2,2432($3)
addiu	$2,$2,1
sh	$2,2432($3)
lw	$3,5780($9)
lw	$2,5784($9)
addiu	$3,$3,-1
xor	$2,$2,$3
j	$31
sltu	$2,$2,1

$L74:
j	$L76
lbu	$3,256($5)

.set	macro
.set	reorder
.end	_tr_tally
.size	_tr_tally, .-_tr_tally
.section	.text.compress_block,"ax",@progbits
.align	2
.align	5
.ent	compress_block
compress_block:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

lw	$10,5784($4)
move	$8,$4
move	$15,$5
beq	$10,$0,$L107
move	$24,$6

lui	$2,%hi(_dist_code)
lw	$7,5812($4)
addiu	$25,$2,%lo(_dist_code)
j	$L81
move	$14,$0

$L110:
sll	$2,$11,2
addu	$3,$15,$2
lhu	$9,2($3)
li	$11,16			# 0x10
subu	$2,$11,$9
slt	$2,$2,$7
beq	$2,$0,$L84
nop

lhu	$6,0($3)
lhu	$3,5808($8)
lw	$5,20($8)
sll	$2,$6,$7
lw	$4,8($8)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$7,5812($8)
addiu	$5,$5,2
subu	$2,$11,$7
addu	$3,$9,$7
sra	$6,$6,$2
addiu	$7,$3,-16
lw	$10,5784($8)
sw	$5,20($8)
sh	$6,5808($8)
sw	$7,5812($8)
$L86:
sltu	$2,$14,$10
beq	$2,$0,$L79
nop

$L81:
lw	$2,5788($8)
sll	$3,$14,1
addu	$3,$3,$2
lw	$4,5776($8)
lhu	$12,0($3)
addu	$4,$4,$14
lbu	$11,0($4)
beq	$12,$0,$L110
addiu	$14,$14,1

lui	$2,%hi(_length_code)
addiu	$2,$2,%lo(_length_code)
addu	$2,$11,$2
lbu	$3,0($2)
li	$13,16			# 0x10
sll	$10,$3,2
addu	$3,$15,$10
lhu	$9,1030($3)
subu	$2,$13,$9
slt	$2,$2,$7
beq	$2,$0,$L87
nop

lhu	$6,1028($3)
lhu	$3,5808($8)
lw	$5,20($8)
sll	$2,$6,$7
lw	$4,8($8)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$7,5812($8)
addiu	$5,$5,2
subu	$2,$13,$7
addu	$3,$9,$7
sra	$6,$6,$2
addiu	$7,$3,-16
sw	$5,20($8)
sh	$6,5808($8)
sw	$7,5812($8)
$L89:
lui	$2,%hi(extra_lbits)
addiu	$2,$2,%lo(extra_lbits)
addu	$2,$10,$2
lw	$6,0($2)
beq	$6,$0,$L90
lui	$2,%hi(base_length)

addiu	$2,$2,%lo(base_length)
addu	$2,$10,$2
li	$10,16			# 0x10
lw	$4,0($2)
subu	$3,$10,$6
slt	$3,$3,$7
beq	$3,$0,$L92
subu	$9,$11,$4

lhu	$3,5808($8)
lw	$5,20($8)
lw	$4,8($8)
sll	$2,$9,$7
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$7,5812($8)
andi	$3,$9,0xffff
subu	$2,$10,$7
addu	$4,$6,$7
sra	$3,$3,$2
addiu	$5,$5,2
addiu	$7,$4,-16
sw	$5,20($8)
sh	$3,5808($8)
sw	$7,5812($8)
$L90:
addiu	$11,$12,-1
sltu	$2,$11,256
beq	$2,$0,$L94
srl	$2,$11,7

addu	$2,$11,$25
lbu	$2,0($2)
$L96:
sll	$10,$2,2
addu	$3,$10,$24
lhu	$9,2($3)
li	$12,16			# 0x10
subu	$2,$12,$9
slt	$2,$2,$7
beq	$2,$0,$L97
nop

lhu	$6,0($3)
lhu	$3,5808($8)
lw	$5,20($8)
sll	$2,$6,$7
lw	$4,8($8)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$7,5812($8)
addiu	$5,$5,2
subu	$2,$12,$7
addu	$3,$9,$7
sra	$6,$6,$2
addiu	$7,$3,-16
sw	$5,20($8)
sh	$6,5808($8)
sw	$7,5812($8)
$L99:
lui	$2,%hi(extra_dbits)
addiu	$2,$2,%lo(extra_dbits)
addu	$2,$10,$2
lw	$6,0($2)
beq	$6,$0,$L108
lui	$2,%hi(base_dist)

addiu	$2,$2,%lo(base_dist)
addu	$2,$10,$2
li	$10,16			# 0x10
lw	$4,0($2)
subu	$3,$10,$6
slt	$3,$3,$7
beq	$3,$0,$L101
subu	$9,$11,$4

lhu	$3,5808($8)
lw	$5,20($8)
lw	$4,8($8)
sll	$2,$9,$7
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$7,5812($8)
andi	$2,$9,0xffff
subu	$3,$10,$7
lw	$10,5784($8)
sra	$2,$2,$3
addu	$4,$6,$7
addiu	$5,$5,2
addiu	$7,$4,-16
sh	$2,5808($8)
sltu	$2,$14,$10
sw	$5,20($8)
bne	$2,$0,$L81
sw	$7,5812($8)

$L79:
addiu	$10,$15,1024
lhu	$9,2($10)
li	$11,16			# 0x10
subu	$2,$11,$9
slt	$2,$2,$7
beq	$2,$0,$L103
addu	$4,$7,$9

lhu	$6,1024($15)
lhu	$4,5808($8)
lw	$5,20($8)
sll	$2,$6,$7
lw	$3,8($8)
or	$2,$2,$4
andi	$2,$2,0xffff
addu	$3,$3,$5
sh	$2,5808($8)
sb	$2,0($3)
lhu	$4,5808($8)
lw	$2,8($8)
srl	$4,$4,8
addu	$2,$5,$2
sb	$4,1($2)
lw	$3,5812($8)
addiu	$5,$5,2
addu	$2,$9,$3
subu	$3,$11,$3
addiu	$7,$2,-16
sra	$6,$6,$3
sw	$5,20($8)
sh	$6,5808($8)
sw	$7,5812($8)
lhu	$2,2($10)
j	$31
sw	$2,5804($8)

$L87:
lhu	$2,1028($3)
lhu	$3,5808($8)
sll	$2,$2,$7
or	$2,$2,$3
addu	$7,$7,$9
sh	$2,5808($8)
j	$L89
sw	$7,5812($8)

$L108:
j	$L86
lw	$10,5784($8)

$L97:
lhu	$2,0($3)
lhu	$3,5808($8)
sll	$2,$2,$7
or	$2,$2,$3
addu	$7,$7,$9
sh	$2,5808($8)
j	$L99
sw	$7,5812($8)

$L94:
addu	$2,$2,$25
j	$L96
lbu	$2,256($2)

$L84:
lhu	$2,0($3)
lhu	$3,5808($8)
sll	$2,$2,$7
or	$2,$2,$3
addu	$7,$7,$9
sh	$2,5808($8)
j	$L86
sw	$7,5812($8)

$L101:
lhu	$3,5808($8)
sll	$2,$9,$7
lw	$10,5784($8)
or	$2,$2,$3
addu	$7,$7,$6
sh	$2,5808($8)
j	$L86
sw	$7,5812($8)

$L92:
lhu	$3,5808($8)
sll	$2,$9,$7
or	$2,$2,$3
addu	$7,$7,$6
sh	$2,5808($8)
j	$L90
sw	$7,5812($8)

$L103:
lhu	$2,1024($15)
lhu	$3,5808($8)
sll	$2,$2,$7
or	$2,$2,$3
sh	$2,5808($8)
sw	$4,5812($8)
lhu	$2,2($10)
j	$31
sw	$2,5804($8)

$L107:
j	$L79
lw	$7,5812($4)

.set	macro
.set	reorder
.end	compress_block
.size	compress_block, .-compress_block
.section	.text._tr_stored_block,"ax",@progbits
.align	2
.align	5
.globl	_tr_stored_block
.ent	_tr_stored_block
_tr_stored_block:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

move	$8,$4
lw	$4,5812($4)
move	$10,$5
slt	$2,$4,14
bne	$2,$0,$L112
move	$9,$6

lhu	$3,5808($8)
lw	$5,20($8)
sll	$2,$7,$4
lw	$4,8($8)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($8)
li	$2,16			# 0x10
subu	$2,$2,$4
andi	$3,$7,0xffff
sra	$3,$3,$2
addiu	$5,$5,2
addiu	$4,$4,-13
sw	$5,20($8)
sh	$3,5808($8)
sw	$4,5812($8)
$L114:
slt	$2,$4,9
bne	$2,$0,$L115
nop

lw	$4,20($8)
lw	$2,8($8)
lhu	$5,5808($8)
addu	$2,$2,$4
sb	$5,0($2)
lw	$3,8($8)
lhu	$2,5808($8)
addu	$3,$4,$3
srl	$2,$2,8
addiu	$4,$4,2
sb	$2,1($3)
sw	$4,20($8)
$L117:
lw	$6,20($8)
lw	$4,8($8)
li	$2,8			# 0x8
addu	$4,$4,$6
sw	$2,5804($8)
sh	$0,5808($8)
sw	$0,5812($8)
sb	$9,0($4)
lw	$3,8($8)
andi	$5,$9,0xffff
addu	$3,$6,$3
srl	$4,$5,8
sb	$4,1($3)
lw	$2,8($8)
nor	$4,$0,$9
addu	$2,$6,$2
sb	$4,2($2)
lw	$3,8($8)
xori	$5,$5,0xff00
addu	$3,$6,$3
srl	$5,$5,8
addiu	$6,$6,4
sb	$5,3($3)
beq	$9,$0,$L124
sw	$6,20($8)

lw	$2,20($8)
move	$5,$10
addu	$6,$10,$9
$L121:
lw	$3,8($8)
lbu	$4,0($5)
addu	$3,$3,$2
addiu	$5,$5,1
addiu	$2,$2,1
sb	$4,0($3)
bne	$5,$6,$L121
sw	$2,20($8)

$L124:
j	$31
nop

$L115:
blez	$4,$L117
nop

lw	$2,20($8)
lw	$3,8($8)
lhu	$4,5808($8)
addu	$3,$3,$2
addiu	$2,$2,1
sb	$4,0($3)
j	$L117
sw	$2,20($8)

$L112:
lhu	$3,5808($8)
sll	$2,$7,$4
or	$2,$2,$3
addiu	$4,$4,3
sh	$2,5808($8)
j	$L114
sw	$4,5812($8)

.set	macro
.set	reorder
.end	_tr_stored_block
.size	_tr_stored_block, .-_tr_stored_block
.section	.text._tr_align,"ax",@progbits
.align	2
.align	5
.globl	_tr_align
.ent	_tr_align
_tr_align:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

move	$8,$4
lw	$4,5812($4)
slt	$2,$4,14
bne	$2,$0,$L126
li	$2,2			# 0x2

lhu	$3,5808($8)
li	$6,2			# 0x2
lw	$5,20($8)
sll	$2,$6,$4
lw	$4,8($8)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$7,5812($8)
li	$2,16			# 0x10
subu	$2,$2,$7
sra	$6,$6,$2
addiu	$5,$5,2
addiu	$7,$7,-13
sw	$5,20($8)
sh	$6,5808($8)
sw	$7,5812($8)
$L128:
lui	$12,%hi(static_ltree)
addiu	$3,$12,%lo(static_ltree)
lhu	$10,1026($3)
li	$9,16			# 0x10
subu	$11,$9,$10
slt	$2,$11,$7
beq	$2,$0,$L129
nop

lhu	$6,1024($3)
lhu	$3,5808($8)
lw	$5,20($8)
sll	$2,$6,$7
lw	$4,8($8)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$7,5812($8)
addiu	$5,$5,2
subu	$2,$9,$7
addu	$3,$10,$7
sra	$6,$6,$2
addiu	$7,$3,-16
li	$2,16			# 0x10
sw	$5,20($8)
sh	$6,5808($8)
beq	$7,$2,$L148
sw	$7,5812($8)

$L132:
slt	$2,$7,8
beq	$2,$0,$L149
nop

$L134:
lw	$2,5804($8)
subu	$2,$2,$7
slt	$2,$2,-2
beq	$2,$0,$L136
slt	$2,$7,14

bne	$2,$0,$L138
li	$2,2			# 0x2

lhu	$3,5808($8)
li	$6,2			# 0x2
lw	$5,20($8)
sll	$2,$6,$7
lw	$4,8($8)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$7,5812($8)
li	$2,16			# 0x10
subu	$2,$2,$7
addiu	$7,$7,-13
sra	$6,$6,$2
addiu	$5,$5,2
slt	$2,$11,$7
sw	$5,20($8)
sh	$6,5808($8)
beq	$2,$0,$L141
sw	$7,5812($8)

$L151:
addiu	$2,$12,%lo(static_ltree)
lhu	$6,1024($2)
lhu	$3,5808($8)
lw	$5,20($8)
sll	$2,$6,$7
lw	$4,8($8)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($8)
sb	$2,0($4)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$7,5812($8)
li	$2,16			# 0x10
subu	$2,$2,$7
addu	$3,$10,$7
sra	$6,$6,$2
addiu	$5,$5,2
addiu	$7,$3,-16
li	$2,16			# 0x10
sw	$5,20($8)
sh	$6,5808($8)
beq	$7,$2,$L150
sw	$7,5812($8)

$L144:
slt	$2,$7,8
bne	$2,$0,$L152
li	$2,7			# 0x7

lw	$4,20($8)
lw	$2,8($8)
lhu	$5,5808($8)
addu	$2,$2,$4
sb	$5,0($2)
lhu	$3,5808($8)
lw	$2,5812($8)
addiu	$4,$4,1
srl	$3,$3,8
addiu	$2,$2,-8
sw	$4,20($8)
sh	$3,5808($8)
sw	$2,5812($8)
$L136:
li	$2,7			# 0x7
$L152:
j	$31
sw	$2,5804($8)

$L129:
lhu	$2,1024($3)
lhu	$3,5808($8)
sll	$2,$2,$7
or	$2,$2,$3
addu	$7,$7,$10
sh	$2,5808($8)
li	$2,16			# 0x10
bne	$7,$2,$L132
sw	$7,5812($8)

$L148:
lw	$4,20($8)
lw	$2,8($8)
lhu	$5,5808($8)
addu	$2,$2,$4
sb	$5,0($2)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$4,$2
move	$7,$0
addiu	$4,$4,2
sb	$3,1($2)
sw	$4,20($8)
sh	$0,5808($8)
j	$L134
sw	$0,5812($8)

$L126:
lhu	$3,5808($8)
sll	$2,$2,$4
or	$2,$2,$3
addiu	$7,$4,3
sh	$2,5808($8)
j	$L128
sw	$7,5812($8)

$L149:
lw	$4,20($8)
lw	$2,8($8)
lhu	$5,5808($8)
addu	$2,$2,$4
sb	$5,0($2)
lhu	$3,5808($8)
lw	$7,5812($8)
addiu	$4,$4,1
srl	$3,$3,8
addiu	$7,$7,-8
sw	$4,20($8)
sh	$3,5808($8)
j	$L134
sw	$7,5812($8)

$L138:
lhu	$3,5808($8)
sll	$2,$2,$7
or	$2,$2,$3
addiu	$7,$7,3
sh	$2,5808($8)
slt	$2,$11,$7
bne	$2,$0,$L151
sw	$7,5812($8)

$L141:
addiu	$2,$12,%lo(static_ltree)
lhu	$3,1024($2)
lhu	$4,5808($8)
sll	$3,$3,$7
or	$3,$3,$4
addu	$7,$7,$10
li	$2,16			# 0x10
sh	$3,5808($8)
bne	$7,$2,$L144
sw	$7,5812($8)

$L150:
lw	$4,20($8)
lw	$2,8($8)
lhu	$5,5808($8)
addu	$2,$2,$4
sb	$5,0($2)
lhu	$3,5808($8)
lw	$2,8($8)
srl	$3,$3,8
addu	$2,$4,$2
sb	$3,1($2)
addiu	$4,$4,2
li	$2,7			# 0x7
sw	$4,20($8)
sh	$0,5808($8)
sw	$0,5812($8)
j	$31
sw	$2,5804($8)

.set	macro
.set	reorder
.end	_tr_align
.size	_tr_align, .-_tr_align
.section	.text.build_tree,"ax",@progbits
.align	2
.align	5
.ent	build_tree
build_tree:
.frame	$sp,56,$31		# vars= 32, regs= 6/0, args= 0, gp= 0
.mask	0x003f0000,-4
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-56
sw	$21,52($sp)
sw	$20,48($sp)
sw	$19,44($sp)
sw	$18,40($sp)
sw	$17,36($sp)
sw	$16,32($sp)
lw	$3,8($5)
li	$2,573			# 0x23d
lw	$19,12($3)
move	$21,$5
move	$24,$4
sw	$2,5196($4)
sw	$0,5192($4)
lw	$25,0($5)
blez	$19,$L283
lw	$7,0($3)

move	$4,$25
move	$6,$0
j	$L157
li	$20,-1			# 0xffffffffffffffff

$L284:
lw	$3,5192($24)
move	$20,$6
addiu	$9,$3,1
sll	$2,$9,2
addu	$2,$2,$24
sw	$6,2900($2)
addiu	$6,$6,1
sb	$0,5200($5)
sw	$9,5192($24)
beq	$19,$6,$L156
addiu	$4,$4,4

$L157:
lhu	$2,0($4)
bne	$2,$0,$L284
addu	$5,$6,$24

addiu	$6,$6,1
sh	$0,2($4)
bne	$19,$6,$L157
addiu	$4,$4,4

$L156:
bne	$7,$0,$L280
li	$6,1

j	$L312
lw	$2,5192($24)

$L285:
move	$5,$0
$L167:
sll	$2,$9,2
addu	$3,$25,$4
addu	$2,$2,$24
sw	$5,2900($2)
sh	$6,0($3)
lw	$2,5792($24)
addu	$4,$5,$24
addiu	$2,$2,-1
sb	$0,5200($4)
sw	$2,5792($24)
lw	$2,5192($24)
$L312:
addiu	$5,$20,1
addiu	$9,$2,1
slt	$2,$2,2
slt	$3,$20,2
beq	$2,$0,$L163
move	$4,$0

beq	$3,$0,$L285
sw	$9,5192($24)

sll	$4,$5,2
j	$L167
move	$20,$5

$L163:
sw	$20,4($21)
lw	$9,5192($24)
srl	$2,$9,31
addu	$2,$2,$9
sra	$14,$2,1
blez	$14,$L172
sll	$2,$14,2

addu	$2,$2,$24
addiu	$17,$2,2900
$L173:
sll	$6,$14,1
slt	$2,$9,$6
bne	$2,$0,$L286
lw	$15,0($17)

sll	$2,$15,2
addu	$13,$25,$2
move	$10,$14
j	$L177
addu	$16,$15,$24

$L289:
addiu	$11,$6,1
sll	$12,$11,2
addu	$2,$12,$24
addu	$3,$8,$24
lw	$4,2900($2)
lw	$5,2900($3)
sll	$2,$4,2
sll	$3,$5,2
addu	$2,$2,$25
addu	$3,$3,$25
lhu	$7,0($2)
lhu	$3,0($3)
sltu	$2,$7,$3
bne	$2,$0,$L180
nop

beq	$7,$3,$L287
addu	$4,$4,$24

$L178:
addu	$2,$8,$24
$L308:
lw	$5,2900($2)
lhu	$4,0($13)
sll	$2,$5,2
addu	$2,$2,$25
lhu	$2,0($2)
sltu	$3,$4,$2
bne	$3,$0,$L176
nop

$L290:
beq	$4,$2,$L288
sll	$2,$10,2

sll	$4,$6,1
addu	$2,$2,$24
slt	$3,$9,$4
bne	$3,$0,$L186
sw	$5,2900($2)

$L301:
move	$10,$6
move	$6,$4
$L177:
slt	$2,$6,$9
bne	$2,$0,$L289
sll	$8,$6,2

addu	$2,$8,$24
lw	$5,2900($2)
lhu	$4,0($13)
sll	$2,$5,2
addu	$2,$2,$25
lhu	$2,0($2)
sltu	$3,$4,$2
beq	$3,$0,$L290
nop

$L176:
sll	$2,$10,2
addu	$2,$2,$24
addiu	$14,$14,-1
sw	$15,2900($2)
bne	$14,$0,$L173
addiu	$17,$17,-4

$L172:
addiu	$2,$19,5200
$L307:
addu	$18,$24,$2
sll	$2,$9,2
addu	$2,$2,$24
sll	$3,$19,2
addiu	$9,$9,-1
lw	$15,2900($2)
addu	$14,$25,$3
slt	$3,$9,2
lw	$16,2904($24)
sw	$9,5192($24)
bne	$3,$0,$L291
sw	$15,2904($24)

$L189:
sll	$2,$15,2
addu	$12,$25,$2
li	$13,1			# 0x1
li	$8,2			# 0x2
j	$L192
addu	$17,$15,$24

$L294:
addiu	$10,$8,1
sll	$11,$10,2
addu	$2,$11,$24
addu	$3,$7,$24
lw	$4,2900($2)
lw	$5,2900($3)
sll	$2,$4,2
sll	$3,$5,2
addu	$2,$2,$25
addu	$3,$3,$25
lhu	$6,0($2)
lhu	$3,0($3)
sltu	$2,$6,$3
bne	$2,$0,$L195
nop

beq	$6,$3,$L292
addu	$4,$4,$24

$L193:
addu	$2,$7,$24
$L309:
lw	$5,2900($2)
lhu	$4,0($12)
sll	$2,$5,2
addu	$2,$2,$25
lhu	$2,0($2)
sltu	$3,$4,$2
bne	$3,$0,$L191
nop

$L295:
beq	$4,$2,$L293
sll	$2,$13,2

sll	$4,$8,1
addu	$2,$2,$24
slt	$3,$9,$4
bne	$3,$0,$L201
sw	$5,2900($2)

$L302:
move	$13,$8
move	$8,$4
$L192:
slt	$2,$8,$9
bne	$2,$0,$L294
sll	$7,$8,2

addu	$2,$7,$24
lw	$5,2900($2)
lhu	$4,0($12)
sll	$2,$5,2
addu	$2,$2,$25
lhu	$2,0($2)
sltu	$3,$4,$2
beq	$3,$0,$L295
nop

$L191:
sll	$2,$13,2
lw	$3,5196($24)
addu	$2,$2,$24
sw	$15,2900($2)
lw	$4,2904($24)
addiu	$7,$3,-2
addiu	$3,$3,-1
sll	$2,$7,2
sll	$3,$3,2
addu	$3,$3,$24
addu	$2,$2,$24
sll	$5,$16,2
sll	$6,$4,2
sw	$16,2900($3)
sw	$7,5196($24)
sw	$4,2900($2)
addu	$7,$25,$5
addu	$6,$25,$6
lhu	$3,0($6)
lhu	$2,0($7)
addu	$5,$16,$24
addu	$2,$2,$3
sh	$2,0($14)
addu	$4,$4,$24
lbu	$5,5200($5)
lbu	$3,5200($4)
sltu	$2,$5,$3
bne	$2,$0,$L203
addiu	$2,$3,1

addiu	$2,$5,1
andi	$5,$2,0x00ff
andi	$2,$19,0xffff
sb	$5,0($18)
sh	$2,2($6)
sh	$2,2($7)
lw	$9,5192($24)
slt	$2,$9,2
bne	$2,$0,$L296
sw	$19,2904($24)

$L206:
li	$12,1			# 0x1
j	$L209
li	$8,2			# 0x2

$L299:
addiu	$10,$8,1
sll	$11,$10,2
addu	$2,$11,$24
addu	$3,$7,$24
lw	$4,2900($2)
lw	$5,2900($3)
sll	$2,$4,2
sll	$3,$5,2
addu	$2,$2,$25
addu	$3,$3,$25
lhu	$6,0($2)
lhu	$3,0($3)
sltu	$2,$6,$3
bne	$2,$0,$L212
nop

beq	$6,$3,$L297
addu	$4,$4,$24

$L210:
addu	$2,$7,$24
$L310:
lw	$5,2900($2)
lhu	$4,0($14)
sll	$2,$5,2
addu	$2,$2,$25
lhu	$2,0($2)
sltu	$3,$4,$2
bne	$3,$0,$L208
nop

$L300:
beq	$4,$2,$L298
sll	$2,$12,2

sll	$4,$8,1
addu	$2,$2,$24
slt	$3,$9,$4
bne	$3,$0,$L218
sw	$5,2900($2)

$L303:
move	$12,$8
move	$8,$4
$L209:
slt	$2,$8,$9
bne	$2,$0,$L299
sll	$7,$8,2

addu	$2,$7,$24
lw	$5,2900($2)
lhu	$4,0($14)
sll	$2,$5,2
addu	$2,$2,$25
lhu	$2,0($2)
sltu	$3,$4,$2
beq	$3,$0,$L300
nop

$L208:
sll	$2,$12,2
addu	$2,$2,$24
slt	$3,$9,2
sw	$19,2900($2)
addiu	$18,$18,1
bne	$3,$0,$L220
addiu	$14,$14,4

$L304:
sll	$2,$9,2
addu	$2,$2,$24
addiu	$9,$9,-1
lw	$15,2900($2)
slt	$3,$9,2
lw	$16,2904($24)
addiu	$19,$19,1
sw	$9,5192($24)
beq	$3,$0,$L189
sw	$15,2904($24)

$L291:
j	$L191
li	$13,1			# 0x1

$L287:
addu	$5,$5,$24
lbu	$2,5200($4)
lbu	$3,5200($5)
sltu	$2,$3,$2
bne	$2,$0,$L308
addu	$2,$8,$24

$L180:
move	$6,$11
j	$L178
move	$8,$12

$L288:
addu	$2,$5,$24
lbu	$4,5200($2)
lbu	$3,5200($16)
sltu	$3,$4,$3
beq	$3,$0,$L176
sll	$2,$10,2

sll	$4,$6,1
addu	$2,$2,$24
slt	$3,$9,$4
beq	$3,$0,$L301
sw	$5,2900($2)

$L186:
move	$10,$6
sll	$2,$10,2
addu	$2,$2,$24
addiu	$14,$14,-1
sw	$15,2900($2)
bne	$14,$0,$L173
addiu	$17,$17,-4

j	$L307
addiu	$2,$19,5200

$L292:
addu	$5,$5,$24
lbu	$2,5200($4)
lbu	$3,5200($5)
sltu	$2,$3,$2
bne	$2,$0,$L309
addu	$2,$7,$24

$L195:
move	$8,$10
j	$L193
move	$7,$11

$L293:
addu	$2,$5,$24
lbu	$4,5200($2)
lbu	$3,5200($17)
sltu	$3,$4,$3
beq	$3,$0,$L191
sll	$2,$13,2

sll	$4,$8,1
addu	$2,$2,$24
slt	$3,$9,$4
beq	$3,$0,$L302
sw	$5,2900($2)

$L201:
j	$L191
move	$13,$8

$L297:
addu	$5,$5,$24
lbu	$2,5200($4)
lbu	$3,5200($5)
sltu	$2,$3,$2
bne	$2,$0,$L310
addu	$2,$7,$24

$L212:
move	$8,$10
j	$L210
move	$7,$11

$L298:
addu	$2,$5,$24
lbu	$4,5200($2)
lbu	$3,0($18)
sltu	$3,$4,$3
beq	$3,$0,$L208
sll	$2,$12,2

sll	$4,$8,1
addu	$2,$2,$24
slt	$3,$9,$4
beq	$3,$0,$L303
sw	$5,2900($2)

$L218:
move	$12,$8
sll	$2,$12,2
addu	$2,$2,$24
slt	$3,$9,2
sw	$19,2900($2)
addiu	$18,$18,1
beq	$3,$0,$L304
addiu	$14,$14,4

$L220:
lw	$3,5196($24)
lw	$4,2904($24)
addiu	$3,$3,-1
sll	$2,$3,2
lw	$12,0($21)
lw	$5,8($21)
addu	$2,$2,$24
sll	$6,$4,2
sw	$3,5196($24)
sw	$4,2900($2)
addu	$6,$6,$12
lw	$9,16($5)
sh	$0,2868($24)
sh	$0,2870($24)
sh	$0,2872($24)
sh	$0,2874($24)
sh	$0,2876($24)
sh	$0,2878($24)
sh	$0,2880($24)
sh	$0,2882($24)
sh	$0,2884($24)
sh	$0,2886($24)
sh	$0,2888($24)
sh	$0,2890($24)
sh	$0,2892($24)
sh	$0,2894($24)
sh	$0,2896($24)
sh	$0,2898($24)
lw	$13,4($21)
lw	$8,0($5)
lw	$17,4($5)
lw	$14,8($5)
sh	$0,2($6)
lw	$2,5196($24)
addiu	$10,$2,1
slt	$3,$10,573
beq	$3,$0,$L222
nop

bne	$8,$0,$L224
sll	$2,$10,2

addu	$2,$2,$24
addiu	$8,$2,2900
move	$15,$0
li	$16,573			# 0x23d
$L226:
lw	$4,0($8)
sll	$3,$4,2
addu	$6,$12,$3
lhu	$2,2($6)
slt	$7,$13,$4
sll	$2,$2,2
addu	$2,$2,$12
lhu	$3,2($2)
addiu	$5,$3,1
slt	$2,$9,$5
beq	$2,$0,$L227
slt	$11,$4,$14

addiu	$15,$15,1
move	$5,$9
$L227:
sll	$3,$5,1
bne	$7,$0,$L229
sh	$5,2($6)

subu	$2,$4,$14
addu	$3,$3,$24
sll	$2,$2,2
addu	$4,$2,$17
lhu	$2,2868($3)
addiu	$2,$2,1
bne	$11,$0,$L233
sh	$2,2868($3)

lw	$7,0($4)
$L233:
lhu	$2,0($6)
addu	$3,$5,$7
mul	$5,$2,$3
lw	$4,5792($24)
addu	$2,$5,$4
sw	$2,5792($24)
$L229:
addiu	$10,$10,1
bne	$10,$16,$L226
addiu	$8,$8,4

$L234:
beq	$15,$0,$L222
addiu	$10,$9,-1

sll	$2,$10,1
addu	$8,$2,$24
sll	$6,$9,1
addu	$7,$6,$24
move	$11,$8
$L244:
lhu	$2,2868($8)
bne	$2,$0,$L305
move	$4,$10

addiu	$5,$11,2868
$L248:
addiu	$5,$5,-2
lhu	$2,0($5)
addiu	$4,$4,-1
beq	$2,$0,$L248
sll	$3,$4,1

addiu	$4,$4,1
addu	$3,$3,$24
addiu	$2,$2,-1
sll	$4,$4,1
sh	$2,2868($3)
addu	$4,$4,$24
lhu	$3,2868($4)
addiu	$15,$15,-2
addiu	$3,$3,2
sh	$3,2868($4)
lhu	$2,2868($7)
addiu	$2,$2,-1
bgtz	$15,$L244
sh	$2,2868($7)

$L306:
beq	$9,$0,$L222
li	$8,573			# 0x23d

$L251:
addu	$2,$6,$24
lhu	$3,2868($2)
beq	$3,$0,$L252
sll	$2,$8,2

addu	$2,$2,$24
move	$6,$3
addiu	$5,$2,2900
addiu	$5,$5,-4
$L311:
lw	$2,0($5)
addiu	$8,$8,-1
sll	$3,$2,2
slt	$2,$13,$2
bne	$2,$0,$L255
addu	$4,$12,$3

lhu	$2,2($4)
addiu	$6,$6,-1
beq	$2,$9,$L255
subu	$7,$9,$2

lhu	$2,0($4)
lw	$3,5792($24)
mul	$11,$7,$2
addu	$2,$11,$3
sw	$2,5792($24)
sh	$9,2($4)
$L255:
bne	$6,$0,$L311
addiu	$5,$5,-4

addiu	$5,$5,4
$L252:
beq	$10,$0,$L222
move	$9,$10

addiu	$10,$10,-1
j	$L251
sll	$6,$9,1

$L203:
andi	$5,$2,0x00ff
andi	$2,$19,0xffff
sb	$5,0($18)
sh	$2,2($6)
sh	$2,2($7)
lw	$9,5192($24)
slt	$2,$9,2
beq	$2,$0,$L206
sw	$19,2904($24)

$L296:
j	$L208
li	$12,1			# 0x1

$L222:
lhu	$16,2868($24)
lhu	$15,2870($24)
sll	$16,$16,1
andi	$16,$16,0xffff
addu	$15,$16,$15
lhu	$14,2872($24)
sll	$15,$15,1
andi	$15,$15,0xffff
addu	$14,$15,$14
lhu	$13,2874($24)
sll	$14,$14,1
andi	$14,$14,0xffff
addu	$13,$14,$13
lhu	$12,2876($24)
sll	$13,$13,1
andi	$13,$13,0xffff
addu	$12,$13,$12
lhu	$11,2878($24)
sll	$12,$12,1
andi	$12,$12,0xffff
addu	$11,$12,$11
lhu	$10,2880($24)
sll	$11,$11,1
andi	$11,$11,0xffff
addu	$10,$11,$10
lhu	$9,2882($24)
sll	$10,$10,1
andi	$10,$10,0xffff
addu	$9,$10,$9
lhu	$8,2884($24)
sll	$9,$9,1
andi	$9,$9,0xffff
addu	$8,$9,$8
lhu	$7,2886($24)
sll	$8,$8,1
andi	$8,$8,0xffff
addu	$7,$8,$7
lhu	$6,2888($24)
sll	$7,$7,1
andi	$7,$7,0xffff
addu	$6,$7,$6
lhu	$5,2890($24)
sll	$6,$6,1
andi	$6,$6,0xffff
addu	$5,$6,$5
lhu	$4,2892($24)
sll	$5,$5,1
andi	$5,$5,0xffff
addu	$4,$5,$4
lhu	$3,2894($24)
sll	$4,$4,1
andi	$4,$4,0xffff
addu	$3,$4,$3
lhu	$2,2896($24)
sll	$3,$3,1
andi	$3,$3,0xffff
addu	$2,$3,$2
sll	$2,$2,1
sh	$2,30($sp)
sh	$16,2($sp)
sh	$15,4($sp)
sh	$14,6($sp)
sh	$13,8($sp)
sh	$12,10($sp)
sh	$11,12($sp)
sh	$10,14($sp)
sh	$9,16($sp)
sh	$8,18($sp)
sh	$7,20($sp)
sh	$6,22($sp)
sh	$5,24($sp)
sh	$4,26($sp)
bltz	$20,$L266
sh	$3,28($sp)

addiu	$4,$25,2
addiu	$10,$20,1
move	$7,$0
$L261:
lhu	$3,0($4)
beq	$3,$0,$L262
sll	$8,$3,1

addu	$2,$8,$sp
lhu	$6,0($2)
move	$5,$3
addiu	$2,$6,1
andi	$9,$2,0xffff
move	$3,$0
$L264:
andi	$2,$6,0x1
or	$2,$3,$2
addiu	$5,$5,-1
srl	$6,$6,1
bne	$5,$0,$L264
sll	$3,$2,1

addu	$2,$8,$sp
srl	$3,$3,1
sh	$9,0($2)
sh	$3,-2($4)
$L262:
addiu	$7,$7,1
bne	$7,$10,$L261
addiu	$4,$4,4

$L266:
lw	$21,52($sp)
lw	$20,48($sp)
lw	$19,44($sp)
lw	$18,40($sp)
lw	$17,36($sp)
lw	$16,32($sp)
j	$31
addiu	$sp,$sp,56

$L305:
sll	$3,$10,1
addiu	$4,$4,1
addu	$3,$3,$24
addiu	$2,$2,-1
sll	$4,$4,1
sh	$2,2868($3)
addu	$4,$4,$24
lhu	$3,2868($4)
addiu	$15,$15,-2
addiu	$3,$3,2
sh	$3,2868($4)
lhu	$2,2868($7)
addiu	$2,$2,-1
bgtz	$15,$L244
sh	$2,2868($7)

j	$L306
nop

$L286:
move	$10,$14
sll	$2,$10,2
addu	$2,$2,$24
addiu	$14,$14,-1
sw	$15,2900($2)
bne	$14,$0,$L173
addiu	$17,$17,-4

j	$L307
addiu	$2,$19,5200

$L283:
j	$L156
li	$20,-1			# 0xffffffffffffffff

$L280:
lw	$2,5192($24)
addiu	$3,$20,1
addiu	$9,$2,1
slt	$2,$2,2
slt	$5,$20,2
sll	$6,$3,2
beq	$2,$0,$L163
move	$4,$0

bne	$5,$0,$L169
sw	$9,5192($24)

move	$3,$0
move	$6,$0
$L171:
sll	$2,$9,2
addu	$2,$2,$24
sw	$3,2900($2)
addu	$4,$25,$4
addu	$5,$3,$24
li	$3,1
sh	$3,0($4)
lw	$2,5792($24)
sb	$0,5200($5)
addiu	$2,$2,-1
sw	$2,5792($24)
addu	$3,$7,$6
lhu	$4,2($3)
lw	$2,5796($24)
subu	$2,$2,$4
j	$L280
sw	$2,5796($24)

$L224:
addu	$2,$2,$24
addiu	$11,$2,2900
move	$15,$0
li	$19,573			# 0x23d
$L235:
lw	$4,0($11)
sll	$18,$4,2
addu	$6,$12,$18
lhu	$2,2($6)
slt	$7,$13,$4
sll	$2,$2,2
addu	$2,$2,$12
lhu	$3,2($2)
addiu	$5,$3,1
slt	$2,$9,$5
beq	$2,$0,$L236
slt	$16,$4,$14

addiu	$15,$15,1
move	$5,$9
$L236:
sll	$2,$5,1
bne	$7,$0,$L238
sh	$5,2($6)

subu	$3,$4,$14
addu	$2,$2,$24
sll	$3,$3,2
addu	$4,$3,$17
lhu	$3,2868($2)
addiu	$3,$3,1
bne	$16,$0,$L242
sh	$3,2868($2)

lw	$7,0($4)
$L242:
lhu	$4,0($6)
addu	$3,$5,$7
mul	$16,$4,$3
lw	$2,5792($24)
addu	$5,$8,$18
addu	$3,$16,$2
sw	$3,5792($24)
lhu	$2,2($5)
lw	$6,5796($24)
addu	$2,$7,$2
mul	$3,$4,$2
addu	$4,$3,$6
sw	$4,5796($24)
$L238:
addiu	$10,$10,1
bne	$10,$19,$L235
addiu	$11,$11,4

j	$L234
nop

$L169:
move	$20,$3
j	$L171
move	$4,$6

.set	macro
.set	reorder
.end	build_tree
.size	build_tree, .-build_tree
.section	.text._tr_flush_block,"ax",@progbits
.align	2
.align	5
.globl	_tr_flush_block
.ent	_tr_flush_block
_tr_flush_block:
.frame	$sp,48,$31		# vars= 0, regs= 7/0, args= 16, gp= 0
.mask	0x803f0000,-8
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

addiu	$sp,$sp,-48
sw	$21,36($sp)
sw	$19,28($sp)
sw	$18,24($sp)
sw	$17,20($sp)
sw	$31,40($sp)
sw	$20,32($sp)
sw	$16,16($sp)
lw	$2,124($4)
move	$19,$4
move	$17,$5
move	$18,$6
blez	$2,$L314
move	$21,$7

lbu	$3,28($4)
li	$2,2			# 0x2
beq	$3,$2,$L455
li	$7,484			# 0x1e4

$L316:
lui	$20,%hi(build_tree)
addiu	$5,$19,2832
addiu	$16,$20,%lo(build_tree)
jal	$16
move	$4,$19

move	$4,$19
jal	$16
addiu	$5,$19,2844

addiu	$6,$19,140
lhu	$5,2($6)
beq	$5,$0,$L322
lw	$4,2836($19)

li	$10,4			# 0x4
li	$8,7			# 0x7
$L324:
sll	$2,$4,2
addu	$2,$2,$6
li	$3,-1
bltz	$4,$L325
sh	$3,6($2)

addiu	$11,$4,1
move	$7,$6
move	$9,$0
li	$3,-1			# 0xffffffffffffffff
move	$4,$0
$L327:
addiu	$4,$4,1
slt	$2,$4,$8
beq	$2,$0,$L328
lhu	$6,6($7)

beq	$5,$6,$L456
nop

$L328:
slt	$2,$4,$10
beq	$2,$0,$L332
nop

sll	$3,$5,2
addu	$3,$3,$19
lhu	$2,2676($3)
addu	$2,$4,$2
sh	$2,2676($3)
$L334:
bne	$6,$0,$L341
nop

$L457:
li	$10,3			# 0x3
move	$4,$0
li	$8,138			# 0x8a
$L331:
addiu	$9,$9,1
beq	$11,$9,$L325
addiu	$7,$7,4

$L459:
move	$3,$5
j	$L327
move	$5,$6

$L322:
li	$10,3			# 0x3
j	$L324
li	$8,138			# 0x8a

$L332:
beq	$5,$0,$L335
slt	$2,$4,11

beq	$5,$3,$L337
sll	$3,$5,2

addu	$3,$3,$19
lhu	$2,2676($3)
addiu	$2,$2,1
sh	$2,2676($3)
$L337:
lhu	$2,2740($19)
addiu	$2,$2,1
beq	$6,$0,$L457
sh	$2,2740($19)

$L341:
beq	$5,$6,$L458
li	$10,3			# 0x3

addiu	$9,$9,1
li	$10,4			# 0x4
move	$4,$0
li	$8,7			# 0x7
bne	$11,$9,$L459
addiu	$7,$7,4

$L325:
addiu	$6,$19,2432
lhu	$5,2($6)
beq	$5,$0,$L346
lw	$4,2848($19)

li	$9,7			# 0x7
li	$8,4			# 0x4
$L348:
sll	$2,$4,2
addu	$2,$2,$6
li	$3,-1
bltz	$4,$L349
sh	$3,6($2)

addiu	$11,$4,1
move	$7,$6
li	$3,-1			# 0xffffffffffffffff
move	$10,$0
move	$4,$0
$L351:
addiu	$4,$4,1
slt	$2,$4,$9
beq	$2,$0,$L352
lhu	$6,6($7)

beq	$6,$5,$L354
nop

$L352:
slt	$2,$4,$8
beq	$2,$0,$L355
nop

sll	$3,$5,2
addu	$3,$3,$19
lhu	$2,2676($3)
addu	$2,$4,$2
sh	$2,2676($3)
$L357:
bne	$6,$0,$L364
nop

$L460:
move	$3,$5
move	$4,$0
li	$9,138			# 0x8a
li	$8,3			# 0x3
$L354:
addiu	$10,$10,1
beq	$10,$11,$L349
addiu	$7,$7,4

$L462:
j	$L351
move	$5,$6

$L335:
beq	$2,$0,$L339
nop

lhu	$2,2744($19)
addiu	$2,$2,1
j	$L334
sh	$2,2744($19)

$L456:
j	$L331
move	$5,$3

$L458:
move	$4,$0
j	$L331
li	$8,6			# 0x6

$L355:
beq	$5,$0,$L358
slt	$2,$4,11

beq	$3,$5,$L360
sll	$3,$5,2

addu	$3,$3,$19
lhu	$2,2676($3)
addiu	$2,$2,1
sh	$2,2676($3)
$L360:
lhu	$2,2740($19)
addiu	$2,$2,1
beq	$6,$0,$L460
sh	$2,2740($19)

$L364:
beq	$6,$5,$L461
move	$3,$6

addiu	$10,$10,1
move	$3,$5
move	$4,$0
li	$9,7			# 0x7
li	$8,4			# 0x4
bne	$10,$11,$L462
addiu	$7,$7,4

$L349:
addiu	$3,$20,%lo(build_tree)
move	$4,$19
jal	$3
addiu	$5,$19,2856

lhu	$2,2738($19)
bne	$2,$0,$L371
li	$7,18			# 0x12

lhu	$2,2682($19)
bne	$2,$0,$L371
li	$7,17			# 0x11

lhu	$2,2734($19)
bne	$2,$0,$L371
li	$7,16			# 0x10

lhu	$2,2686($19)
bne	$2,$0,$L371
li	$7,15			# 0xf

lhu	$2,2730($19)
bne	$2,$0,$L371
li	$7,14			# 0xe

lhu	$2,2690($19)
bne	$2,$0,$L371
li	$7,13			# 0xd

lhu	$2,2726($19)
bne	$2,$0,$L371
li	$7,12			# 0xc

lhu	$2,2694($19)
bne	$2,$0,$L371
li	$7,11			# 0xb

lhu	$2,2722($19)
bne	$2,$0,$L371
li	$7,10			# 0xa

lhu	$2,2698($19)
bne	$2,$0,$L371
li	$7,9			# 0x9

lhu	$2,2718($19)
bne	$2,$0,$L371
li	$7,8			# 0x8

lhu	$2,2702($19)
bne	$2,$0,$L371
li	$7,7			# 0x7

lhu	$2,2714($19)
bne	$2,$0,$L371
li	$7,6			# 0x6

lhu	$2,2706($19)
bne	$2,$0,$L371
li	$7,5			# 0x5

lhu	$2,2710($19)
bne	$2,$0,$L477
li	$2,3			# 0x3

lhu	$3,2678($19)
li	$7,2			# 0x2
movn	$7,$2,$3
$L371:
lw	$2,5792($19)
sll	$4,$7,1
addiu	$2,$2,17
addu	$4,$4,$7
lw	$3,5796($19)
addu	$4,$4,$2
addiu	$5,$4,10
addiu	$3,$3,10
srl	$5,$5,3
srl	$3,$3,3
sltu	$2,$5,$3
bne	$2,$0,$L402
sw	$4,5792($19)

move	$5,$3
$L402:
addiu	$2,$18,4
sltu	$2,$5,$2
bne	$2,$0,$L404
nop

beq	$17,$0,$L404
lui	$2,%hi(_tr_stored_block)

move	$5,$17
move	$6,$18
move	$4,$19
addiu	$2,$2,%lo(_tr_stored_block)
jal	$2
move	$7,$21

move	$3,$0
$L480:
li	$4,1144			# 0x478
$L431:
addu	$2,$3,$19
addiu	$3,$3,4
bne	$3,$4,$L431
sh	$0,140($2)

move	$3,$0
li	$4,120			# 0x78
$L433:
addu	$2,$19,$3
addiu	$3,$3,4
bne	$3,$4,$L433
sh	$0,2432($2)

move	$3,$0
li	$4,76			# 0x4c
$L435:
addu	$2,$19,$3
addiu	$3,$3,4
bne	$3,$4,$L435
sh	$0,2676($2)

li	$2,1
sh	$2,1164($19)
sw	$0,5796($19)
sw	$0,5792($19)
sw	$0,5800($19)
beq	$21,$0,$L443
sw	$0,5784($19)

lw	$3,5812($19)
slt	$2,$3,9
bne	$2,$0,$L439
nop

lw	$4,20($19)
lw	$2,8($19)
lhu	$5,5808($19)
addu	$2,$2,$4
sb	$5,0($2)
lw	$3,8($19)
lhu	$2,5808($19)
addu	$3,$4,$3
srl	$2,$2,8
addiu	$4,$4,2
sb	$2,1($3)
sw	$4,20($19)
$L441:
sw	$0,5812($19)
sh	$0,5808($19)
$L443:
lw	$31,40($sp)
lw	$21,36($sp)
lw	$20,32($sp)
lw	$19,28($sp)
lw	$18,24($sp)
lw	$17,20($sp)
lw	$16,16($sp)
j	$31
addiu	$sp,$sp,48

$L358:
beq	$2,$0,$L362
nop

lhu	$2,2744($19)
addiu	$2,$2,1
j	$L357
sh	$2,2744($19)

$L461:
move	$4,$0
li	$9,6			# 0x6
j	$L354
li	$8,3			# 0x3

$L339:
lhu	$2,2748($19)
addiu	$2,$2,1
j	$L334
sh	$2,2748($19)

$L362:
lhu	$2,2748($19)
addiu	$2,$2,1
j	$L357
sh	$2,2748($19)

$L404:
beq	$5,$3,$L478
nop

lw	$4,5812($19)
slt	$2,$4,14
bne	$2,$0,$L413
addiu	$2,$21,4

lhu	$3,5808($19)
addiu	$5,$21,4
lw	$6,20($19)
sll	$2,$5,$4
lw	$4,8($19)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$6
sh	$2,5808($19)
sb	$2,0($4)
lhu	$3,5808($19)
lw	$2,8($19)
srl	$3,$3,8
addu	$2,$6,$2
sb	$3,1($2)
lw	$8,5812($19)
li	$2,16			# 0x10
subu	$2,$2,$8
andi	$5,$5,0xffff
sra	$5,$5,$2
addiu	$6,$6,2
addiu	$8,$8,-13
sw	$6,20($19)
sh	$5,5808($19)
sw	$8,5812($19)
$L415:
lw	$5,2836($19)
lw	$2,2848($19)
slt	$3,$8,12
addiu	$9,$2,1
addiu	$11,$7,1
bne	$3,$0,$L416
addiu	$13,$5,1

lhu	$3,5808($19)
addiu	$5,$5,-256
lw	$6,20($19)
sll	$2,$5,$8
lw	$4,8($19)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$6
sh	$2,5808($19)
sb	$2,0($4)
lhu	$3,5808($19)
lw	$2,8($19)
srl	$3,$3,8
addu	$2,$6,$2
sb	$3,1($2)
lw	$8,5812($19)
li	$2,16			# 0x10
subu	$2,$2,$8
andi	$5,$5,0xffff
sra	$5,$5,$2
addiu	$6,$6,2
addiu	$8,$8,-11
sw	$6,20($19)
sh	$5,5808($19)
sw	$8,5812($19)
$L418:
slt	$2,$8,12
bne	$2,$0,$L419
addiu	$20,$9,-1

lhu	$3,5808($19)
lw	$5,20($19)
lw	$4,8($19)
sll	$2,$20,$8
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$5
sh	$2,5808($19)
sb	$2,0($4)
lhu	$3,5808($19)
lw	$2,8($19)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$8,5812($19)
li	$2,16			# 0x10
subu	$2,$2,$8
andi	$3,$20,0xffff
sra	$3,$3,$2
addiu	$5,$5,2
addiu	$8,$8,-11
sw	$5,20($19)
sh	$3,5808($19)
sw	$8,5812($19)
$L421:
slt	$2,$8,13
bne	$2,$0,$L422
addiu	$2,$11,-4

lhu	$3,5808($19)
addiu	$5,$11,-4
lw	$6,20($19)
sll	$2,$5,$8
lw	$4,8($19)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$6
sh	$2,5808($19)
sb	$2,0($4)
lhu	$3,5808($19)
lw	$2,8($19)
srl	$3,$3,8
addu	$2,$6,$2
sb	$3,1($2)
lw	$4,5812($19)
li	$2,16			# 0x10
andi	$5,$5,0xffff
subu	$2,$2,$4
sra	$5,$5,$2
addiu	$6,$6,2
addiu	$8,$4,-12
sw	$6,20($19)
sh	$5,5808($19)
sw	$8,5812($19)
$L424:
blez	$11,$L425
lui	$2,%hi(bl_order)

addiu	$9,$2,%lo(bl_order)
move	$10,$0
j	$L427
li	$12,16			# 0x10

$L479:
lbu	$2,0($9)
lhu	$3,5808($19)
sll	$2,$2,2
addu	$2,$2,$19
lhu	$6,2678($2)
lw	$5,20($19)
lw	$4,8($19)
sll	$2,$6,$7
or	$2,$2,$3
addu	$4,$4,$5
andi	$2,$2,0xffff
sh	$2,5808($19)
sb	$2,0($4)
lhu	$3,5808($19)
lw	$2,8($19)
srl	$3,$3,8
addu	$2,$5,$2
sb	$3,1($2)
lw	$4,5812($19)
addiu	$5,$5,2
subu	$2,$12,$4
sra	$6,$6,$2
addiu	$8,$4,-13
addiu	$10,$10,1
sw	$5,20($19)
sh	$6,5808($19)
sw	$8,5812($19)
beq	$10,$11,$L425
addiu	$9,$9,1

$L427:
lw	$7,5812($19)
slt	$2,$7,14
beq	$2,$0,$L479
addiu	$8,$7,3

lbu	$2,0($9)
lhu	$4,5808($19)
sll	$2,$2,2
addu	$2,$2,$19
lhu	$3,2678($2)
addiu	$10,$10,1
sll	$3,$3,$7
or	$3,$3,$4
sw	$8,5812($19)
sh	$3,5808($19)
bne	$10,$11,$L427
addiu	$9,$9,1

$L425:
addiu	$18,$19,140
lui	$16,%hi(send_tree)
addiu	$6,$13,-1
addiu	$16,$16,%lo(send_tree)
move	$4,$19
move	$5,$18
jal	$16
addiu	$17,$19,2432

move	$6,$20
move	$4,$19
jal	$16
move	$5,$17

lui	$2,%hi(compress_block)
move	$4,$19
move	$5,$18
addiu	$2,$2,%lo(compress_block)
jal	$2
move	$6,$17

j	$L480
move	$3,$0

$L346:
li	$9,138			# 0x8a
j	$L348
li	$8,3			# 0x3

$L439:
blez	$3,$L441
nop

lw	$2,20($19)
lw	$3,8($19)
lhu	$4,5808($19)
addu	$3,$3,$2
addiu	$2,$2,1
sb	$4,0($3)
j	$L441
sw	$2,20($19)

$L422:
lhu	$3,5808($19)
sll	$2,$2,$8
or	$2,$2,$3
addiu	$8,$8,4
sh	$2,5808($19)
j	$L424
sw	$8,5812($19)

$L419:
lhu	$3,5808($19)
sll	$2,$20,$8
or	$2,$2,$3
addiu	$8,$8,5
sh	$2,5808($19)
j	$L421
sw	$8,5812($19)

$L416:
lhu	$3,5808($19)
addiu	$2,$5,-256
sll	$2,$2,$8
or	$2,$2,$3
addiu	$8,$8,5
sh	$2,5808($19)
j	$L418
sw	$8,5812($19)

$L413:
lhu	$3,5808($19)
sll	$2,$2,$4
or	$2,$2,$3
addiu	$8,$4,3
sh	$2,5808($19)
j	$L415
sw	$8,5812($19)

$L455:
lhu	$5,140($4)
lhu	$3,144($4)
lhu	$4,148($4)
addu	$3,$3,$5
lhu	$2,152($19)
addu	$4,$4,$3
lhu	$5,156($19)
addu	$2,$2,$4
lhu	$6,160($19)
addu	$2,$2,$5
lhu	$3,164($19)
addu	$2,$2,$6
addu	$5,$2,$3
move	$6,$0
move	$4,$0
$L318:
addu	$2,$4,$19
lhu	$3,168($2)
addiu	$4,$4,4
bne	$4,$7,$L318
addu	$6,$6,$3

move	$4,$0
li	$7,512			# 0x200
$L320:
addu	$2,$4,$19
lhu	$3,652($2)
addiu	$4,$4,4
bne	$4,$7,$L320
addu	$5,$5,$3

srl	$2,$6,2
sltu	$2,$2,$5
xori	$2,$2,0x1
j	$L316
sb	$2,28($19)

$L314:
addiu	$3,$6,5
move	$5,$3
j	$L402
move	$7,$0

$L478:
lw	$4,5812($19)
slt	$2,$4,14
bne	$2,$0,$L410
addiu	$2,$21,2

lhu	$3,5808($19)
addiu	$5,$21,2
lw	$6,20($19)
sll	$2,$5,$4
lw	$4,8($19)
or	$2,$2,$3
andi	$2,$2,0xffff
addu	$4,$4,$6
sh	$2,5808($19)
sb	$2,0($4)
lhu	$3,5808($19)
lw	$2,8($19)
srl	$3,$3,8
addu	$2,$6,$2
sb	$3,1($2)
lw	$4,5812($19)
li	$2,16			# 0x10
andi	$5,$5,0xffff
subu	$2,$2,$4
sra	$5,$5,$2
addiu	$6,$6,2
addiu	$8,$4,-13
sw	$6,20($19)
sh	$5,5808($19)
sw	$8,5812($19)
$L412:
lui	$5,%hi(static_ltree)
lui	$6,%hi(static_dtree)
lui	$2,%hi(compress_block)
addiu	$5,$5,%lo(static_ltree)
addiu	$6,$6,%lo(static_dtree)
addiu	$2,$2,%lo(compress_block)
jal	$2
move	$4,$19

j	$L480
move	$3,$0

$L410:
lhu	$3,5808($19)
sll	$2,$2,$4
or	$2,$2,$3
addiu	$8,$4,3
sh	$2,5808($19)
j	$L412
sw	$8,5812($19)

$L477:
j	$L371
li	$7,4			# 0x4

.set	macro
.set	reorder
.end	_tr_flush_block
.size	_tr_flush_block, .-_tr_flush_block
.globl	_dist_code
.rdata
.align	2
.type	_dist_code, @object
.size	_dist_code, 512
_dist_code:
.byte	0
.byte	1
.byte	2
.byte	3
.byte	4
.byte	4
.byte	5
.byte	5
.byte	6
.byte	6
.byte	6
.byte	6
.byte	7
.byte	7
.byte	7
.byte	7
.byte	8
.byte	8
.byte	8
.byte	8
.byte	8
.byte	8
.byte	8
.byte	8
.byte	9
.byte	9
.byte	9
.byte	9
.byte	9
.byte	9
.byte	9
.byte	9
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	10
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	11
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	12
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	13
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	14
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	15
.byte	0
.byte	0
.byte	16
.byte	17
.byte	18
.byte	18
.byte	19
.byte	19
.byte	20
.byte	20
.byte	20
.byte	20
.byte	21
.byte	21
.byte	21
.byte	21
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	28
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.byte	29
.globl	_length_code
.align	2
.type	_length_code, @object
.size	_length_code, 256
_length_code:
.byte	0
.byte	1
.byte	2
.byte	3
.byte	4
.byte	5
.byte	6
.byte	7
.byte	8
.byte	8
.byte	9
.byte	9
.byte	10
.byte	10
.byte	11
.byte	11
.byte	12
.byte	12
.byte	12
.byte	12
.byte	13
.byte	13
.byte	13
.byte	13
.byte	14
.byte	14
.byte	14
.byte	14
.byte	15
.byte	15
.byte	15
.byte	15
.byte	16
.byte	16
.byte	16
.byte	16
.byte	16
.byte	16
.byte	16
.byte	16
.byte	17
.byte	17
.byte	17
.byte	17
.byte	17
.byte	17
.byte	17
.byte	17
.byte	18
.byte	18
.byte	18
.byte	18
.byte	18
.byte	18
.byte	18
.byte	18
.byte	19
.byte	19
.byte	19
.byte	19
.byte	19
.byte	19
.byte	19
.byte	19
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	20
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	21
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	22
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	23
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	24
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	25
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	26
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	27
.byte	28
.align	2
.type	static_ltree, @object
.size	static_ltree, 1152
static_ltree:
.half	12
.half	8
.half	140
.half	8
.half	76
.half	8
.half	204
.half	8
.half	44
.half	8
.half	172
.half	8
.half	108
.half	8
.half	236
.half	8
.half	28
.half	8
.half	156
.half	8
.half	92
.half	8
.half	220
.half	8
.half	60
.half	8
.half	188
.half	8
.half	124
.half	8
.half	252
.half	8
.half	2
.half	8
.half	130
.half	8
.half	66
.half	8
.half	194
.half	8
.half	34
.half	8
.half	162
.half	8
.half	98
.half	8
.half	226
.half	8
.half	18
.half	8
.half	146
.half	8
.half	82
.half	8
.half	210
.half	8
.half	50
.half	8
.half	178
.half	8
.half	114
.half	8
.half	242
.half	8
.half	10
.half	8
.half	138
.half	8
.half	74
.half	8
.half	202
.half	8
.half	42
.half	8
.half	170
.half	8
.half	106
.half	8
.half	234
.half	8
.half	26
.half	8
.half	154
.half	8
.half	90
.half	8
.half	218
.half	8
.half	58
.half	8
.half	186
.half	8
.half	122
.half	8
.half	250
.half	8
.half	6
.half	8
.half	134
.half	8
.half	70
.half	8
.half	198
.half	8
.half	38
.half	8
.half	166
.half	8
.half	102
.half	8
.half	230
.half	8
.half	22
.half	8
.half	150
.half	8
.half	86
.half	8
.half	214
.half	8
.half	54
.half	8
.half	182
.half	8
.half	118
.half	8
.half	246
.half	8
.half	14
.half	8
.half	142
.half	8
.half	78
.half	8
.half	206
.half	8
.half	46
.half	8
.half	174
.half	8
.half	110
.half	8
.half	238
.half	8
.half	30
.half	8
.half	158
.half	8
.half	94
.half	8
.half	222
.half	8
.half	62
.half	8
.half	190
.half	8
.half	126
.half	8
.half	254
.half	8
.half	1
.half	8
.half	129
.half	8
.half	65
.half	8
.half	193
.half	8
.half	33
.half	8
.half	161
.half	8
.half	97
.half	8
.half	225
.half	8
.half	17
.half	8
.half	145
.half	8
.half	81
.half	8
.half	209
.half	8
.half	49
.half	8
.half	177
.half	8
.half	113
.half	8
.half	241
.half	8
.half	9
.half	8
.half	137
.half	8
.half	73
.half	8
.half	201
.half	8
.half	41
.half	8
.half	169
.half	8
.half	105
.half	8
.half	233
.half	8
.half	25
.half	8
.half	153
.half	8
.half	89
.half	8
.half	217
.half	8
.half	57
.half	8
.half	185
.half	8
.half	121
.half	8
.half	249
.half	8
.half	5
.half	8
.half	133
.half	8
.half	69
.half	8
.half	197
.half	8
.half	37
.half	8
.half	165
.half	8
.half	101
.half	8
.half	229
.half	8
.half	21
.half	8
.half	149
.half	8
.half	85
.half	8
.half	213
.half	8
.half	53
.half	8
.half	181
.half	8
.half	117
.half	8
.half	245
.half	8
.half	13
.half	8
.half	141
.half	8
.half	77
.half	8
.half	205
.half	8
.half	45
.half	8
.half	173
.half	8
.half	109
.half	8
.half	237
.half	8
.half	29
.half	8
.half	157
.half	8
.half	93
.half	8
.half	221
.half	8
.half	61
.half	8
.half	189
.half	8
.half	125
.half	8
.half	253
.half	8
.half	19
.half	9
.half	275
.half	9
.half	147
.half	9
.half	403
.half	9
.half	83
.half	9
.half	339
.half	9
.half	211
.half	9
.half	467
.half	9
.half	51
.half	9
.half	307
.half	9
.half	179
.half	9
.half	435
.half	9
.half	115
.half	9
.half	371
.half	9
.half	243
.half	9
.half	499
.half	9
.half	11
.half	9
.half	267
.half	9
.half	139
.half	9
.half	395
.half	9
.half	75
.half	9
.half	331
.half	9
.half	203
.half	9
.half	459
.half	9
.half	43
.half	9
.half	299
.half	9
.half	171
.half	9
.half	427
.half	9
.half	107
.half	9
.half	363
.half	9
.half	235
.half	9
.half	491
.half	9
.half	27
.half	9
.half	283
.half	9
.half	155
.half	9
.half	411
.half	9
.half	91
.half	9
.half	347
.half	9
.half	219
.half	9
.half	475
.half	9
.half	59
.half	9
.half	315
.half	9
.half	187
.half	9
.half	443
.half	9
.half	123
.half	9
.half	379
.half	9
.half	251
.half	9
.half	507
.half	9
.half	7
.half	9
.half	263
.half	9
.half	135
.half	9
.half	391
.half	9
.half	71
.half	9
.half	327
.half	9
.half	199
.half	9
.half	455
.half	9
.half	39
.half	9
.half	295
.half	9
.half	167
.half	9
.half	423
.half	9
.half	103
.half	9
.half	359
.half	9
.half	231
.half	9
.half	487
.half	9
.half	23
.half	9
.half	279
.half	9
.half	151
.half	9
.half	407
.half	9
.half	87
.half	9
.half	343
.half	9
.half	215
.half	9
.half	471
.half	9
.half	55
.half	9
.half	311
.half	9
.half	183
.half	9
.half	439
.half	9
.half	119
.half	9
.half	375
.half	9
.half	247
.half	9
.half	503
.half	9
.half	15
.half	9
.half	271
.half	9
.half	143
.half	9
.half	399
.half	9
.half	79
.half	9
.half	335
.half	9
.half	207
.half	9
.half	463
.half	9
.half	47
.half	9
.half	303
.half	9
.half	175
.half	9
.half	431
.half	9
.half	111
.half	9
.half	367
.half	9
.half	239
.half	9
.half	495
.half	9
.half	31
.half	9
.half	287
.half	9
.half	159
.half	9
.half	415
.half	9
.half	95
.half	9
.half	351
.half	9
.half	223
.half	9
.half	479
.half	9
.half	63
.half	9
.half	319
.half	9
.half	191
.half	9
.half	447
.half	9
.half	127
.half	9
.half	383
.half	9
.half	255
.half	9
.half	511
.half	9
.half	0
.half	7
.half	64
.half	7
.half	32
.half	7
.half	96
.half	7
.half	16
.half	7
.half	80
.half	7
.half	48
.half	7
.half	112
.half	7
.half	8
.half	7
.half	72
.half	7
.half	40
.half	7
.half	104
.half	7
.half	24
.half	7
.half	88
.half	7
.half	56
.half	7
.half	120
.half	7
.half	4
.half	7
.half	68
.half	7
.half	36
.half	7
.half	100
.half	7
.half	20
.half	7
.half	84
.half	7
.half	52
.half	7
.half	116
.half	7
.half	3
.half	8
.half	131
.half	8
.half	67
.half	8
.half	195
.half	8
.half	35
.half	8
.half	163
.half	8
.half	99
.half	8
.half	227
.half	8
.align	2
.type	static_dtree, @object
.size	static_dtree, 120
static_dtree:
.half	0
.half	5
.half	16
.half	5
.half	8
.half	5
.half	24
.half	5
.half	4
.half	5
.half	20
.half	5
.half	12
.half	5
.half	28
.half	5
.half	2
.half	5
.half	18
.half	5
.half	10
.half	5
.half	26
.half	5
.half	6
.half	5
.half	22
.half	5
.half	14
.half	5
.half	30
.half	5
.half	1
.half	5
.half	17
.half	5
.half	9
.half	5
.half	25
.half	5
.half	5
.half	5
.half	21
.half	5
.half	13
.half	5
.half	29
.half	5
.half	3
.half	5
.half	19
.half	5
.half	11
.half	5
.half	27
.half	5
.half	7
.half	5
.half	23
.half	5
.align	2
.type	bl_order, @object
.size	bl_order, 19
bl_order:
.byte	16
.byte	17
.byte	18
.byte	0
.byte	8
.byte	7
.byte	9
.byte	6
.byte	10
.byte	5
.byte	11
.byte	4
.byte	12
.byte	3
.byte	13
.byte	2
.byte	14
.byte	1
.byte	15
.align	2
.type	extra_lbits, @object
.size	extra_lbits, 116
extra_lbits:
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	1
.word	1
.word	1
.word	1
.word	2
.word	2
.word	2
.word	2
.word	3
.word	3
.word	3
.word	3
.word	4
.word	4
.word	4
.word	4
.word	5
.word	5
.word	5
.word	5
.word	0
.align	2
.type	base_length, @object
.size	base_length, 116
base_length:
.word	0
.word	1
.word	2
.word	3
.word	4
.word	5
.word	6
.word	7
.word	8
.word	10
.word	12
.word	14
.word	16
.word	20
.word	24
.word	28
.word	32
.word	40
.word	48
.word	56
.word	64
.word	80
.word	96
.word	112
.word	128
.word	160
.word	192
.word	224
.word	0
.align	2
.type	extra_dbits, @object
.size	extra_dbits, 120
extra_dbits:
.word	0
.word	0
.word	0
.word	0
.word	1
.word	1
.word	2
.word	2
.word	3
.word	3
.word	4
.word	4
.word	5
.word	5
.word	6
.word	6
.word	7
.word	7
.word	8
.word	8
.word	9
.word	9
.word	10
.word	10
.word	11
.word	11
.word	12
.word	12
.word	13
.word	13
.align	2
.type	base_dist, @object
.size	base_dist, 120
base_dist:
.word	0
.word	1
.word	2
.word	3
.word	4
.word	6
.word	8
.word	12
.word	16
.word	24
.word	32
.word	48
.word	64
.word	96
.word	128
.word	192
.word	256
.word	384
.word	512
.word	768
.word	1024
.word	1536
.word	2048
.word	3072
.word	4096
.word	6144
.word	8192
.word	12288
.word	16384
.word	24576
.data
.align	2
.type	static_l_desc, @object
.size	static_l_desc, 20
static_l_desc:
.word	static_ltree
.word	extra_lbits
.word	257
.word	286
.word	15
.align	2
.type	static_d_desc, @object
.size	static_d_desc, 20
static_d_desc:
.word	static_dtree
.word	extra_dbits
.word	0
.word	30
.word	15
.align	2
.type	static_bl_desc, @object
.size	static_bl_desc, 20
static_bl_desc:
.word	0
.word	extra_blbits
.word	0
.word	19
.word	7
.rdata
.align	2
.type	extra_blbits, @object
.size	extra_blbits, 76
extra_blbits:
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	0
.word	2
.word	3
.word	7
.ident	"GCC: (GNU) 4.1.2"
