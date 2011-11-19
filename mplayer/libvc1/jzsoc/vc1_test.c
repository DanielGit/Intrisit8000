/*******************************************************
 Motion Test Center
 ******************************************************/
#define __place_k0_data__
#undef printf
#undef fprintf
#include "t_motion.h"
#include "t_intpid.h"
#include "t_vputlb.h"
#include "vc1_tcsm.h"
short mpFrame;
#define ROA_ALN     256
//#define DOUT_Y_STRD 16
//#define DOUT_C_STRD 8

#define MOTION_BUF_VA    0xF4001000
#define TCSM0_V2P(a)     (((a) & 0xFFFF) | 0x132B0000)
//#define MOTION_BUF_SIZE  0x1000
volatile uint8_t *motion_buf, *motion_dha, *motion_dsa, *motion_douty, *motion_doutc, *motion_iwta;
volatile int total_tran, total_work;
volatile int tlb_i;

uint8_t  its_scale;
uint16_t its_rnd_y;
uint16_t its_rnd_c;

FILE *err_fp;
int *gp0_chain_ptr;
#if 0 
void motion_init(int intpid, int cintpid)
{

  err_fp = fopen("err.log\n", "aw");
  int i;
  tlb_i = 0;
  for(i=0; i<8; i++)
    SET_VPU_TLB(i, 0, 0, 0, 0);
  
  /************* motion DOUT buffer alloc ***************/
#if 0
  motion_buf = (volatile uint8_t *)MOTION_BUF_VA;
  motion_dha = motion_buf;
  motion_dsa = motion_dha + 0x200;
  motion_douty = 0xF4003000;
  motion_doutc = motion_douty + 0x200;
  //#else
  motion_buf = (volatile uint8_t *)TCSM1_MOTION_DHA;
  motion_dha = motion_buf;
  motion_dsa = TCSM1_MOTION_DSA;
  motion_douty = RECON_YBUF0;
  motion_doutc = RECON_UBUF0;
#endif
  for(i=0; i<16; i++){
    SET_TAB1_ILUT(i,/*idx*/
		  IntpFMT[intpid][i].intp[1],/*intp2*/
		  IntpFMT[intpid][i].intp_pkg[1],/*intp2_pkg*/
		  IntpFMT[intpid][i].hldgl,/*hldgl*/
		  IntpFMT[intpid][i].avsdgl,/*avsdgl*/
		  IntpFMT[intpid][i].intp_dir[1],/*intp2_dir*/
		  IntpFMT[intpid][i].intp_rnd[1],/*intp2_rnd*/
		  IntpFMT[intpid][i].intp_sft[1],/*intp2_sft*/
		  IntpFMT[intpid][i].intp_sintp[1],/*sintp2*/
		  IntpFMT[intpid][i].intp_srnd[1],/*sintp2_rnd*/
		  IntpFMT[intpid][i].intp_sbias[1],/*sintp2_bias*/
		  IntpFMT[intpid][i].intp[0],/*intp1*/
		  IntpFMT[intpid][i].tap,/*tap*/
		  IntpFMT[intpid][i].intp_pkg[0],/*intp1_pkg*/
		  IntpFMT[intpid][i].intp_dir[0],/*intp1_dir*/
		  IntpFMT[intpid][i].intp_rnd[0],/*intp1_rnd*/
		  IntpFMT[intpid][i].intp_sft[0],/*intp1_sft*/
		  IntpFMT[intpid][i].intp_sintp[0],/*sintp1*/
		  IntpFMT[intpid][i].intp_srnd[0],/*sintp1_rnd*/
		  IntpFMT[intpid][i].intp_sbias[0]/*sintp1_bias*/
		  );
    SET_TAB1_CLUT(i,/*idx*/
		  IntpFMT[intpid][i].intp_coef[0][7],/*coef8*/
		  IntpFMT[intpid][i].intp_coef[0][6],/*coef7*/
		  IntpFMT[intpid][i].intp_coef[0][5],/*coef6*/
		  IntpFMT[intpid][i].intp_coef[0][4],/*coef5*/
		  IntpFMT[intpid][i].intp_coef[0][3],/*coef4*/
		  IntpFMT[intpid][i].intp_coef[0][2],/*coef3*/
		  IntpFMT[intpid][i].intp_coef[0][1],/*coef2*/
		  IntpFMT[intpid][i].intp_coef[0][0] /*coef1*/
		  );
    SET_TAB1_CLUT(16+i,/*idx*/
		  IntpFMT[intpid][i].intp_coef[1][7],/*coef8*/
		  IntpFMT[intpid][i].intp_coef[1][6],/*coef7*/
		  IntpFMT[intpid][i].intp_coef[1][5],/*coef6*/
		  IntpFMT[intpid][i].intp_coef[1][4],/*coef5*/
		  IntpFMT[intpid][i].intp_coef[1][3],/*coef4*/
		  IntpFMT[intpid][i].intp_coef[1][2],/*coef3*/
		  IntpFMT[intpid][i].intp_coef[1][1],/*coef2*/
		  IntpFMT[intpid][i].intp_coef[1][0] /*coef1*/
		  );

    SET_TAB2_ILUT(i,/*idx*/
		  IntpFMT[cintpid][i].intp[1],/*intp2*/
		  IntpFMT[cintpid][i].intp_dir[1],/*intp2_dir*/
		  IntpFMT[cintpid][i].intp_sft[1],/*intp2_sft*/
		  IntpFMT[cintpid][i].intp_coef[1][0],/*intp2_lcoef*/
		  IntpFMT[cintpid][i].intp_coef[1][1],/*intp2_rcoef*/
		  IntpFMT[cintpid][i].intp_rnd[1],/*intp2_rnd*/
		  IntpFMT[cintpid][i].intp[0],/*intp1*/
		  IntpFMT[cintpid][i].intp_dir[0],/*intp1_dir*/
		  IntpFMT[cintpid][i].intp_sft[0],/*intp1_sft*/
		  IntpFMT[cintpid][i].intp_coef[0][0],/*intp1_lcoef*/
		  IntpFMT[cintpid][i].intp_coef[0][1],/*intp1_rcoef*/
		  IntpFMT[cintpid][i].intp_rnd[0]/*intp1_rnd*/
		  );
  }

  SET_REG1_STAT(1,/*pfe*/
		1,/*lke*/
		1 /*tke*/);
  SET_REG2_STAT(1,/*pfe*/
		1,/*lke*/
		1 /*tke*/);
  SET_REG1_CTRL(0,/*esms*/
		0,/*esa*/
		0,/*esmd*/
		0,/*csf*/
		0,/*cara*/
		0,/*cae*/
		0,/*crpm*/
		0xF,/*pgc*/
		3, /*pri*/
		0,/*ckge*/
		0,/*ofa*/
		0,/*rot*/
		USE_TDD,/*ddm*/
		0,/*wm*/
		1,/*ccf*/
		0,/*csl*/
		0,/*rst*/
		1 /*en*/);
  
  SET_REG1_BINFO(AryFMT[intpid],/*ary*/
		 0,/*doe*/
		 0,/*expdy*/
		 0,/*expdx*/
		 0,/*ilmd*/
		 SubPel[intpid]-1,/*pel*/
		 0,/*fld*/
		 0,/*fldsel*/
		 0,/*boy*/
		 0,/*box*/
		 0,/*bh*/
		 0,/*bw*/
		 0/*pos*/);
  SET_REG2_BINFO(0,/*ary*/
		 0,/*doe*/
		 0,/*expdy*/
		 0,/*expdx*/
		 0,/*ilmd*/
		 0,/*pel*/
		 0,/*fld*/
		 0,/*fldsel*/
		 0,/*boy*/
		 0,/*box*/
		 0,/*bh*/
		 0,/*bw*/
		 0/*pos*/);
  //printf("aaaaaaaaaaaaa\n");
  //exit_player_with_rc();


}


