/*****************************************************************************
 *
 * JZ4760 TCSM1 Space Seperate
 *
 * $Id: rv9_tcsm1.h,v 1.15 2010/12/29 02:18:25 xqliang Exp $
 *
 ****************************************************************************/

#ifndef __RV9_TCSM1_H__
#define __RV9_TCSM1_H__

#define TCSM1_BANK0 0xF4000000
#define TCSM1_BANK1 0xF4001000
#define TCSM1_BANK2 0xF4002000
#define TCSM1_BANK3 0xF4003000

#define TCSM1_BANK4 0xF4004000
#define TCSM1_BANK5 0xF4005000
#define TCSM1_BANK6 0xF4006000
#define TCSM1_BANK7 0xF4007000

#define TCSM1_PADDR(a)        ((((unsigned)(a)) & 0xFFFF) | 0x132C0000) 
#define TCSM1_VCADDR(a)       ((((unsigned)(a)) & 0xFFFF) | 0xB32C0000) 
#define TCSM1_VUCADDR(a)      ((((unsigned)(a)) & 0xFFFF) | 0xB32C0000) 
#define TCSM1_INNER_ADDR(a)   ((((unsigned)(a)) & 0xFFFF) | 0xF4000000) 

#define P1_MAIN_ADDR (TCSM1_BANK0)

#define TCSM1_SLICE_BUF0 (TCSM1_BANK4 + 0x600)
#define TCSM1_SLICE_BUF1 (TCSM1_SLICE_BUF0+(SLICE_T_CC_LINE*32))

#define TCSM1_CMD_LEN           (16 << 2)
#define TCSM1_MBNUM_WP          (TCSM1_SLICE_BUF1+(SLICE_T_CC_LINE*32))
#define TCSM1_MBNUM_RP          (TCSM1_MBNUM_WP+4)
#define TCSM1_ADDR_RP           (TCSM1_MBNUM_RP+4)
#define TCSM1_DCORE_SHARE_ADDR  (TCSM1_ADDR_RP+4)
#define TCSM1_FIRST_MBLEN       (TCSM1_DCORE_SHARE_ADDR+4)
#define TCSM1_DBG_RESERVE       (TCSM1_FIRST_MBLEN+4)
#define TCSM1_P1_STOP           TCSM1_DBG_RESERVE       
#define TCSM1_BUG_W             (TCSM1_P1_STOP + 4)
#define TCSM1_BUG_H             (TCSM1_BUG_W + 4)
#define TCSM1_BUG_TRIG          (TCSM1_BUG_H + 4)

#define P1_T_BUF_LEN (sizeof(struct RV9_XCH2_T))

#define TCSM1_XCH2_T_BUF0 (TCSM1_MBNUM_WP + TCSM1_CMD_LEN)
#define TCSM1_XCH2_T_BUF1 (TCSM1_XCH2_T_BUF0+P1_T_BUF_LEN)

//#define TASK_BUF_LEN            (sizeof(struct RV9_MB_Ctrl_DecARGs)+sizeof(struct RV9_MB_InterB_DecARGs)+(296<<2)+(68<<1))//7+53+296+34 words
#define TASK_BUF_LEN            (sizeof(struct RV9_MB_DecARGs))

#define TASK_BUF0               (TCSM1_XCH2_T_BUF1+P1_T_BUF_LEN)
#define TASK_BUF1               (TASK_BUF0+TASK_BUF_LEN)
#define TASK_BUF2               (TASK_BUF1+TASK_BUF_LEN)

#define PREVIOUS_LUMA_STRIDE    (4+16)
#define PREVIOUS_CHROMA_STRIDE  (4+8)
#define PREVIOUS_C_LEN          (PREVIOUS_CHROMA_STRIDE<<3)
#define PREVIOUS_OFFSET_U       ((PREVIOUS_LUMA_STRIDE<<4))
#define PREVIOUS_OFFSET_V       (PREVIOUS_OFFSET_U+(PREVIOUS_CHROMA_STRIDE<<3))
#define DBLK_LEFT_Y0            (TASK_BUF2+TASK_BUF_LEN)
#define RECON_PREVIOUS_YBUF0    (DBLK_LEFT_Y0+4)
#define DBLK_LEFT_U0            (DBLK_LEFT_Y0+(PREVIOUS_LUMA_STRIDE<<4))
#define RECON_PREVIOUS_UBUF0    (DBLK_LEFT_U0+4)
#define DBLK_LEFT_V0            (DBLK_LEFT_U0+(PREVIOUS_CHROMA_STRIDE<<3))
#define RECON_PREVIOUS_VBUF0    (DBLK_LEFT_V0+4)

#define DBLK_LEFT_Y1            (DBLK_LEFT_V0+(PREVIOUS_CHROMA_STRIDE<<3))
#define RECON_PREVIOUS_YBUF1    (DBLK_LEFT_Y1+4)
#define DBLK_LEFT_U1            (DBLK_LEFT_Y1+(PREVIOUS_LUMA_STRIDE<<4))
#define RECON_PREVIOUS_UBUF1    (DBLK_LEFT_U1+4)
#define DBLK_LEFT_V1            (DBLK_LEFT_U1+(PREVIOUS_CHROMA_STRIDE<<3))
#define RECON_PREVIOUS_VBUF1    (DBLK_LEFT_V1+4)

