#define JZC_4760
//#define JZC_H264_SHRINK_OPT

#define __tcsm0_text0__ __attribute__ ((__section__ (".tcsm0_text0")))
#define __cache_text0__ __attribute__ ((__section__ (".cache_text0")))
#define __cache_text1__ __attribute__ ((__section__ (".cache_text1")))

#define H264_MAX_ROW_MBNUM (80)//1280

#define JZC_DCORE_OPT

#define JZC_GP2DDR_OPT
#define JZC_DCORE_SYN_DEBUG
//#define JZC_DBG_WAIT
//#define MOT_HW_EVA
#define DDMA_GP2_DIS_CHN_NOD_NUM (96+4+1)//12
#define SRAM_GP2_CHN_NOD_NUM (96 + 4 + 1 - DDMA_GP2_DIS_CHN_NOD_NUM  ) 

//#define JZC_PMON_P0
//#define STA_CCLK
//#define STA_DCC
//#define JZC_PMON_P1

#if defined(JZC_PMON_P0) && defined(JZC_PMON_P1)
#error JZC_PMON_P1 and JZC_PMON_P1 can define only one 
#endif

#ifdef JZC_PMON_P0
#if defined(STA_INSN)
# define PMON_P0_FILE_NAME "jz4760e_pmon_p0.insn"
#elif defined(STA_UINSN)
# define PMON_P0_FILE_NAME "jz4760e_pmon_p0.insn"
#elif defined(STA_CCLK)
# define PMON_P0_FILE_NAME "jz4760e_pmon_p0.cclk"
#elif defined(STA_DCC)
# define PMON_P0_FILE_NAME "jz4760e_pmon_p0.cc"
#elif defined(STA_ICC)
# define PMON_P0_FILE_NAME "jz4760e_pmon_p0.cc"
#elif defined(STA_TLB)
# define PMON_P0_FILE_NAME "jz4760e_pmon_p0.tlb"
#else
# error "If JZC_PMON_P0 defined, one of STA_INSN/STA_CCLK/STA_DCC/STA_ICC must be defined!"
#endif
#endif // JZC_PMON_P0

#define JZC_PRE_FILL_CACHES
//#define DECODING_SHUT_DOWN_INTERRUPT
//#define P0_INI_OPT 
//#define GP0_PIPE_EVA
//#define GP0_EVA_OPT0
/* -------------  JZ Media PMON define ------------------*/


/* -------------  JZ Media HW MACRO define ------------------*/
#define JZC_TEST_OPT
#define JZC_MC_OPT
#define JZC_DBLKLI_OPT
#ifdef JZC_DBLKLI_OPT
#define JZC_ROTA90_OPT
#endif

#define JZC_CABAC_HW_OPT

/* -------------  JZ Media BS MACRO define ------------------*/
//#define JZC_CABAC_OPT
//#define JZC_CAVLC_OPT


#ifdef JZC_DCORE_OPT
#ifndef JZC_CABAC_HW_OPT
# error "If JZC_DCORE_OPT defined, JZC_CABAC_HW_OPT should be defined, or there will be no residual for P1 !!"
#endif
#else
#ifdef JZC_PRE_FILL_CACHES
# error "If JZC_DCORE_OPT NOT defined, JZC_PRE_FILL_CACHES should NOT be defined !!"
#endif
#endif