void motion_config_vc1(VC1Context *v)
{
  int i, j;
  MpegEncContext * const s = &v->s;
  SET_REG1_CTRL(0,/*esms*/
		0,/*esa*/
		0,/*esmd*/
		0,/*csf*/
		0,/*cara*/
		0,/*cae*/
		0,/*crpm*/
		0xF,/*pgc*/
		0, /*pri*/
		0,/*ckge*/
		0,/*ofa*/
		0,/*rot*/
		USE_TDD,/*ddm*/
		0,/*wm*/
		1,/*ccf*/
		0,/*csl*/
		0,/*rst*/
		1 /*en*/);

  /******************* ROA setting ******************/
  int tile_y_ofst = 0;
  int tile_c_ofst = 0;

  SET_TAB1_RLUT(0, get_phy_addr((int)(s->last_picture.data[0])), 0, 0); 
  SET_TAB1_RLUT(16, get_phy_addr((int)(s->next_picture.data[0])), 0, 0); 
  SET_TAB2_RLUT(0, get_phy_addr((int)(s->last_picture.data[1])), 0, 0, 0, 0); 
  SET_TAB2_RLUT(16, get_phy_addr((int)(s->next_picture.data[1])), 0, 0, 0, 0);  

  SET_REG1_PINFO(0,/*rgr*/
		 0,/*its*/
		 6,/*its_sft*/
		 its_scale,/*its_scale*/
		 its_rnd_y/*its_rnd*/);
  SET_REG2_PINFO(0,/*rgr*/
		 0,/*its*/
		 6,/*its_sft*/
		 its_scale,/*its_scale*/
		 its_rnd_c/*its_rnd*/);

  SET_REG1_WINFO(0,/*wt*/
		 0, /*wtpd*/
		 IS_BIAVG,/*wtmd*/
		 1,/*biavg_rnd*/
		 0,/*wt_denom*/
		 0,/*wt_sft*/
		 0,/*wt_lcoef*/
		 0/*wt_rcoef*/);
  SET_REG1_WTRND(0);

  SET_REG2_WINFO1(0,/*wt*/
		  0, /*wtpd*/
		  IS_BIAVG,/*wtmd*/
		  1,/*biavg_rnd*/
		  0,/*wt_denom*/
		  0,/*wt_sft*/
		  0,/*wt_lcoef*/
		  0/*wt_rcoef*/);
  SET_REG2_WINFO2(0,/*wt_sft*/
		  0,/*wt_lcoef*/
		  0/*wt_rcoef*/);
  SET_REG2_WTRND(0, 0);

  //SET_REG1_DSTA(TCSM1_PADDR((int)motion_douty));
  SET_REG1_STRD(s->linesize/16,0,DOUT_Y_STRD);
  SET_REG1_GEOM(s->mb_height*16,s->mb_width*16);
  //SET_REG1_DSA(TCSM1_PADDR((int)motion_dsa));
    
  //SET_REG2_DSTA(TCSM1_PADDR((int)motion_doutc));
  SET_REG2_STRD(s->linesize/16,0,DOUT_C_STRD);
  //SET_REG2_DSA(TCSM1_PADDR((int)motion_dsa));
}
#endif

