.file	1 "zutil.c"
.section .mdebug.abi32
.previous
.section	.rodata.str1.4,"aMS",@progbits,1
.align	2
$LC0:
.ascii	"1.2.1\000"
.section	.text.zlibVersion,"ax",@progbits
.align	2
.align	5
.globl	zlibVersion
.ent	zlibVersion
zlibVersion:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

lui	$2,%hi($LC0)
j	$31
addiu	$2,$2,%lo($LC0)

.set	macro
.set	reorder
.end	zlibVersion
.size	zlibVersion, .-zlibVersion
.section	.text.zlibCompileFlags,"ax",@progbits
.align	2
.align	5
.globl	zlibCompileFlags
.ent	zlibCompileFlags
zlibCompileFlags:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

j	$31
li	$2,85			# 0x55

.set	macro
.set	reorder
.end	zlibCompileFlags
.size	zlibCompileFlags, .-zlibCompileFlags
.section	.text.zError,"ax",@progbits
.align	2
.align	5
.globl	zError
.ent	zError
zError:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

li	$3,2			# 0x2
subu	$3,$3,$4
lui	$2,%hi(z_errmsg)
addiu	$2,$2,%lo(z_errmsg)
sll	$3,$3,2
addu	$3,$3,$2
j	$31
lw	$2,0($3)

.set	macro
.set	reorder
.end	zError
.size	zError, .-zError
.section	.text.zcfree,"ax",@progbits
.align	2
.align	5
.globl	zcfree
.ent	zcfree
zcfree:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

lui	$25,%hi(uc_free)
addiu	$25,$25,%lo(uc_free)
jr	$25
move	$4,$5

.set	macro
.set	reorder
.end	zcfree
.size	zcfree, .-zcfree
.section	.text.zcalloc,"ax",@progbits
.align	2
.align	5
.globl	zcalloc
.ent	zcalloc
zcalloc:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
.mask	0x00000000,0
.fmask	0x00000000,0
.set	noreorder
.set	nomacro

lui	$25,%hi(uc_malloc)
addiu	$25,$25,%lo(uc_malloc)
jr	$25
mul	$4,$5,$6

.set	macro
.set	reorder
.end	zcalloc
.size	zcalloc, .-zcalloc
.globl	z_errmsg
.section	.rodata.str1.4
.align	2
$LC1:
.ascii	"need dictionary\000"
.align	2
$LC2:
.ascii	"stream end\000"
.subsection	-1
.align	2
$LC3:
.ascii	"\000"
.align	2
$LC4:
.ascii	"file error\000"
.align	2
$LC5:
.ascii	"stream error\000"
.align	2
$LC6:
.ascii	"data error\000"
.align	2
$LC7:
.ascii	"insufficient memory\000"
.align	2
$LC8:
.ascii	"buffer error\000"
.align	2
$LC9:
.ascii	"incompatible version\000"
.rdata
.align	2
.type	z_errmsg, @object
.size	z_errmsg, 40
z_errmsg:
.word	$LC1
.word	$LC2
.word	$LC3
.word	$LC4
.word	$LC5
.word	$LC6
.word	$LC7
.word	$LC8
.word	$LC9
.word	$LC3
.ident	"GCC: (GNU) 4.1.2"