#define DBLK_LEFT_Y2            (DBLK_LEFT_V1+(PREVIOUS_CHROMA_STRIDE<<3))
#define RECON_PREVIOUS_YBUF2    (DBLK_LEFT_Y2+4)
#define DBLK_LEFT_U2            (DBLK_LEFT_Y2+(PREVIOUS_LUMA_STRIDE<<4))
#define RECON_PREVIOUS_UBUF2    (DBLK_LEFT_U2+4)
#define DBLK_LEFT_V2            (DBLK_LEFT_U2+(PREVIOUS_CHROMA_STRIDE<<3))
#define RECON_PREVIOUS_VBUF2    (DBLK_LEFT_V2+4)

#if 0
#define DOUT_Y_STRD             (4+16)
#define DOUT_C_STRD             (4+8)

#define RV9_DYBUF_LEN           (DOUT_Y_STRD*16)
#define RV9_DCBUF_LEN           (DOUT_Y_STRD* 8)

#define RECON_YBUF0             (DBLK_LEFT_V2+(PREVIOUS_CHROMA_STRIDE<<3)+4)
#define RECON_CBUF0             (RECON_YBUF0 + RV9_DYBUF_LEN)

#define RECON_YBUF1             (RECON_CBUF0 + RV9_DCBUF_LEN)
#define RECON_CBUF1             (RECON_YBUF1 + RV9_DYBUF_LEN)

#endif

#define RV9_NLEN                (24 << 2)
#define TCSM1_MOTION_DHA        (DBLK_LEFT_V2+(PREVIOUS_CHROMA_STRIDE<<3)+4)
#define TCSM1_MOTION_DSA        (TCSM1_MOTION_DHA + RV9_NLEN)

#define IDCT_DES_CHAIN_LEN      (5<<2)
#define IDCT_DES_CHAIN          (TCSM1_MOTION_DSA+4)
#define IDCT_DES_CHAIN1         (IDCT_DES_CHAIN+IDCT_DES_CHAIN_LEN)

#define DBLK_END_FLAG           (IDCT_DES_CHAIN1+IDCT_DES_CHAIN_LEN)
#define DBLK_DES_CHAIN_LEN      (19<<2)
#define DBLK_DES_CHAIN0         (DBLK_END_FLAG+4)
#define DBLK_DES_CHAIN1         (DBLK_DES_CHAIN0+DBLK_DES_CHAIN_LEN)
#define DBLK_SYN_VALUE          (0xa5a5)

#define TCSM1_ACCUM_NUM                   (1)
#define TCSM1_DBLK_UPOUT_STRD_Y           ((TCSM1_ACCUM_NUM<<4))//ping-pong buffer
#define TCSM1_DBLK_UPOUT_STRD_C           ((TCSM1_ACCUM_NUM<<3)<<1)
#define TCSM1_DBLK_UPOUT_UV_OFFSET        (8)
#define TCSM1_DBLK_UPOUT_Y0               (DBLK_DES_CHAIN1+DBLK_DES_CHAIN_LEN)
#define TCSM1_DBLK_UPOUT_Y1               (TCSM1_DBLK_UPOUT_Y0+64)
#define TCSM1_DBLK_UPOUT_U0               (TCSM1_DBLK_UPOUT_Y1+64)
#define TCSM1_DBLK_UPOUT_V0               (TCSM1_DBLK_UPOUT_U0+8)
#define TCSM1_DBLK_UPOUT_U1               (TCSM1_DBLK_UPOUT_U0+64)
#define TCSM1_DBLK_UPOUT_V1               (TCSM1_DBLK_UPOUT_U1+8)

#define TCSM1_DBLK_MBOUT_STRD_Y           (4+(TCSM1_ACCUM_NUM<<4))
#define TCSM1_DBLK_MBOUT_STRD_C           (4+(TCSM1_ACCUM_NUM<<3))
#define TCSM1_DBLK_MBOUT_UV_OFFSET        (TCSM1_DBLK_MBOUT_STRD_C<<3)
#define TCSM1_DBLK_MBOUT_Y0               (TCSM1_DBLK_UPOUT_U1+64)
#define TCSM1_DBLK_MBOUT_U0               (TCSM1_DBLK_MBOUT_Y0+(TCSM1_DBLK_MBOUT_STRD_Y<<4))
#define TCSM1_DBLK_MBOUT_V0               (TCSM1_DBLK_MBOUT_U0+(TCSM1_DBLK_MBOUT_STRD_C<<3))
#define TCSM1_DBLK_MBOUT_Y1               (TCSM1_DBLK_MBOUT_V0+(TCSM1_DBLK_MBOUT_STRD_C<<3))
#define TCSM1_DBLK_MBOUT_U1               (TCSM1_DBLK_MBOUT_Y1+(TCSM1_DBLK_MBOUT_STRD_Y<<4))
#define TCSM1_DBLK_MBOUT_V1               (TCSM1_DBLK_MBOUT_U1+(TCSM1_DBLK_MBOUT_STRD_C<<3))