void motion_execute_vc1(VC1Context *v, int dir, int mbtype, 
			int *mv, int *pos, int uvmvx, int uvmvy,
			uint8_t *dest_y, uint8_t *dest_c)
{
  MpegEncContext * const s = &v->s;
  //printf("MBY[%d], MBX[%d]\n", dMB->mb_y, dMB->mb_x);
#if 0
  motion_buf = (volatile uint8_t *)MOTION_BUF_VA;
  motion_dha = motion_buf;
  motion_dsa = motion_dha + 0x200;
#else
  motion_buf = (volatile uint8_t *)TCSM1_MOTION_DHA;
  motion_dha = motion_buf;
  motion_dsa = TCSM1_MOTION_DSA;
#endif
  motion_douty = RECON_YBUF0;
  motion_doutc = RECON_UBUF0;

  SET_REG1_DSTA(TCSM1_PADDR((int)motion_douty));
  SET_REG2_DSTA(TCSM1_PADDR((int)motion_doutc));
  SET_REG1_DSA(TCSM1_PADDR((int)motion_dsa));
  SET_REG2_DSA(TCSM1_PADDR((int)motion_dsa));  

  volatile int *tdd = TCSM1_VUCADDR((int *)motion_dha);
  int tkn = 0;
  volatile uint8_t *ddsa = TCSM1_VUCADDR((int *)motion_dsa);
  ddsa[0] = 0x0;
  SET_REG1_BINFO(0,0,0,0,s->mspel? (IS_ILUT0): (IS_ILUT1),0,0,0,0,0,0,0,0);
  if(mbtype == 0){
    //printf("1111111111\n");
    tdd[0] = TDD_HEAD(1,/*vld*/
		      1,/*lk*/
		      0,/*sync*/
		      SubPel[VC1_QPEL]-1,/*ch1pel*/
		      SubPel[VC1_QPEL]-1,/*ch2pel*/ 
		      TDD_POS_SPEC,/*posmd*/
		      TDD_MV_SPEC,/*mvmd*/ 
		      1,/*ch2en*/
		      2,/*tkn*/
		      dMB->mb_y,/*mby*/
		      dMB->mb_x/*mbx*/);

    tdd[1] = TDD_MV(mv[0]>>16, mv[0]);
    tdd[2] = TDD_CMD(0,/*bidir*/
		     dir,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     v->rangeredfrm,/*rgr*/
		     (v->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
		     1,/*doe*/
		     1,/*cflo*/
		     0,/*ypos*/
		     s->mspel? (IS_ILUT0): (IS_ILUT1 | (1-v->rnd)),/*lilmd*/
		     0,/*cilmd*/
		     0,/*list*/
		     0,/*boy*/
		     0,/*box*/
		     BLK_H16,/*bh*/
		     BLK_W16,/*bw*/
		     pos[0]/*xpos*/);
    tdd[3] = TDD_MV(uvmvy, uvmvx);
       
      {
    tdd[4] = TDD_CMD(0,/*bidir*/
		     dir,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     v->rangeredfrm,/*rgr*/
		     (v->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
		     1,/*doe*/
		     0,/*cflo*/
		     (uvmvy&3)<<1,/*ypos*/
		     0,/*lilmd*/
		     IS_EC,/*cilmd*/
		     0,/*list*/
		     0,/*boy*/
		     0,/*box*/
		     BLK_H16,/*bh*/
		     BLK_W16,/*bw*/
		     (uvmvx&3)<<1/*xpos*/);
    }
    tdd[5] = TDD_HEAD(1,/*vld*/
		      0,/*lk*/
		      1,/*sync*/
		      0,/*ch1pel*/
		      0,/*ch2pel*/ 
		      TDD_POS_SPEC,/*posmd*/
		      TDD_MV_SPEC,/*mvmd*/ 
		      1,/*ch2en*/
		      0,/*tkn*/
		      0xFF,/*mby*/
		      0xFF/*mbx*/);
  } else if(mbtype == 1){
    
    tdd[0] = TDD_HEAD(1,/*vld*/
		      1,/*lk*/
		      0,/*sync*/
		      0,/*ch1pel*/
		      0,/*ch2pel*/ 
		      TDD_POS_SPEC,/*posmd*/
		      TDD_MV_SPEC,/*mvmd*/ 
		      1,/*ch2en*/
		      0,/*tkn*/
		      0,/*mby*/
		      0/*mbx*/);
    tdd[2] = TDD_MV(mv[0]>>16, mv[0]);
    tdd[3] = TDD_CMD(0,/*bidir*/
		     dir,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     v->rangeredfrm,/*rgr*/
		     (v->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
		     1,/*doe*/
		     0,/*cflo*/
		     0,/*ypos*/
		     s->mspel? (IS_ILUT0): (IS_ILUT1 | (1-v->rnd)),/*lilmd*/
		     0,/*cilmd*/
		     0,/*list*/
		     0,/*boy*/
		     0,/*box*/
		     BLK_H8,/*bh*/
		     BLK_W8,/*bw*/
		     pos[0]/*xpos*/);

    tdd[1] = TDD_HEAD(1,/*vld*/
		      1,/*lk*/
		      0,/*sync*/
		      SubPel[VC1_QPEL]-1,/*ch1pel*/
		      SubPel[VC1_QPEL]-1,/*ch2pel*/ 
		      TDD_POS_SPEC,/*posmd*/
		      TDD_MV_SPEC,/*mvmd*/ 
		      1,/*ch2en*/
		      1,/*tkn*/
		      dMB->mb_y,/*mby*/
		      dMB->mb_x/*mbx*/);

    tdd[4] = TDD_HEAD(1,/*vld*/
		      0,/*lk*/
		      1,/*sync*/
		      0,/*ch1pel*/
		      0,/*ch2pel*/ 
		      TDD_POS_SPEC,/*posmd*/
		      TDD_MV_SPEC,/*mvmd*/ 
		      1,/*ch2en*/
		      0,/*tkn*/
		      0xFF,/*mby*/
		      0xFF/*mbx*/);
    
  } else if(mbtype == 2){
  }


  CLR_PMON1_INTP();
  CLR_PMON2_INTP();
  CLR_PMON1_TRAN();
  CLR_PMON2_TRAN();
  CLR_PMON1_WORK();
  CLR_PMON2_WORK();
  CLR_PMON1_IMISS();
  CLR_PMON2_IMISS();
  CLR_PMON1_PMISS();
  CLR_PMON2_PMISS();

  write_reg( (MOTION_V_BASE+REG1_MBPOS), 0x0000fffc);


  SET_REG1_DDC(TCSM1_PADDR((int)motion_dha) | 0x1);

  int error_i=0;

  //printf("aaaaaaa%9x\n",*((volatile int *)TCSM1_VUCADDR((motion_dsa))));  
  while(*((volatile int *)(ddsa)) != (0x80000000 | 0xFFFF) ){

    //printf("++++++++%9x\n",*((volatile int *)TCSM1_VUCADDR((motion_dsa))));  

  };

  int i,j;
  uint8_t *y1,*u1;
  y1 = TCSM1_VUCADDR(motion_douty);
  u1 = TCSM1_VUCADDR(motion_doutc);

  for(j=0; j<16; j++){
    for(i=0; i<16; i++){
      // printf("aaaaaaaaaa\n");
      //printf("\n");
    }
  }
#if 0
  int aaa[100];
  for(j=0,i=0; j<24; j++,i+=4){
    aaa[j] = read_reg(0xB3250000,i);
    printf("%d = %x\n",i , aaa[j]);
  }

  if(!aaa[25]){
    aaa[25] = read_reg(0xb3250000,0x100);
    printf("%x\n",aaa[25]);
  }
  if(!aaa[26]){
    aaa[26] = read_reg(0xb3250000,0x300);
    printf("%x\n",aaa[26]);
  }
  if(!aaa[27]){
    aaa[27] = read_reg(0xb3250000,0x400);
    printf("%x\n",aaa[27]);
  }
  if(!aaa[28]){
    aaa[28] = read_reg(0xb3250000,0x500);
    printf("%x\n",aaa[28]);
  }
  if(!aaa[29]){
    aaa[29] = read_reg(0xb3250000,0x900);
    printf("%x\n",aaa[29]);
  }
  if(!aaa[30]){
    aaa[30] = read_reg(0xb3250000,0xb00);
    printf("%x\n",aaa[30]);
  }
  if(!aaa[31]){
    aaa[31] = read_reg(0xb3250000,0xd00);
    printf("%x\n",aaa[31]);
  }

#endif


#if 0
  printf("\n[motion y]:\n");
  for(j=0; j<16; j++){
    for(i=0; i<16; i++)
      printf("  %02x", y1[j*16+i]);
    printf("\n");
  }
  
  printf("\n[motion c]:\n");
  for(j=0; j<8; j++){
    for(i=0; i<8; i++)
      printf("  %02x", u1[j*8+i]);
    printf("  |");
    for(i=0; i<8; i++)
      printf("  %02x", u1[8*8+j*8+i]);
    printf("\n");
  }
#endif
#if 1
  for(j=0; j<16; j++)
    for(i=0; i<16; i++)
      dest_y[j*16+i] = y1[j*16+i];


  for(j=0; j<8; j++)
    for(i=0; i<8; i++){
      dest_c[j*16 + i] = u1[j*8+i];
      dest_c[8 + j*16 + i] = u1[8*8+j*8+i];
    }
#endif

  //if((dMB->mb_x==9) && (dMB->mb_y==0)){
#if 1
  if((mpFrame==2) && (dMB->mb_x==0) && (dMB->mb_y==0)){
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    for(i=0; i<=5; i++)
      printf("TDD[%d]\t%08x\n", i, tdd[i]);

    for(i=0; i<tlb_i; i++)
      printf("VPU_TLB[%d]: %08x\n", i, GET_VPU_TLB(i));
    printf("CTRL: %08x\n", GET_REG1_CTRL());
    printf("STAT: %08x\n", GET_REG1_STAT());
    printf("MBPOS: %08x\n", GET_REG1_MBPOS());
    printf("REFA: %08x\n", GET_REG1_REFA());
    
    int tt = GET_REG1_REFA();
    printf(" %08x\n", tt + dMB->mb_y*16*s->linesize + dMB->mb_x*16*16);

    printf("DSTA: %08x\n", GET_REG1_DSTA());
    printf("PINFO: %08x\n", GET_REG1_PINFO());
    printf("BINFO: %08x\n", GET_REG1_BINFO());
    printf("IINFO1: %08x\n", GET_REG1_IINFO1());
    printf("IINFO2: %08x\n", GET_REG1_IINFO2());
    printf("WINFO: %08x\n", GET_REG1_WINFO());
    printf("WTRND: %08x\n", GET_REG1_WTRND());
    printf("TAP1L: %08x\n", GET_REG1_TAP1L());
    printf("TAP1M: %08x\n", GET_REG1_TAP1M());
    printf("TAP2L: %08x\n", GET_REG1_TAP2L());
    printf("TAP2M: %08x\n", GET_REG1_TAP2M());
    printf("STRD: %08x\n", GET_REG1_STRD());
    printf("GEOM: %08x\n", GET_REG1_GEOM());
    printf("DDC: %08x\n", GET_REG1_DDC());
    printf("DSA: %08x\n\n", GET_REG1_DSA());

    printf("STAT: %08x\n", GET_REG2_STAT());
    printf("MBPOS: %08x\n", GET_REG2_MBPOS());
    printf("REFA: %08x\n", GET_REG2_REFA());
    printf("DSTA: %08x\n", GET_REG2_DSTA());
    printf("PINFO: %08x\n", GET_REG2_PINFO());
    printf("BINFO: %08x\n", GET_REG2_BINFO());
    printf("IINFO1: %08x\n", GET_REG2_IINFO1());
    printf("IINFO2: %08x\n", GET_REG2_IINFO2());
    printf("WINFO1: %08x\n", GET_REG2_WINFO1());
    printf("WINFO2: %08x\n", GET_REG2_WINFO2());
    printf("WTRND: %08x\n", GET_REG2_WTRND());
    printf("STRD: %08x\n", GET_REG2_STRD());
    printf("DDC: %08x\n", GET_REG2_DDC());
    printf("DSA: %08x\n\n", GET_REG2_DSA());
    
    printf("INTP1: %d\n", GET_PMON1_INTP());
    printf("INTP2: %d\n", GET_PMON2_INTP());
    printf("WORK1: %d\n", GET_PMON1_WORK());
    printf("WORK2: %d\n", GET_PMON2_WORK());
    printf("IMISS1: %d\n", GET_PMON1_IMISS());
    printf("IMISS2: %d\n", GET_PMON2_IMISS());
    printf("PMISS1: %d\n", GET_PMON1_PMISS());
    printf("PMISS2: %d\n", GET_PMON2_PMISS());
    printf("TRAN1: %d\n", GET_PMON1_TRAN());
    printf("TRAN2: %d\n", GET_PMON2_TRAN());

    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    //    fprintf(err_fp,"Error: frame: %d, mbx: %d, mby: %d\n", mpFrame, s->mb_x, s->mb_y);

    exit_player(1);
  }
#endif

  //  printf("motion verify\n");
  //  int i, j;
#if 0
  int fail_y=0, fail_c=0;
	
  for(j=0; j<16; j++)
    for(i=0; i<16; i++)
      if(motion_douty[j*16+i] != dest_y[j*s->linesize+i])
	fail_y++;

  for(j=0; j<8; j++)
    for(i=0; i<8; i++)
      if((motion_doutc[j*8+i]     != dest_cb[j*s->uvlinesize+i]) ||
	 (motion_doutc[8*8+j*8+i] != dest_cr[j*s->uvlinesize+i]) )
	fail_c++;
  
  //  printf("s->mb_y = %d , s->mb_x = %d\n", s->mb_y, s->mb_x);

  if(fail_y || fail_c){
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("motion verify failed @(MBY[%d], MBX[%d])\n\n", s->mb_y, s->mb_x);
  }
  if(fail_y){
    printf("[std dest_y]:\n");
    for(j=0; j<16; j++){
      for(i=0; i<16; i++)
	printf("  %02x", dest_y[j*s->linesize+i]);
      printf("\n");
    }
    printf("\n[motion y]:\n");
    for(j=0; j<16; j++){
      for(i=0; i<16; i++)
	printf("  %02x", motion_douty[j*16+i]);
      printf("\n");
    }
  }

  if(fail_c){
    printf("[std dest_c]:\n");
    for(j=0; j<8; j++){
      for(i=0; i<8; i++)
	printf("  %02x", dest_cb[j*s->uvlinesize+i]);
      printf("  |");
      for(i=0; i<8; i++)
	printf("  %02x", dest_cr[j*s->uvlinesize+i]);
      printf("\n");
    }
    printf("\n[motion c]:\n");
    for(j=0; j<8; j++){
      for(i=0; i<8; i++)
	printf("  %02x", motion_doutc[j*8+i]);
      printf("  |");
      for(i=0; i<8; i++)
	printf("  %02x", motion_doutc[8*8+j*8+i]);
      printf("\n");
    }
  }
 #endif 
  if(0){
    //  if((mpFrame==10) && (s->mb_x==11) && (s->mb_y==17)){
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    for(i=0; i<=5; i++)
      printf("TDD[%d]\t%08x\n", i, tdd[i]);

    for(i=0; i<tlb_i; i++)
      printf("VPU_TLB[%d]: %08x\n", i, GET_VPU_TLB(i));
    printf("CTRL: %08x\n", GET_REG1_CTRL());
    printf("STAT: %08x\n", GET_REG1_STAT());
    printf("MBPOS: %08x\n", GET_REG1_MBPOS());
    printf("REFA: %08x\n", GET_REG1_REFA());

    int tt = GET_REG1_REFA();
    printf(" %08x\n", tt + s->mb_y*16*s->linesize + s->mb_x*16*16);

    printf("DSTA: %08x\n", GET_REG1_DSTA());
    printf("PINFO: %08x\n", GET_REG1_PINFO());
    printf("BINFO: %08x\n", GET_REG1_BINFO());
    printf("IINFO1: %08x\n", GET_REG1_IINFO1());
    printf("IINFO2: %08x\n", GET_REG1_IINFO2());
    printf("WINFO: %08x\n", GET_REG1_WINFO());
    printf("WTRND: %08x\n", GET_REG1_WTRND());
    printf("TAP1L: %08x\n", GET_REG1_TAP1L());
    printf("TAP1M: %08x\n", GET_REG1_TAP1M());
    printf("TAP2L: %08x\n", GET_REG1_TAP2L());
    printf("TAP2M: %08x\n", GET_REG1_TAP2M());
    printf("STRD: %08x\n", GET_REG1_STRD());
    printf("GEOM: %08x\n", GET_REG1_GEOM());
    printf("DDC: %08x\n", GET_REG1_DDC());
    printf("DSA: %08x\n\n", GET_REG1_DSA());

    printf("STAT: %08x\n", GET_REG2_STAT());
    printf("MBPOS: %08x\n", GET_REG2_MBPOS());
    printf("REFA: %08x\n", GET_REG2_REFA());
    printf("DSTA: %08x\n", GET_REG2_DSTA());
    printf("PINFO: %08x\n", GET_REG2_PINFO());
    printf("BINFO: %08x\n", GET_REG2_BINFO());
    printf("IINFO1: %08x\n", GET_REG2_IINFO1());
    printf("IINFO2: %08x\n", GET_REG2_IINFO2());
    printf("WINFO1: %08x\n", GET_REG2_WINFO1());
    printf("WINFO2: %08x\n", GET_REG2_WINFO2());
    printf("WTRND: %08x\n", GET_REG2_WTRND());
    printf("STRD: %08x\n", GET_REG2_STRD());
    printf("DDC: %08x\n", GET_REG2_DDC());
    printf("DSA: %08x\n\n", GET_REG2_DSA());
    
    printf("INTP1: %d\n", GET_PMON1_INTP());
    printf("INTP2: %d\n", GET_PMON2_INTP());
    printf("WORK1: %d\n", GET_PMON1_WORK());
    printf("WORK2: %d\n", GET_PMON2_WORK());
    printf("IMISS1: %d\n", GET_PMON1_IMISS());
    printf("IMISS2: %d\n", GET_PMON2_IMISS());
    printf("PMISS1: %d\n", GET_PMON1_PMISS());
    printf("PMISS2: %d\n", GET_PMON2_PMISS());
    printf("TRAN1: %d\n", GET_PMON1_TRAN());
    printf("TRAN2: %d\n", GET_PMON2_TRAN());

    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    //    fprintf(err_fp,"Error: frame: %d, mbx: %d, mby: %d\n", mpFrame, s->mb_x, s->mb_y);

    exit_player(1);
  }
  //////////////////////////////////////////////////////////////////////
  total_tran += GET_PMON1_TRAN() + GET_PMON2_TRAN();
  total_work += (GET_PMON1_WORK()>GET_PMON2_WORK())? GET_PMON1_WORK() : GET_PMON2_WORK();
}


void motion_execute_vc1_4mv(VC1Context *v, int n,int *tdd, int *tkn, int dir, int mbtype, 
			int *mv, int *pos, int uvmvx, int uvmvy,
			uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr)
{
  MpegEncContext * const s = &v->s;
  //  printf("MBY[%d], MBX[%d]\n", s->mb_y, s->mb_x);
  //volatile int *tdd = (int *)motion_dha;
  //int tkn = 0;
  //motion_dsa[0] = 0x0;
  SET_REG1_BINFO(0,0,0,0,s->mspel? (IS_ILUT0): (IS_ILUT1),0,0,0,0,0,0,0,0);
  if(mbtype == 1){
    printf("mbtype 1\n");
    printf("tdd[2*tkn[0] = %x\n",tdd);
    tdd[2*tkn[0]] = TDD_MV(mv[0]>>16, mv[0]);
    tdd[2*tkn[0]+1] = TDD_CMD(0,/*bidir*/
		     dir,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     v->rangeredfrm,/*rgr*/
		     (v->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
		     1,/*doe*/
		     0,/*cflo*/
		     0,/*ypos*/
		     s->mspel? (IS_ILUT0): (IS_ILUT1),/*lilmd*/
		     0,/*cilmd*/
		     0,/*list*/
		     (n&0x2),/*boy*/
		     (n&0x1)*2,/*box*/
		     BLK_H8,/*bh*/
		     BLK_W8,/*bw*/
		     pos[0]/*xpos*/);
    
  } else if(mbtype == 2){
  }
  (*tkn)++;
}

