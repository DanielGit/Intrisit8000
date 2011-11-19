/*****************************************************************************
 *
 * JZ4760 TCSM1 Space Seperate
 *
 * $Id: vc1_tcsm1.h,v 1.12 2011/03/05 06:40:45 xjyu Exp $
 *
 ****************************************************************************/

#ifndef __VC1_TCSM1_H__
#define __VC1_TCSM1_H__

#define TCSM1_BANK0 0xF4000000
#define TCSM1_BANK1 0xF4001000
#define TCSM1_BANK2 0xF4002000
#define TCSM1_BANK3 0xF4003000
#define TCSM1_BANK4 0xF4004000

#define TCSM1_PADDR(a)        ((((unsigned)(a)) & 0xFFFF) | 0x132C0000) 
#define TCSM1_VCADDR(a)       ((((unsigned)(a)) & 0xFFFF) | 0xB32C0000) 
#define TCSM1_VUCADDR(a)      ((((unsigned)(a)) & 0xFFFF) | 0xB32C0000) 

#define VC1_P1_MAIN (TCSM1_BANK0)

#define TCSM1_CMD_LEN           (8 << 2)
#define TCSM1_MBNUM_WP          (TCSM1_BANK4)
#define TCSM1_MBNUM_RP          (TCSM1_MBNUM_WP+4)
#define TCSM1_ADDR_RP           (TCSM1_MBNUM_RP+4)
#define TCSM1_DCORE_SHARE_ADDR  (TCSM1_ADDR_RP+4)
#define TCSM1_FIRST_MBLEN       (TCSM1_DCORE_SHARE_ADDR+4)

#define DFRM_BUF_LEN            (((sizeof(struct VC1_Frame_GlbARGs)+31)/32)*32)
#define TCSM1_DFRM_BUF          (TCSM1_MBNUM_WP+TCSM1_CMD_LEN)//must be cache align

#define TASK_BUF_LEN            ((sizeof(struct VC1_MB_DecARGs) + 3) & 0xFFFFFFFC)
#define TASK_BUF0               (TCSM1_DFRM_BUF+DFRM_BUF_LEN)
#define TASK_BUF1               (TASK_BUF0+TASK_BUF_LEN)
#define TASK_BUF2               (TASK_BUF1+TASK_BUF_LEN)

#define DOUT_Y_STRD             (16)
#define DOUT_C_STRD             (8)
#define VC1_DYBUF_LEN           (DOUT_Y_STRD*16)
#define VC1_DCBUF_LEN           (DOUT_Y_STRD* 8)
#define VC1_DBUF_LEN            (VC1_DYBUF_LEN + VC1_DCBUF_LEN)
/*mc previous recon buffer*/
#define DOUT_YBUF0              (TASK_BUF2+TASK_BUF_LEN)
#define DOUT_CBUF0              (DOUT_YBUF0 + VC1_DYBUF_LEN)

#define DOUT_YBUF1              (DOUT_CBUF0 + VC1_DCBUF_LEN)
#define DOUT_CBUF1              (DOUT_YBUF1 + VC1_DYBUF_LEN)

#define DOUT_YBUF2              (DOUT_CBUF1 + VC1_DCBUF_LEN)
#define DOUT_CBUF2              (DOUT_YBUF2 + VC1_DYBUF_LEN)
#define DOUT_CBUF3              (DOUT_CBUF2 + VC1_DCBUF_LEN)

#define EDGE_BUF_LEN            (256+128)
#define EDGE_YOUT_BUF           (DOUT_CBUF3+VC1_DCBUF_LEN)
#define EDGE_COUT_BUF           (EDGE_YOUT_BUF+256)

#define EDGE_YOUT_BUF1          (EDGE_YOUT_BUF+EDGE_BUF_LEN)
#define EDGE_COUT_BUF1          (EDGE_YOUT_BUF1+256)

#define VC1_NLEN                ((2*14) << 2)

#define TCSM1_MOTION_DHA0        (EDGE_YOUT_BUF1+EDGE_BUF_LEN)
#define TCSM1_MOTION_DSA0        (TCSM1_MOTION_DHA0 + VC1_NLEN)

#define TCSM1_MOTION_DHA1        (TCSM1_MOTION_DSA0+0x4)
#define TCSM1_MOTION_DSA1        (TCSM1_MOTION_DHA1 + VC1_NLEN)

#define DDMA_GP0_DES_CHAIN_LEN  ((2*4)<<2)
#define DDMA_GP0_DES_CHAIN      (TCSM1_MOTION_DSA1+0x4)
#define DDMA_GP1_DES_CHAIN_LEN  ((4*9)<<2)
#define DDMA_GP1_DES_CHAIN      (DDMA_GP0_DES_CHAIN+DDMA_GP0_DES_CHAIN_LEN)
#define DDMA_GP1_DES_CHAIN1     (DDMA_GP1_DES_CHAIN+DDMA_GP1_DES_CHAIN_LEN)

#define TCSM1_GP1_POLL_END      (DDMA_GP1_DES_CHAIN1 + DDMA_GP1_DES_CHAIN_LEN)
#define TCSM1_GP0_POLL_END      (TCSM1_GP1_POLL_END + 4)

#define ROTA_Y_OFFSET   (16 * 16)
#define ROTA_C_OFFSET   (16 * 8)
#define ROTA_Y_BUF (TCSM1_GP0_POLL_END + 4)
#define ROTA_C_BUF (ROTA_Y_BUF + ROTA_Y_OFFSET)

#define ROTA_Y_BUF1 (ROTA_C_BUF + ROTA_C_OFFSET)
#define ROTA_C_BUF1 (ROTA_Y_BUF1 + ROTA_Y_OFFSET)

#define TCSM1_IDCT_BUF          (ROTA_C_BUF1 + ROTA_C_OFFSET)
#define TCSM1_IDCT_BUF_LEN      (384<<1) 
#define TCSM1_PMON_BUF_LEN      (18 << 2)
#define TCSM1_PMON_BUF          (TCSM1_IDCT_BUF+TCSM1_IDCT_BUF_LEN)

#define EDGE_WIDTH  32
#endif
