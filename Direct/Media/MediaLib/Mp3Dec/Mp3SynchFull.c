//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－合成多相滤波器
//        --------------------------------------
//                 版权: 新诺亚舟科技
//             ---------------------------
//                  版   本   历   史
//        --------------------------------------
//  版本    日前		说明		
//  V0.1    2005-8      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]
#include "Mp3Prv.h"

#ifdef CONFIG_DECODE_MP3_ENABLE

#define DB_INT(x)			CONST_INT(x)
#define DCT_MUL(x, y) 		(((x) * (y)) >> 13)
#define DCT_SHIFT(x)		(x)
#define ML0(hi, lo, x, y)	((lo)  = (((int)(x) * (int)(y))))
#define MLA(hi, lo, x, y)	((lo) += (((int)(x) * (int)(y))))
#define MLN(hi, lo)			((lo)  = -(lo))
#define MLZ(hi, lo)			((MP3INT) (lo) >> 14)

extern MP3INT const Mp3SynthFullDW[17][32];
#if defined(CONFIG_ARCH_XBURST)	&& !defined(WIN32)	
extern void MxuMp3SynthDct32(int const in[32], unsigned int slot, int lo[16][8], int hi[16][8]);
extern int MxuMp3SynthMac0(int v, int *ptr0, int const *ptr1);
extern int MxuMp3SynthMac1(int v, int *ptr0, int const *ptr1);
#endif

