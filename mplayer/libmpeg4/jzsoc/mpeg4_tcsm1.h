/*****************************************************************************
 *
 * JZ4765 TCSM1 Space Seperate
 *
 * $Id: mpeg4_tcsm1.h,v 1.8 2010/11/18 03:27:28 hpwang Exp $
 *
 ****************************************************************************/

#ifndef __MPEG4_TCSM1_H__
#define __MPEG4_TCSM1_H__

#define TCSM1_BANK0 0xF4000000
#define TCSM1_BANK1 0xF4001000
#define TCSM1_BANK2 0xF4002000
#define TCSM1_BANK3 0xF4003000
#define TCSM1_BANK4 0xF4004000

#define TCSM1_PADDR(a)        ((((unsigned)a) & 0xFFFF) | 0x132C0000) 
#define TCSM1_VCADDR(a)       ((((unsigned)a) & 0xFFFF) | 0xB32C0000) 
#define TCSM1_VUCADDR(a)      ((((unsigned)a) & 0xFFFF) | 0xB32C0000) 

#define TCSM1_CMD_LEN           (8 << 2)
#define TCSM1_MBNUM_WP          (0xF4000200)
#define TCSM1_MBNUM_RP          (TCSM1_MBNUM_WP+4)
#define TCSM1_ADDR_RP           (TCSM1_MBNUM_RP+4)
#define TCSM1_DCORE_SHARE_ADDR  (TCSM1_ADDR_RP+4)
#define TCSM1_FIRST_MBLEN       (TCSM1_DCORE_SHARE_ADDR+4)

#define DFRM_BUF_LEN            (((sizeof(MPEG4_Frame_GlbARGs)+31)/32)*32)
#define TCSM1_DFRM_BUF          (TCSM1_MBNUM_WP+TCSM1_CMD_LEN)//must be cache align

#define TASK_BUF_LEN            ((sizeof(MPEG4_MB_DecARGs) + 3) & 0xFFFFFFFC)
#define TASK_BUF0               (TCSM1_DFRM_BUF+DFRM_BUF_LEN)
#define TASK_BUF1               (TASK_BUF0+TASK_BUF_LEN)
#define TASK_BUF2               (TASK_BUF1+TASK_BUF_LEN)

//those used by i-frame only version, so inherit it.
#define RECON_BUF_STRD   (16)
#define RECON_BUF_LEN    (24 * 16)
#define TCSM1_RECON_BUF (TASK_BUF2 + TASK_BUF_LEN)

#define DOUT_Y_STRD 16
#define DOUT_C_STRD 8
#define RECON_YBUF0    (TASK_BUF2+TASK_BUF_LEN)
#define RECON_UBUF0    (RECON_YBUF0 + 256)

#define RECON_YBUF1    (RECON_UBUF0 + 256)
#define RECON_UBUF1    (RECON_YBUF1 + 256)

#define RECON_BUF_USE  (RECON_UBUF1+256)

#define MPEG4_NLEN             (12*8)
#define TCSM1_MOTION_DHA (RECON_BUF_USE+4)
#define TCSM1_MOTION_DSA (TCSM1_MOTION_DHA + MPEG4_NLEN)
#define DDMA_GP0_DES_CHAIN_LEN  (4<<3)
#define DDMA_GP0_DES_CHAIN      (TCSM1_MOTION_DSA+0x4)
#define DDMA_GP1_DES_CHAIN_LEN  (160)
#define DDMA_GP1_DES_CHAIN      (DDMA_GP0_DES_CHAIN+DDMA_GP0_DES_CHAIN_LEN)

#define EDGE_BUF_LEN (384)
#define EDGE_YOUT_BUF (DDMA_GP1_DES_CHAIN+DDMA_GP1_DES_CHAIN_LEN)
#define EDGE_COUT_BUF (EDGE_YOUT_BUF+256)

#define RECON_UBUF2 (EDGE_YOUT_BUF+EDGE_BUF_LEN)

#define TCSM1_P0_POLL (RECON_UBUF2+256)
#define TCSM1_P1_POLL (TCSM1_P0_POLL + 4)

#define ROTA_Y_OFFSET   (16 * 16)
#define ROTA_C_OFFSET   (16 * 8)
#define ROTA_Y_BUF  (TCSM1_P0_POLL + 32)
#define ROTA_C_BUF  (ROTA_Y_BUF + ROTA_Y_OFFSET)
#define ROTA_Y_BUF1 (ROTA_C_BUF + ROTA_C_OFFSET)
#define ROTA_C_BUF1 (ROTA_Y_BUF1 + ROTA_Y_OFFSET)

#define TCSM1_DBG_BUF   (ROTA_C_BUF1 + ROTA_C_OFFSET)

#define EDGE_WIDTH  32

#define MPEG4_P1_MAIN (TCSM1_BANK4)
#endif