//#define DDMA_GP0_DES_CHAIN_LEN  (4<<2)
#define DDMA_GP0_DES_CHAIN_LEN  (8<<2)
#define DDMA_GP0_DES_CHAIN      (TCSM1_DBLK_MBOUT_V1 + (TCSM1_DBLK_MBOUT_STRD_C<<3))

#define DDMA_GP1_DES_CHAIN_LEN  ((4*13)<<2)
#define DDMA_GP1_DES_CHAIN       (DDMA_GP0_DES_CHAIN + DDMA_GP0_DES_CHAIN_LEN)
#define DDMA_GP1_DES_CHAIN1      (DDMA_GP1_DES_CHAIN + DDMA_GP1_DES_CHAIN_LEN)

#define TCSM1_IDCT_BUF           (DDMA_GP1_DES_CHAIN1 + DDMA_GP1_DES_CHAIN_LEN)
#define TCSM1_IDCT_BUF1         (TCSM1_IDCT_BUF + (384<<2))

#define TCSM1_DBLK_MBOUT_DES_STRD_Y       (256)
#define TCSM1_DBLK_MBOUT_DES_STRD_C       (128)
#define TCSM1_DBLK_MBOUT_DES_OFFS_C       (8)

#define TCSM1_DBLK_MBOUT_YDES             (TCSM1_IDCT_BUF1 + (384<<2))
#define TCSM1_DBLK_MBOUT_UDES             (TCSM1_DBLK_MBOUT_YDES + TCSM1_DBLK_MBOUT_DES_STRD_Y)
#define TCSM1_DBLK_MBOUT_VDES             (TCSM1_DBLK_MBOUT_UDES + TCSM1_DBLK_MBOUT_DES_OFFS_C)

#define TCSM1_DBLK_MBOUT_YDES1             (TCSM1_DBLK_MBOUT_UDES + TCSM1_DBLK_MBOUT_DES_STRD_C)
#define TCSM1_DBLK_MBOUT_UDES1             (TCSM1_DBLK_MBOUT_YDES1 + TCSM1_DBLK_MBOUT_DES_STRD_Y)
#define TCSM1_DBLK_MBOUT_VDES1             (TCSM1_DBLK_MBOUT_UDES1 + TCSM1_DBLK_MBOUT_DES_OFFS_C)

#define TCSM1_DBLK_MBOUT_YDES2             (TCSM1_DBLK_MBOUT_UDES1 + TCSM1_DBLK_MBOUT_DES_STRD_C)
#define TCSM1_DBLK_MBOUT_UDES2             (TCSM1_DBLK_MBOUT_YDES2 + TCSM1_DBLK_MBOUT_DES_STRD_Y)
#define TCSM1_DBLK_MBOUT_VDES2             (TCSM1_DBLK_MBOUT_UDES2 + TCSM1_DBLK_MBOUT_DES_OFFS_C)

#define TCSM1_ROTA_MB_YBUF1                (TCSM1_DBLK_MBOUT_UDES2 + TCSM1_DBLK_MBOUT_DES_STRD_C)
#define TCSM1_ROTA_MB_YBUF2                (TCSM1_ROTA_MB_YBUF1 + 256)
#define TCSM1_ROTA_MB_YBUF3                (TCSM1_ROTA_MB_YBUF2 + 256)

#define TCSM1_ROTA_MB_CBUF1                (TCSM1_ROTA_MB_YBUF3 + 256)
#define TCSM1_ROTA_MB_CBUF2                (TCSM1_ROTA_MB_CBUF1 + 128)
#define TCSM1_ROTA_MB_CBUF3                (TCSM1_ROTA_MB_CBUF2 + 128)

#define TCSM1_EDGE_YBUF                    (TCSM1_ROTA_MB_CBUF3 + 128)
#define TCSM1_EDGE_CBUF                    (TCSM1_EDGE_YBUF + 256)
#define TCSM1_EDGE_YBUF1                   (TCSM1_EDGE_CBUF + 128)
#define TCSM1_EDGE_CBUF1                   (TCSM1_EDGE_YBUF1 + 256)

#define TCSM1_GP1_POLL_END                 (TCSM1_EDGE_CBUF1 + 128)
#define TCSM1_GP0_POLL_END                 (TCSM1_GP1_POLL_END + 4)


#define TCSM1_UP_EDGE_YBUF                 (TCSM1_GP0_POLL_END + 4)
#define TCSM1_UP_EDGE_YBUF1                (TCSM1_UP_EDGE_YBUF + 64)

#define TCSM1_UP_ROTA_YBUF1                (TCSM1_UP_EDGE_YBUF1 + 64)
#define TCSM1_UP_ROTA_YBUF2                (TCSM1_UP_ROTA_YBUF1 + 64)

#define TCSM1_MC_BUG_SPACE                 (TCSM1_UP_ROTA_YBUF2 + 64)
#define EDGE_WIDTH 32
#endif 