#if !defined(CONFIG_ARCH_XBURST) || defined(WIN32)	
////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static void Mp3SynthDct32 (MP3INT *in, unsigned int slot, 
            MP3INT lo[16][8], MP3INT hi[16][8])
{
	MP3INT t0, t1, t2, t3, t4, t5, t6, t7;
	MP3INT t8, t9, t10, t11, t12, t13, t14, t15;
	MP3INT t16, t17, t18, t19, t20, t21, t22, t23;
	MP3INT t24, t25, t26, t27, t28, t29, t30, t31;
	MP3INT t32, t33, t34, t35, t36, t37, t38, t39;
	MP3INT t40, t41, t42, t43, t44, t45, t46, t47;
	MP3INT t48, t49, t50, t51, t52, t53, t54, t55;
	MP3INT t56, t57, t58, t59, t60, t61, t62, t63;
	MP3INT t64, t65, t66, t67, t68, t69, t70, t71;
	MP3INT t72, t73, t74, t75, t76, t77, t78, t79;
	MP3INT t80, t81, t82, t83, t84, t85, t86, t87;
	MP3INT t88, t89, t90, t91, t92, t93, t94, t95;
	MP3INT t96, t97, t98, t99, t100, t101, t102, t103;
	MP3INT t104, t105, t106, t107, t108, t109, t110, t111;
	MP3INT t112, t113, t114, t115, t116, t117, t118, t119;
	MP3INT t120, t121, t122, t123, t124, t125, t126, t127;
	MP3INT t128, t129, t130, t131, t132, t133, t134, t135;
	MP3INT t136, t137, t138, t139, t140, t141, t142, t143;
	MP3INT t144, t145, t146, t147, t148, t149, t150, t151;
	MP3INT t152, t153, t154, t155, t156, t157, t158, t159;
	MP3INT t160, t161, t162, t163, t164, t165, t166, t167;
	MP3INT t168, t169, t170, t171, t172, t173, t174, t175;
	MP3INT t176;
	/* COSTAB[i] = cos(PI / (2 * 32) * i) */

#  define COSTAB1	DB_INT(0x0ffb10f2)  /* 0.998795456 */
#  define COSTAB2	DB_INT(0x0fec46d2)  /* 0.995184727 */
#  define COSTAB3	DB_INT(0x0fd3aac0)  /* 0.989176510 */
#  define COSTAB4	DB_INT(0x0fb14be8)  /* 0.980785280 */
#  define COSTAB5	DB_INT(0x0f853f7e)  /* 0.970031253 */
#  define COSTAB6	DB_INT(0x0f4fa0ab)  /* 0.956940336 */
#  define COSTAB7	DB_INT(0x0f109082)  /* 0.941544065 */
#  define COSTAB8	DB_INT(0x0ec835e8)  /* 0.923879533 */
#  define COSTAB9	DB_INT(0x0e76bd7a)  /* 0.903989293 */
#  define COSTAB10	DB_INT(0x0e1c5979)  /* 0.881921264 */
#  define COSTAB11	DB_INT(0x0db941a3)  /* 0.857728610 */
#  define COSTAB12	DB_INT(0x0d4db315)  /* 0.831469612 */
#  define COSTAB13	DB_INT(0x0cd9f024)  /* 0.803207531 */
#  define COSTAB14	DB_INT(0x0c5e4036)  /* 0.773010453 */
#  define COSTAB15	DB_INT(0x0bdaef91)  /* 0.740951125 */
#  define COSTAB16	DB_INT(0x0b504f33)  /* 0.707106781 */
#  define COSTAB17	DB_INT(0x0abeb49a)  /* 0.671558955 */
#  define COSTAB18	DB_INT(0x0a267993)  /* 0.634393284 */
#  define COSTAB19	DB_INT(0x0987fbfe)  /* 0.595699304 */
#  define COSTAB20	DB_INT(0x08e39d9d)  /* 0.555570233 */
#  define COSTAB21	DB_INT(0x0839c3cd)  /* 0.514102744 */
#  define COSTAB22	DB_INT(0x078ad74e)  /* 0.471396737 */
#  define COSTAB23	DB_INT(0x06d74402)  /* 0.427555093 */
#  define COSTAB24	DB_INT(0x061f78aa)  /* 0.382683432 */
#  define COSTAB25	DB_INT(0x0563e69d)  /* 0.336889853 */
#  define COSTAB26	DB_INT(0x04a5018c)  /* 0.290284677 */
#  define COSTAB27	DB_INT(0x03e33f2f)  /* 0.242980180 */
#  define COSTAB28	DB_INT(0x031f1708)  /* 0.195090322 */
#  define COSTAB29	DB_INT(0x0259020e)  /* 0.146730474 */
#  define COSTAB30	DB_INT(0x01917a6c)  /* 0.098017140 */
#  define COSTAB31	DB_INT(0x00c8fb30)  /* 0.049067674 */

	t0 = in[0] + in[31];
	t16 = DCT_MUL(in[0] - in[31], COSTAB1);
	t1 = in[15] + in[16];
	t17 = DCT_MUL(in[15] - in[16], COSTAB31);

	t41 = t16 + t17;
	t59 = DCT_MUL(t16 - t17, COSTAB2);
	t33 = t0 + t1;
	t50 = DCT_MUL(t0 - t1, COSTAB2);

	t2 = in[7] + in[24];
	t18 = DCT_MUL(in[7] - in[24], COSTAB15);
	t3 = in[8] + in[23];
	t19 = DCT_MUL(in[8] - in[23], COSTAB17);

	t42 = t18 + t19;
	t60 = DCT_MUL(t18 - t19, COSTAB30);
	t34 = t2 + t3;
	t51 = DCT_MUL(t2 - t3, COSTAB30);

	t4 = in[3] + in[28];
	t20 = DCT_MUL(in[3] - in[28], COSTAB7);
	t5 = in[12] + in[19];
	t21 = DCT_MUL(in[12] - in[19], COSTAB25);

	t43 = t20 + t21;
	t61 = DCT_MUL(t20 - t21, COSTAB14);
	t35 = t4 + t5;
	t52 = DCT_MUL(t4 - t5, COSTAB14);

	t6 = in[4] + in[27];
	t22 = DCT_MUL(in[4] - in[27], COSTAB9);
	t7 = in[11] + in[20];
	t23 = DCT_MUL(in[11] - in[20], COSTAB23);

	t44 = t22 + t23;
	t62 = DCT_MUL(t22 - t23, COSTAB18);
	t36 = t6 + t7;
	t53 = DCT_MUL(t6 - t7, COSTAB18);

	t8 = in[1] + in[30];
	t24 = DCT_MUL(in[1] - in[30], COSTAB3);
	t9 = in[14] + in[17];
	t25 = DCT_MUL(in[14] - in[17], COSTAB29);

	t45 = t24 + t25;
	t63 = DCT_MUL(t24 - t25, COSTAB6);
	t37 = t8 + t9;
	t54 = DCT_MUL(t8 - t9, COSTAB6);

	t10 = in[6] + in[25];
	t26 = DCT_MUL(in[6] - in[25], COSTAB13);
	t11 = in[9] + in[22];
	t27 = DCT_MUL(in[9] - in[22], COSTAB19);

	t46 = t26 + t27;
	t64 = DCT_MUL(t26 - t27, COSTAB26);
	t38 = t10 + t11;
	t55 = DCT_MUL(t10 - t11, COSTAB26);

	t12 = in[2] + in[29];
	t28 = DCT_MUL(in[2] - in[29], COSTAB5);
	t13 = in[13] + in[18];
	t29 = DCT_MUL(in[13] - in[18], COSTAB27);

	t47 = t28 + t29;
	t65 = DCT_MUL(t28 - t29, COSTAB10);
	t39 = t12 + t13;
	t56 = DCT_MUL(t12 - t13, COSTAB10);

	t14 = in[5] + in[26];
	t30 = DCT_MUL(in[5] - in[26], COSTAB11);
	t15 = in[10] + in[21];
	t31 = DCT_MUL(in[10] - in[21], COSTAB21);

	t48 = t30 + t31;
	t66 = DCT_MUL(t30 - t31, COSTAB22);
	t40 = t14 + t15;
	t57 = DCT_MUL(t14 - t15, COSTAB22);

	t69 = t33 + t34;
	t89 = DCT_MUL(t33 - t34, COSTAB4);
	t70 = t35 + t36;
	t90 = DCT_MUL(t35 - t36, COSTAB28);
	t71 = t37 + t38;
	t91 = DCT_MUL(t37 - t38, COSTAB12);
	t72 = t39 + t40;
	t92 = DCT_MUL(t39 - t40, COSTAB20);
	t73 = t41 + t42;
	t94 = DCT_MUL(t41 - t42, COSTAB4);
	t74 = t43 + t44;
	t95 = DCT_MUL(t43 - t44, COSTAB28);
	t75 = t45 + t46;
	t96 = DCT_MUL(t45 - t46, COSTAB12);
	t76 = t47 + t48;
	t97 = DCT_MUL(t47 - t48, COSTAB20);

	t78 = t50 + t51;
	t100 = DCT_MUL(t50 - t51, COSTAB4);
	t79 = t52 + t53;
	t101 = DCT_MUL(t52 - t53, COSTAB28);
	t80 = t54 + t55;
	t102 = DCT_MUL(t54 - t55, COSTAB12);
	t81 = t56 + t57;
	t103 = DCT_MUL(t56 - t57, COSTAB20);

	t83 = t59 + t60;
	t106 = DCT_MUL(t59 - t60, COSTAB4);
	t84 = t61 + t62;
	t107 = DCT_MUL(t61 - t62, COSTAB28);
	t85 = t63 + t64;
	t108 = DCT_MUL(t63 - t64, COSTAB12);
	t86 = t65 + t66;
	t109 = DCT_MUL(t65 - t66, COSTAB20);

	t113 = t69 + t70;
	t114 = t71 + t72;

	/*  0 */
	hi[15][slot] = DCT_SHIFT(t113 + t114);
	/* 16 */
	lo[ 0][slot] = DCT_SHIFT(DCT_MUL(t113 - t114, COSTAB16));

	t115 = t73 + t74;
	t116 = t75 + t76;

	t32 = t115 + t116;

	/*  1 */
	hi[14][slot] = DCT_SHIFT(t32);

	t118 = t78 + t79;
	t119 = t80 + t81;

	t58 = t118 + t119;

	/*  2 */
	hi[13][slot] = DCT_SHIFT(t58);

	t121 = t83 + t84;
	t122 = t85 + t86;

	t67 = t121 + t122;

	t49 = (t67 * 2) - t32;

	/*  3 */
	hi[12][slot] = DCT_SHIFT(t49);

	t125 = t89 + t90;
	t126 = t91 + t92;

	t93 = t125 + t126;

	/*  4 */
	hi[11][slot] = DCT_SHIFT(t93);

	t128 = t94 + t95;
	t129 = t96 + t97;

	t98 = t128 + t129;

	t68 = (t98 * 2) - t49;

	/*  5 */
	hi[10][slot] = DCT_SHIFT(t68);

	t132 = t100 + t101;
	t133 = t102 + t103;

	t104 = t132 + t133;

	t82 = (t104 * 2) - t58;

	/*  6 */
	hi[ 9][slot] = DCT_SHIFT(t82);

	t136 = t106 + t107;
	t137 = t108 + t109;

	t110 = t136 + t137;

	t87 = (t110 * 2) - t67;

	t77 = (t87 * 2) - t68;

	/*  7 */
	hi[ 8][slot] = DCT_SHIFT(t77);

	t141 = DCT_MUL(t69 - t70, COSTAB8);
	t142 = DCT_MUL(t71 - t72, COSTAB24);
	t143 = t141 + t142;

	/*  8 */
	hi[ 7][slot] = DCT_SHIFT(t143);
	/* 24 */
	lo[ 8][slot] =
	    DCT_SHIFT((DCT_MUL(t141 - t142, COSTAB16) * 2) - t143);

	t144 = DCT_MUL(t73 - t74, COSTAB8);
	t145 = DCT_MUL(t75 - t76, COSTAB24);
	t146 = t144 + t145;

	t88 = (t146 * 2) - t77;

	/*  9 */
	hi[ 6][slot] = DCT_SHIFT(t88);

	t148 = DCT_MUL(t78 - t79, COSTAB8);
	t149 = DCT_MUL(t80 - t81, COSTAB24);
	t150 = t148 + t149;

	t105 = (t150 * 2) - t82;

	/* 10 */
	hi[ 5][slot] = DCT_SHIFT(t105);

	t152 = DCT_MUL(t83 - t84, COSTAB8);
	t153 = DCT_MUL(t85 - t86, COSTAB24);
	t154 = t152 + t153;

	t111 = (t154 * 2) - t87;

	t99 = (t111 * 2) - t88;

	/* 11 */
	hi[ 4][slot] = DCT_SHIFT(t99);

	t157 = DCT_MUL(t89 - t90, COSTAB8);
	t158 = DCT_MUL(t91 - t92, COSTAB24);
	t159 = t157 + t158;

	t127 = (t159 * 2) - t93;

	/* 12 */
	hi[ 3][slot] = DCT_SHIFT(t127);

	t160 = (DCT_MUL(t125 - t126, COSTAB16) * 2) - t127;

	/* 20 */
	lo[ 4][slot] = DCT_SHIFT(t160);
	/* 28 */
	lo[12][slot] =
	    DCT_SHIFT((((DCT_MUL(t157 - t158, COSTAB16) * 2) - t159) * 2) - t160);

	t161 = DCT_MUL(t94 - t95, COSTAB8);
	t162 = DCT_MUL(t96 - t97, COSTAB24);
	t163 = t161 + t162;

	t130 = (t163 * 2) - t98;

	t112 = (t130 * 2) - t99;

	/* 13 */
	hi[ 2][slot] = DCT_SHIFT(t112);

	t164 = (DCT_MUL(t128 - t129, COSTAB16) * 2) - t130;

	t166 = DCT_MUL(t100 - t101, COSTAB8);
	t167 = DCT_MUL(t102 - t103, COSTAB24);
	t168 = t166 + t167;

	t134 = (t168 * 2) - t104;

	t120 = (t134 * 2) - t105;

	/* 14 */
	hi[ 1][slot] = DCT_SHIFT(t120);

	t135 = (DCT_MUL(t118 - t119, COSTAB16) * 2) - t120;

	/* 18 */
	lo[ 2][slot] = DCT_SHIFT(t135);

	t169 = (DCT_MUL(t132 - t133, COSTAB16) * 2) - t134;

	t151 = (t169 * 2) - t135;

	/* 22 */
	lo[ 6][slot] = DCT_SHIFT(t151);

	t170 = (((DCT_MUL(t148 - t149, COSTAB16) * 2) - t150) * 2) - t151;

	/* 26 */
	lo[10][slot] = DCT_SHIFT(t170);
	/* 30 */
	lo[14][slot] =
	    DCT_SHIFT((((((DCT_MUL(t166 - t167, COSTAB16) * 2) -
	              t168) * 2) - t169) * 2) - t170);

	t171 = DCT_MUL(t106 - t107, COSTAB8);
	t172 = DCT_MUL(t108 - t109, COSTAB24);
	t173 = t171 + t172;

	t138 = (t173 * 2) - t110;

	t123 = (t138 * 2) - t111;

	t139 = (DCT_MUL(t121 - t122, COSTAB16) * 2) - t123;

	t117 = (t123 * 2) - t112;

	/* 15 */
	hi[ 0][slot] = DCT_SHIFT(t117);

	t124 = (DCT_MUL(t115 - t116, COSTAB16) * 2) - t117;

	/* 17 */
	lo[ 1][slot] = DCT_SHIFT(t124);

	t131 = (t139 * 2) - t124;

	/* 19 */
	lo[ 3][slot] = DCT_SHIFT(t131);

	t140 = (t164 * 2) - t131;

	/* 21 */
	lo[ 5][slot] = DCT_SHIFT(t140);

	t174 = (DCT_MUL(t136 - t137, COSTAB16) * 2) - t138;

	t155 = (t174 * 2) - t139;

	t147 = (t155 * 2) - t140;

	/* 23 */
	lo[ 7][slot] = DCT_SHIFT(t147);

	t156 = (((DCT_MUL(t144 - t145, COSTAB16) * 2) - t146) * 2) - t147;

	/* 25 */
	lo[ 9][slot] = DCT_SHIFT(t156);

	t175 = (((DCT_MUL(t152 - t153, COSTAB16) * 2) - t154) * 2) - t155;

	t165 = (t175 * 2) - t156;

	/* 27 */
	lo[11][slot] = DCT_SHIFT(t165);

	t176 = (((((DCT_MUL(t161 - t162, COSTAB16) * 2) -
	           t163) * 2) - t164) * 2) - t165;

	/* 29 */
	lo[13][slot] = DCT_SHIFT(t176);
	/* 31 */
	lo[15][slot] =
	    DCT_SHIFT((((((((DCT_MUL(t171 - t172, COSTAB16) * 2) -
	                t173) * 2) - t174) * 2) - t175) * 2) - t176);

	/*
	 * Totals:
	 *  80 multiplies
	 *  80 additions
	 * 119 subtractions
	 *  49 shifts (not counting SSO)
	 */
}
#endif

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static inline short Mp3SynthScale(MP3INT sample)
{
	if(sample >= 32767)
		sample = 32767;
	else if(sample <= -32767)
		sample = - 32767;
	return (short)sample;
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3SynthFull(MP3_STREAM *stream, int nch, int ns)
{
	int phase, ch, s, sb, pe, po;
	MP3INT (*filter)[2][2][16][8];
	MP3INT *sbsample;
	MP3INT (*fe)[8], (*fx)[8], (*fo)[8];
	MP3INT const (*Dptr)[32];
	MP3INT *ptr0;
	MP3INT const *ptr1;
	short *pcm1, *pcm2;
	MP3INT lo;

	for (ch = 0; ch < nch; ++ch)
	{
		sbsample = stream->SbSample[ch];
		filter = &stream->Synth.Filter[ch];
		phase = stream->Synth.Phase;
		pcm1 = stream->Synth.Pcm[ch];

		for (s = 0; s < ns; ++s)
		{
#if defined(CONFIG_ARCH_XBURST)	&& !defined(WIN32)	
			MxuMp3SynthDct32(&sbsample[s*32], phase >> 1,
				(*filter)[0][phase & 1], (*filter)[1][phase & 1]);

			pe = phase & ~1;
			po = ((phase - 1) & 0xf) | 1;

			/* calculate 32 samples */

			fe = &(*filter)[0][ phase & 1][0];
			fx = &(*filter)[0][~phase & 1][0];
			fo = &(*filter)[1][~phase & 1][0];

			Dptr = &Mp3SynthFullDW[0];

			ptr0 = *fx;
			ptr1 = *Dptr + po;
			lo = MxuMp3SynthMac0(0, ptr0, ptr1);

			ptr0 = *fe;
			ptr1 = *Dptr + pe;
			lo = MxuMp3SynthMac0(-lo, ptr0, ptr1);

			*pcm1++ = Mp3SynthScale((MLZ(hi, lo)));

			pcm2 = pcm1 + 30;

			for (sb = 1; sb < 16; ++sb)
			{
				++fe;
				++Dptr;

				/* D[32 - sb][i] == -D[sb][31 - i] */
				ptr0 = *fo;
				ptr1 = *Dptr + po;
				lo = MxuMp3SynthMac0(0, ptr0, ptr1);
                
                ptr0 = *fe;
				ptr1 = *Dptr + pe;
				lo = MxuMp3SynthMac0(-lo, ptr0, ptr1);

				*pcm1++ = Mp3SynthScale((MLZ(hi, lo)));
				
				ptr0 = *fe;
				ptr1 = *Dptr - pe;
				lo = MxuMp3SynthMac1(0, ptr0, ptr1);
				
				ptr0 = *fo;
				ptr1 = *Dptr - po;
				lo = MxuMp3SynthMac1(lo, ptr0, ptr1);

				*pcm2-- = Mp3SynthScale((MLZ(hi, lo)));

				++fo;
			}

			++Dptr;
			
			ptr0 = *fo;
			ptr1 = *Dptr + po;
			lo = MxuMp3SynthMac0(0, ptr0, ptr1);

			*pcm1 = Mp3SynthScale(-(MLZ(hi, lo)));
			pcm1 += 16;

			phase = (phase + 1) % 16;
#else
			Mp3SynthDct32(&sbsample[s*32], phase >> 1,
				(*filter)[0][phase & 1], (*filter)[1][phase & 1]);

			pe = phase & ~1;
			po = ((phase - 1) & 0xf) | 1;

			/* calculate 32 samples */

			fe = &(*filter)[0][ phase & 1][0];
			fx = &(*filter)[0][~phase & 1][0];
			fo = &(*filter)[1][~phase & 1][0];

			Dptr = &Mp3SynthFullDW[0];

			ptr0 = *fx;
			ptr1 = *Dptr + po;
			ML0(hi, lo, ptr0[0], ptr1[ 0]);
			MLA(hi, lo, ptr0[1], ptr1[14]);
			MLA(hi, lo, ptr0[2], ptr1[12]);
			MLA(hi, lo, ptr0[3], ptr1[10]);
			MLA(hi, lo, ptr0[4], ptr1[ 8]);
			MLA(hi, lo, ptr0[5], ptr1[ 6]);
			MLA(hi, lo, ptr0[6], ptr1[ 4]);
			MLA(hi, lo, ptr0[7], ptr1[ 2]);
			MLN(hi, lo);

			ptr0 = *fe;
			ptr1 = *Dptr + pe;
			MLA(hi, lo, ptr0[0], ptr1[ 0]);
			MLA(hi, lo, ptr0[1], ptr1[14]);
			MLA(hi, lo, ptr0[2], ptr1[12]);
			MLA(hi, lo, ptr0[3], ptr1[10]);
			MLA(hi, lo, ptr0[4], ptr1[ 8]);
			MLA(hi, lo, ptr0[5], ptr1[ 6]);
			MLA(hi, lo, ptr0[6], ptr1[ 4]);
			MLA(hi, lo, ptr0[7], ptr1[ 2]);

			*pcm1++ = Mp3SynthScale((MLZ(hi, lo)));

			pcm2 = pcm1 + 30;

			for (sb = 1; sb < 16; ++sb)
			{
				++fe;
				++Dptr;

				/* D[32 - sb][i] == -D[sb][31 - i] */
				ptr0 = *fo;
				ptr1 = *Dptr + po;
				ML0(hi, lo, ptr0[0], ptr1[ 0]);
				MLA(hi, lo, ptr0[1], ptr1[14]);
				MLA(hi, lo, ptr0[2], ptr1[12]);
				MLA(hi, lo, ptr0[3], ptr1[10]);
				MLA(hi, lo, ptr0[4], ptr1[ 8]);
				MLA(hi, lo, ptr0[5], ptr1[ 6]);
				MLA(hi, lo, ptr0[6], ptr1[ 4]);
				MLA(hi, lo, ptr0[7], ptr1[ 2]);
				MLN(hi, lo);
                
                ptr0 = *fe;
				ptr1 = *Dptr + pe;
				MLA(hi, lo, ptr0[0], ptr1[ 0]);
				MLA(hi, lo, ptr0[1], ptr1[14]);
				MLA(hi, lo, ptr0[2], ptr1[12]);
				MLA(hi, lo, ptr0[3], ptr1[10]);
				MLA(hi, lo, ptr0[4], ptr1[ 8]);
				MLA(hi, lo, ptr0[5], ptr1[ 6]);
				MLA(hi, lo, ptr0[6], ptr1[ 4]);
				MLA(hi, lo, ptr0[7], ptr1[ 2]);

				*pcm1++ = Mp3SynthScale((MLZ(hi, lo)));
				
				ptr0 = *fe;
				ptr1 = *Dptr - pe;
				ML0(hi, lo, ptr0[0], ptr1[31 - 16]);
				MLA(hi, lo, ptr0[1], ptr1[31 - 14]);
				MLA(hi, lo, ptr0[2], ptr1[31 - 12]);
				MLA(hi, lo, ptr0[3], ptr1[31 - 10]);
				MLA(hi, lo, ptr0[4], ptr1[31 - 8]);
				MLA(hi, lo, ptr0[5], ptr1[31 - 6]);
				MLA(hi, lo, ptr0[6], ptr1[31 - 4]);
				MLA(hi, lo, ptr0[7], ptr1[31 - 2]);
				
				ptr0 = *fo;
				ptr1 = *Dptr - po;
				MLA(hi, lo, ptr0[0], ptr1[31 - 16]);
				MLA(hi, lo, ptr0[1], ptr1[31 - 14]);
				MLA(hi, lo, ptr0[2], ptr1[31 - 12]);
				MLA(hi, lo, ptr0[3], ptr1[31 - 10]);
				MLA(hi, lo, ptr0[4], ptr1[31 - 8]);
				MLA(hi, lo, ptr0[5], ptr1[31 - 6]);
				MLA(hi, lo, ptr0[6], ptr1[31 - 4]);
				MLA(hi, lo, ptr0[7], ptr1[31 - 2]);

				*pcm2-- = Mp3SynthScale((MLZ(hi, lo)));

				++fo;
			}

			++Dptr;
			
			ptr0 = *fo;
			ptr1 = *Dptr + po;
			ML0(hi, lo, ptr0[0], ptr1[ 0]);
			MLA(hi, lo, ptr0[1], ptr1[14]);
			MLA(hi, lo, ptr0[2], ptr1[12]);
			MLA(hi, lo, ptr0[3], ptr1[10]);
			MLA(hi, lo, ptr0[4], ptr1[ 8]);
			MLA(hi, lo, ptr0[5], ptr1[ 6]);
			MLA(hi, lo, ptr0[6], ptr1[ 4]);
			MLA(hi, lo, ptr0[7], ptr1[ 2]);

			*pcm1 = Mp3SynthScale(-(MLZ(hi, lo)));
			pcm1 += 16;

			phase = (phase + 1) % 16;
#endif				
		}
	}
}
 

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3SynthInit(MP3_STREAM *stream) 
{
#if defined(CONFIG_MCU_C33L27)
	if(stream->DecodeLines != MP3_DECODE_LINE)
	{
		return Mp3SynthHalfInit(stream);
	}
#endif
	return 0;
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3Synth(MP3_STREAM *stream) 
{
	short nch, ns;
	
	if(stream->DecodeLines != MP3_DECODE_LINE)
	{
		return Mp3SynthHalf(stream);
	}
	nch = (stream->Header.Mode == MP3_MODE_SINGLECHANNEL) ? 1 : 2;
	ns = (stream->Header.Mpeg == MP3_MPEG1) ? 36 : 18;
	stream->Synth.Channels = nch;
	stream->Synth.SampleRate = stream->Header.Samplerate;
	stream->Synth.Samples = ns << 5;

	Mp3SynthFull(stream, nch, ns);

	stream->Synth.Phase = (stream->Synth.Phase + ns) % 16;
	return 0;
}

#endif // CONFIG_DECODE_MP3_ENABLE

