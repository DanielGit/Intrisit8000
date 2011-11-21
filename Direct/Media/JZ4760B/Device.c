//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典模拟平台  *****
//        --------------------------------------
//    	           模拟平台按键处理部分
//        --------------------------------------
//                 版权: 新诺亚舟科技
//             ---------------------------
//                  版   本   历   史
//        --------------------------------------
//  版本    日前		说明		
//  V0.1    2005-8      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]

#include <kernel/kernel.h>
#include <kernel/irq.h>
#include <kernel/thread.h>
#include <kernel/device.h>
#include <kernel/karch.h>
#include <direct/media.h>
#include <platform/platform.h>

//设置声音播放的DMA通道
#define PLAYBACK_CHANNEL		6
#define RECORD_CHANNEL			7


//#define KPRINTF_DEF
#define PHYADDR(n)			(unsigned int)((n) & 0x1fffffff)
#define PHYSADDR(x)			(((unsigned int)x) & 0x1fffffff)

#define DEVICE_CLOSE_STATUS	0
#define DEVICE_OPEN_STATUS	1

extern void ClockDelay(DWORD usec);
extern void DacClearPcmData(void);
extern void AdcSetPcmData(void);

//#define MAX_TEST_DATA 2*1024*1024
//static WORD test_buf[MAX_TEST_DATA];
extern DWORD TimerCount(void);
static HANDLE hPlayer = NULL;
static HANDLE hRecord = NULL;
static unsigned int	g_volume = 0;
static void dma_start(unsigned int channel, unsigned int srcAddr, unsigned int dstAddr, unsigned int count,unsigned char mode);
static unsigned char codec_reg_read(unsigned char addr);
extern void DmaDataToAic(int arg,unsigned int sour_addr, unsigned int len,unsigned char mode);
extern void EnableMediaGpio(void);
extern void dma_cache_wback_inv(unsigned long addr, unsigned long size);
extern void __dcache_inv(unsigned long addr, unsigned long size);
static DWORD interrupt_count;
static BYTE  fDeviceStatus;
void GetDmaInfo();
////////////////////////////////////////////////////
// 功能: 延迟N个豪秒
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static void dma_init(unsigned int channel, char mode)
{
	unsigned int group, data;

	group = channel / HALF_DMA_NUM;

	REG_DMAC_DMACR(group) |= DMAC_DMACR_DMAE | DMAC_DMACR_FAIC; /* set DMA control register AIC fast DMA and enable DMA channel transfer */
	
	REG_DMAC_DMACKES(group) |= (1 << (channel % HALF_DMA_NUM)); /* open channel clock */

	REG_DMAC_DCCSR(channel) = DMAC_DCCSR_NDES; /* set no descriptor transfer */
	if(mode)
	{
		data = DMAC_DCMD_SAI | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_16 | DMAC_DCMD_DS_16BYTE | DMAC_DCMD_RDIL_IGN
				| DMAC_DCMD_TIE;
		/* playback channel source increment, source port width 32bit, target port width 16bit,data size of transfer 16Byte ,enable interrupt */
		REG_DMAC_DCMD(channel) = data;
		REG_DMAC_DRSR(channel) = DMAC_DRSR_RS_AICOUT;/* AIC out request */
	}
	else
	{
		data = DMAC_DCMD_DAI | DMAC_DCMD_SWDH_16 | DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS_16BYTE | DMAC_DCMD_RDIL_IGN
				| DMAC_DCMD_TIE;
		REG_DMAC_DCMD(channel) = data;
		REG_DMAC_DRSR(channel) = DMAC_DRSR_RS_AICIN;
	}
}

////////////////////////////////////////////////////
// 功能: 延迟N个豪秒
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static void dma_start(unsigned int channel, unsigned int srcAddr, unsigned int dstAddr, unsigned int count, unsigned char mode)
{
#ifdef KPRINTF_DEF
	kprintf("dma channle = %d\n",channel);
	kprintf("source = %x, destion = %x, count = %x\n",srcAddr,dstAddr,count*16);
#endif
//	REG_DMAC_DCCSR(channel) &= ~(DMAC_DCMD_TIE | DMAC_DCCSR_AR | DMAC_DCCSR_HLT | DMAC_DCCSR_EN | DMAC_DCCSR_CT);/* jz4760 not count terminated bit ignore for no descriptor transfter*/
	/* clear address error flag,clear halt flag,channel transfer disable */
	REG_DMAC_DSAR(channel) = srcAddr; /* set srource address*/
	REG_DMAC_DTAR(channel) = dstAddr; /* set target address */
	REG_DMAC_DTCR(channel) = (count + 15) / 16; /* set  size of transfer unit */
	REG_DMAC_DCCSR(channel) |= DMAC_DCCSR_EN; /* enable channel transfter */

	//判断是否允许DMA中断
	InterruptUnmask(EIRQ_DMA_BASE + channel, 0);		// 允许DMA结束后，自动产生中断
}

////////////////////////////////////////////////////
// 功能: 得到当前DMA传送的数据
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
DWORD GetDmaCount(void)
{
	return (REG_DMAC_DTCR(PLAYBACK_CHANNEL) & 0x0ffffff) * 16;
}

////////////////////////////////////////////////////
// 功能: 读取DMA状态
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void GetDmaInfo()
{
	kprintf("dma count = %d\n",GetDmaCount());
	kprintf("AIC_FR           [0x%x]\n", INREG32(AIC_FR ));
	kprintf("AIC_CR           [0x%x]\n", INREG32(AIC_CR ));
	kprintf("AIC_ACCR1        [0x%x]\n", INREG32(AIC_ACCR1 ));
	kprintf("AIC_ACCR2        [0x%x]\n", INREG32(AIC_ACCR2 ));
	kprintf("AIC_I2SCR        [0x%x]\n", INREG32(AIC_I2SCR ));
	kprintf("AIC_SR           [0x%x]\n", INREG32(AIC_SR ));
	kprintf("AIC_ACSR         [0x%x]\n", INREG32(AIC_ACSR ));
	kprintf("AIC_I2SSR        [0x%x]\n", INREG32(AIC_I2SSR ));
	kprintf("AIC_ACCAR        [0x%x]\n", INREG32(AIC_ACCAR ));
	kprintf("AIC_ACCDR        [0x%x]\n", INREG32(AIC_ACCDR ));
	kprintf("AIC_ACSAR        [0x%x]\n", INREG32(AIC_ACSAR ));
	kprintf("AIC_ACSDR        [0x%x]\n", INREG32(AIC_ACSDR ));
	kprintf("AIC_I2SDIV       [0x%x]\n", INREG32(AIC_I2SDIV ));
	kprintf("AIC_DR           [0x%x]\n", INREG32(AIC_DR ));

	kprintf("A_CODEC_SR       [0x%x]\n", codec_reg_read(CODEC_SR ));
	kprintf("A_CODEC_AICR     [0x%x]\n", codec_reg_read(CODEC_AICR ));
	kprintf("A_CODEC_CR1      [0x%x]\n", codec_reg_read(CODEC_CR1 ));
	kprintf("A_CODEC_CR2      [0x%x]\n", codec_reg_read(CODEC_CR2 ));
	kprintf("A_CODEC_CR3      [0x%x]\n", codec_reg_read(CODEC_CR3 ));
	kprintf("A_CODEC_CR4      [0x%x]\n", codec_reg_read(CODEC_CR4 ));
	kprintf("A_CODEC_CCR1     [0x%x]\n", codec_reg_read(CODEC_CCR1 ));
	kprintf("A_CODEC_CCR2     [0x%x]\n", codec_reg_read(CODEC_CCR2 ));
	kprintf("A_CODEC_PMR1     [0x%x]\n", codec_reg_read(CODEC_PMR1 ));
	kprintf("A_CODEC_PMR2     [0x%x]\n", codec_reg_read(CODEC_PMR2 ));
	kprintf("A_CODEC_ICR      [0x%x]\n", codec_reg_read(CODEC_ICR ));
	kprintf("A_CODEC_IFR      [0x%x]\n", codec_reg_read(CODEC_IFR ));
	kprintf("A_CODEC_CGR1     [0x%x]\n", codec_reg_read(CODEC_CGR1 ));
	kprintf("A_CODEC_CGR2     [0x%x]\n", codec_reg_read(CODEC_CGR2 ));
	kprintf("A_CODEC_CGR3     [0x%x]\n", codec_reg_read(CODEC_CGR3 ));
	kprintf("A_CODEC_CGR4     [0x%x]\n", codec_reg_read(CODEC_CGR4 ));
	kprintf("A_CODEC_CGR5     [0x%x]\n", codec_reg_read(CODEC_CGR5 ));
	kprintf("A_CODEC_CGR6     [0x%x]\n", codec_reg_read(CODEC_CGR6 ));
	kprintf("A_CODEC_CGR7     [0x%x]\n", codec_reg_read(CODEC_CGR7 ));
	kprintf("A_CODEC_CGR8     [0x%x]\n", codec_reg_read(CODEC_CGR8 ));
	kprintf("A_CODEC_CGR9     [0x%x]\n", codec_reg_read(CODEC_CGR9 ));
	kprintf("A_CODEC_AGC1     [0x%x]\n", codec_reg_read(CODEC_AGC1 ));
	kprintf("A_CODEC_AGC2     [0x%x]\n", codec_reg_read(CODEC_AGC2 ));
	kprintf("A_CODEC_AGC3     [0x%x]\n", codec_reg_read(CODEC_AGC3 ));
	kprintf("A_CODEC_AGC4     [0x%x]\n", codec_reg_read(CODEC_AGC4 ));
	kprintf("A_CODEC_AGC5     [0x%x]\n", codec_reg_read(CODEC_AGC5 ));
	kprintf("A_CODEC_MIX1     [0x%x]\n", codec_reg_read(CODEC_MIX1 ));
	kprintf("A_CODEC_MIX2     [0x%x]\n", codec_reg_read(CODEC_MIX2 ));
	kprintf("A_CODEC_TR1      [0x%x]\n", codec_reg_read(CODEC_TR1 ));
	kprintf("A_CODEC_TR2      [0x%x]\n", codec_reg_read(CODEC_TR2 ));
	kprintf("A_CODEC_TR3      [0x%x]\n", codec_reg_read(CODEC_TR3 ));
	kprintf("A_CODEC_TR4      [0x%x]\n", codec_reg_read(CODEC_TR4 ));
}

////////////////////////////////////////////////////
// 功能: 延迟N个豪秒
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void MillinsecoundDelay(unsigned int msec)
{
	unsigned int i;

	for (i = 0; i < msec; i++)
		ClockDelay(1000);
}

////////////////////////////////////////////////////
// 功能: codec 寄存器读出
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static unsigned char codec_reg_read(unsigned char addr)
{
	volatile int reg;
	AIC_RW_CODEC_START();
	OUTREG16(ICDC_RGADW, addr << ICDC_RGADW_RGADDR_BIT);
	reg = INREG8(ICDC_RGDATA);
	reg = INREG8(ICDC_RGDATA);
	reg = INREG8(ICDC_RGDATA);
	reg = INREG8(ICDC_RGDATA);
	reg = INREG8(ICDC_RGDATA);
	reg = INREG8(ICDC_RGDATA);
	return (INREG8(ICDC_RGDATA));
}

////////////////////////////////////////////////////
// 功能: codec 寄存器写入
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static void codec_reg_write(unsigned char addr, unsigned char data)
{
	unsigned int temp;
	volatile int reg;
	temp = (addr << ICDC_RGADW_RGADDR_BIT) | (data << ICDC_RGADW_RGDIN_BIT);
	AIC_RW_CODEC_START();
	OUTREG32(ICDC_RGADW, temp);
	OUTREG32(ICDC_RGADW, temp | ICDC_RGADW_RGWR);

	reg = INREG32(ICDC_RGADW);
	reg = INREG32(ICDC_RGADW);
	reg = INREG32(ICDC_RGADW);
	reg = INREG32(ICDC_RGADW);
	reg = INREG32(ICDC_RGADW);
	reg = INREG32(ICDC_RGADW);
	AIC_RW_CODEC_STOP();
}

////////////////////////////////////////////////////
// 功能: codec 寄存器设置
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static void codec_reg_set(unsigned char addr, unsigned char data)
{
	volatile int reg;
	reg = codec_reg_read(addr); /* read codec register value*/
	codec_reg_write(addr, reg | data); /*  set bit to codec register */
}

////////////////////////////////////////////////////
// 功能: codec 寄存器清除
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static void codec_reg_clear(unsigned char addr, unsigned char data)
{
	volatile int reg;
	reg = codec_reg_read(addr); /* read codec register value*/
	codec_reg_write(addr, reg & (~data));/* clear bit to codec register */
}

////////////////////////////////////////////////////
// 功能: 利用DMA播放数据
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static void set_volume_reg(unsigned char arg)
{
	unsigned char tmp;
	if (arg == 0)
	{
		codec_reg_write(CODEC_CGR1, GOL(31));
		codec_reg_write(CODEC_CGR2, GOR(31));
		codec_reg_write(CODEC_CGR5, GODR(15));
		codec_reg_write(CODEC_CGR6, GODL(15));
		codec_reg_set(CODEC_CR2, DAC_MUTE);		//DAC_MUTE
		return;
	}

	codec_reg_clear(CODEC_CR2, DAC_MUTE);		//DAC_MUTE

	if (arg >= 8)
	{
		tmp = 31 - (arg - 8);
		codec_reg_write(CODEC_CGR5, GODR(0));
		codec_reg_write(CODEC_CGR6, GODL(0));
		codec_reg_write(CODEC_CGR1, GOL(tmp));
		codec_reg_write(CODEC_CGR2, GOR(tmp));
	}
	else
	{
		tmp = 15 - arg * 2;
		codec_reg_write(CODEC_CGR5, GODR(tmp));
		codec_reg_write(CODEC_CGR6, GODL(tmp));
		codec_reg_write(CODEC_CGR1, GOL(31));
		codec_reg_write(CODEC_CGR2, GOR(31));
	}
}

////////////////////////////////////////////////////
// 功能: 利用DMA播放数据
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static unsigned char get_volume_reg(void)
{
	unsigned char cgr5 = codec_reg_read(CODEC_CGR5) & 0x0F;
	unsigned char cgr1 = codec_reg_read(CODEC_CGR1) & 0x1F;

	return (31 - cgr1 + ((15 - cgr5) >> 1));
}

////////////////////////////////////////////////////
// 功能: 利用DMA播放数据
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static int intr_handler_playback_dma(int arg)
{
	int group,channle;
	unsigned int stat;

	//清除DMA标记
#ifdef KPRINTF_DEF
	kprintf("play music interrupt happend: %d-%d\n",GetDmaCount(),TimerCount());
	kprintf("INTC_ICMR           [0x%x]\n", INREG32(INTC_ICMR(0)) );
	kprintf("INTC_ICSR           [0x%x]\n", INREG32(INTC_ICSR(0)) );
	kprintf("INTC_ICPR           [0x%x]\n", INREG32(INTC_ICPR(0)) );
	kprintf("DMAC_DMAIPR start   [0x%x]\n", INREG32(DMAC_DMAIPR(0)) );
#endif
	channle = arg - EIRQ_DMA_BASE;
	stat    = REG_DMAC_DCCSR(channle);
	group   = (arg - EIRQ_DMA_BASE) / HALF_DMA_NUM;


	__dmac_channel_ack_irq( channle );

#ifdef KPRINTF_DEF
 	kprintf("DMAC_DMAIPR end     [0x%x]\n", INREG32(DMAC_DMAIPR(0)) );
#endif

	if (stat & DCS_TT)
	{
		/* disable DMA channel tranfer , clear transfer terminate flag*/
		REG_DMAC_DCCSR(PLAYBACK_CHANNEL) &= ~(DMAC_DCCSR_EN | DMAC_DCCSR_CT | DMAC_DCCSR_TT);
		DacClearPcmData();		//清空PCM数据
		interrupt_count++;
	}
	else
		kdebug(mod_media, PRINT_ERROR, "stat & DCS_TT == 0\n");

	return INT_DONE;
}

////////////////////////////////////////////////////
// 功能: 音频初始化
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static int intr_handler_record_dma(int arg)
{
	unsigned int channle;
	unsigned int group = arg / HALF_DMA_NUM;

#ifdef KPRINTF_DEF
	kprintf("recorde interrupt happend\n");
#endif
	channle = arg - EIRQ_DMA_BASE;
	REG_DMAC_DMACR(group) &= ~(DMAC_DMACR_AR | DMAC_DMACR_HLT);
	REG_DMAC_DCCSR(channle) &= ~(DMAC_DCCSR_EN | DMAC_DCCSR_CT | DMAC_DCCSR_TT | DMAC_DCCSR_AR | DMAC_DCCSR_HLT);

	AdcSetPcmData();		//读出PCM数据
	return INT_DONE;
}

////////////////////////////////////////////////////
// 功能: 音频初始化
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static void jz_audio_reset(void)
{
	REG_AIC_CR &= ~(AIC_CR_ERPL | AIC_CR_EREC);
	REG_AIC_CR |= (AIC_CR_RFLUSH | AIC_CR_TFLUSH);
	REG_AIC_ACSR = 0x00000000;
}

////////////////////////////////////////////////////
// 功能: 音频初始化
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static inline void set_sample_rate(unsigned int rate)
{
	unsigned int val;

	if (rate == 44100)
		val = 2;
	else if (rate == 48000)
		val = 1;
	else if (rate == 32000)
		val = 3;
	else if (rate == 22050)
		val = 5;
	else if (rate == 11025)
		val = 8;
	else if (rate == 8000)
		val = 10;
	else if (rate == 96000)
		val = 0;
	else if (rate == 24000)
		val = 4;
	else if (rate == 16000)
		val = 6;
	else if (rate == 12000)
		val = 7;
	else if (rate == 9600)
		val = 9;
	else
	{
		kprintf("Invalid Sample Rate[%d], set to 8000\n", rate);
		val = 10;
	}
	codec_reg_write(CODEC_CCR2, DAC_FREQ(val) | ADC_FREQ(val));
}

////////////////////////////////////////////////////
// 功能: 音频初始化
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int pcm_ioctl(unsigned int cmd, unsigned long arg)
{
	unsigned int data, temp, aicfr, aiccr;
	unsigned char RegVolume = 0, VolDif = 0, JumpCoun = 0;

	switch (cmd)
	{
	case PCM_SET_SAMPLE_RATE:
		kprintf("pcm_ioctl, PCM_SET_SAMPLE_RATE,arg = %d\n", arg);
		set_sample_rate(arg);/* set sample rate */
		break;

	case PCM_SET_CHANNEL:
		kprintf("pcm_ioctl, PCM_SET_CHANNEL,arg = %d.\n", arg);
		if (arg == 2)/* set channel stereo*/
		{
			REG_AIC_CR &= ~AIC_CR_M2S;
			REG_AIC_CR |= AIC_CR_CHANNEL_STEREO;
		}
		else if (arg == 1) /* set channel mono */
		{
			REG_AIC_CR |= AIC_CR_M2S;
			REG_AIC_CR &= ~AIC_CR_CHANNEL_MASK;
		}
		break;

	case PCM_SET_FORMAT:
		kprintf("pcm_ioctl, PCM_SET_FORMAT, arg = %d.\n",arg);
		/*Config for Playback */
		aicfr = REG_AIC_FR & (~AIC_FR_TFTH_MASK);
		aiccr = REG_AIC_CR & (~AIC_CR_OSS_MASK);
		temp = REG_DMAC_DCMD(PLAYBACK_CHANNEL) & (~DMAC_DCMD_DWDH_MASK);
		if (arg == 16)
		{
			aicfr |= AIC_FR_TFTH(24);
			temp |= DMAC_DCMD_DWDH_16;
			aiccr &= ~AIC_CR_AVSTSU;
			aiccr |= AIC_CR_OSS_16BIT;
		}
		else if (arg == 8)
		{
			aicfr |= AIC_FR_TFTH(16);
			temp |= DMAC_DCMD_DWDH_8;
			aiccr |= AIC_CR_AVSTSU;
			aiccr |= AIC_CR_OSS_8BIT;
		}

		REG_AIC_FR = aicfr;
		REG_AIC_CR = aiccr;
		REG_DMAC_DCMD(PLAYBACK_CHANNEL) = temp;

		/* Config for Recorder */
		temp = REG_DMAC_DCMD(RECORD_CHANNEL) & (~DMAC_DCMD_DWDH_MASK);
		if (arg == 16)
			temp |= DMAC_DCMD_DWDH_16;
		else if (arg == 8)
			temp |= DMAC_DCMD_DWDH_8;

		REG_DMAC_DCMD(RECORD_CHANNEL) = temp;
		break;

	case PCM_SET_MUTE:
		kprintf("pcm_ioctl, PCM_SET_MUTE, arg = %d.\n", arg);
		if ((g_volume == 0) || arg == 1)
			codec_reg_set(CODEC_CR2, DAC_MUTE);/* set DAC_MUTE */
		else
			codec_reg_clear(CODEC_CR2, DAC_MUTE); /* clear DAC_MUTE */
		break;

	case PCM_SET_VOL:
	case PCM_SET_HP_VOL:
		kprintf("pcm_ioctl, PCM_SET_HP_VOL, val = %d.\n", arg);
		data = arg > 31 ? 31 : arg;
		g_volume = data;
		RegVolume = get_volume_reg();
		VolDif = RegVolume > data ? (RegVolume - data) : (data - RegVolume);
		if (VolDif > 7)
		{
			for (JumpCoun = 0; JumpCoun <= data; JumpCoun++)
			{
				set_volume_reg(JumpCoun);
			}
		}
		else
		{
			set_volume_reg(data);
		}
		break;

	case PCM_SET_PAUSE:
		kprintf("pcm_ioctl, PCM_SET_PAUSE\n");
		REG_DMAC_DCCSR(PLAYBACK_CHANNEL) &= ~(DMAC_DCCSR_AR | DMAC_DCCSR_HLT | DMAC_DCCSR_EN | DMAC_DCCSR_CT);/* reset playback DMA channel  */
		jz_audio_reset();/* reset audio */
		break;

	case PCM_SET_PLAY:
	case PCM_SET_REPLAY:
		kprintf("pcm_ioctl, PCM_SET_REPLAY\n");
		REG_DMAC_DCCSR(RECORD_CHANNEL) &= ~(DMAC_DCCSR_AR | DMAC_DCCSR_HLT | DMAC_DCCSR_EN | DMAC_DCCSR_CT);/* reset record DMA channel */
		jz_audio_reset(); /* audio reset */

		switch (arg)
		{
		case DAC_TO_HP:
			/* set dac to headphone */
			codec_reg_clear(CODEC_PMR1, SB); //SB
			codec_reg_clear(CODEC_PMR1, SB_SLEEP); //SB_SLEEP
			codec_reg_clear(CODEC_PMR2, SB_DAC); //SB_DAC
			codec_reg_clear(CODEC_PMR2, SB_HP); //SB_HP
			codec_reg_set(CODEC_PMR2, SB_LOUT); //SB_LOUT
			codec_reg_set(CODEC_PMR2, SB_BTL); //SB_BTL
			codec_reg_set(CODEC_CR1, OUTSEL(3)); //OUTSEL(3)
			codec_reg_clear(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
			codec_reg_clear(CODEC_CR2, DAC_MUTE); //DAC_MUTE
			codec_reg_clear(CODEC_CR1, HP_MUTE); //HP_MUTE
			codec_reg_set(CODEC_CR2, NOMAD); //NOMAD

			/* reduce power set */
			codec_reg_set(CODEC_PMR2, SB_ADC); //SB_ADC
			codec_reg_set(CODEC_PMR1, SB_MICBIAS); // SB_MICBIAS
			codec_reg_set(CODEC_PMR1, SB_MIC1); //SB_MIC1
			codec_reg_set(CODEC_PMR1, SB_MIC2); //SB_MIC2
			codec_reg_set(CODEC_PMR1, SB_LINE); //SB_LINE
			codec_reg_set(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
			codec_reg_set(CODEC_CR3, INSEL(3)); //INSEL(3)
			codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
			codec_reg_set(CODEC_CR4, ADC_R_ONLY); //ADC_R_ONLY
			codec_reg_set(CODEC_CR1, LINEOUT_MUTE); //LINEOUT_MUTE
			codec_reg_set(CODEC_CR1, BTL_MUTE); //BTL_MUTE
			break;

		case DAC_TO_LOUT:
			/* set dac to lineout */
			codec_reg_clear(CODEC_PMR1, SB); //SB
			codec_reg_clear(CODEC_PMR1, SB_SLEEP); //SB_SLEEP
			codec_reg_clear(CODEC_PMR2, SB_DAC); //SB_DAC
			codec_reg_clear(CODEC_PMR2, SB_HP); //SB_HP
			codec_reg_clear(CODEC_PMR2, SB_LOUT); //SB_LOUT
			codec_reg_set(CODEC_PMR2, SB_BTL); //SB_BTL
			codec_reg_set(CODEC_CR1, OUTSEL(3)); //OUTSEL(3)
			codec_reg_clear(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
			codec_reg_clear(CODEC_CR2, DAC_MUTE); //DAC_MUTE
			codec_reg_clear(CODEC_CR1, HP_MUTE); //HP_MUTE
			codec_reg_clear(CODEC_CR1, LINEOUT_MUTE);//LINEOUT_MUTE
			codec_reg_clear(CODEC_CR2, NOMAD); //NOMAD

			/* reduce power set */
			codec_reg_set(CODEC_PMR2, SB_ADC); //SB_ADC
			codec_reg_set(CODEC_PMR1, SB_MICBIAS); // SB_MICBIAS
			codec_reg_set(CODEC_PMR1, SB_MIC1); //SB_MIC1
			codec_reg_set(CODEC_PMR1, SB_MIC2); //SB_MIC2
			codec_reg_set(CODEC_PMR1, SB_LINE); //SB_LINE
			codec_reg_set(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
			codec_reg_set(CODEC_CR3, INSEL(3)); //INSEL(3)
			codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
			codec_reg_set(CODEC_CR4, ADC_R_ONLY); //ADC_R_ONLY
			codec_reg_set(CODEC_CR1, BTL_MUTE); //BTL_MUTE
			break;

		case DAC_TO_BTL:
			/* set dac to btl*/
			codec_reg_clear(CODEC_PMR1, SB); //SB
			codec_reg_clear(CODEC_PMR1, SB_SLEEP); //SB_SLEEP
			codec_reg_clear(CODEC_PMR2, SB_DAC); //SB_DAC
			codec_reg_clear(CODEC_PMR2, SB_HP); //SB_HP
			codec_reg_clear(CODEC_PMR2, SB_LOUT); //SB_LOUT
			codec_reg_clear(CODEC_PMR2, SB_BTL); //SB_BTL
			codec_reg_set(CODEC_CR1, OUTSEL(3)); //OUTSEL(3)
			codec_reg_clear(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
			codec_reg_clear(CODEC_CR2, DAC_MUTE); //DAC_MUTE
			codec_reg_clear(CODEC_CR1, HP_MUTE); //HP_MUTE
			codec_reg_clear(CODEC_CR1, LINEOUT_MUTE);//LINEOUT_MUTE
			codec_reg_clear(CODEC_CR1, BTL_MUTE); //BTL_MUTE
			codec_reg_clear(CODEC_CR2, NOMAD); //NOMAD

			/* reduce power set */
			codec_reg_set(CODEC_PMR2, SB_ADC); //SB_ADC
			codec_reg_set(CODEC_PMR1, SB_MICBIAS); //SB_MICBIAS
			codec_reg_set(CODEC_PMR1, SB_MIC1); //SB_MIC1
			codec_reg_set(CODEC_PMR1, SB_MIC2); //SB_MIC2
			codec_reg_set(CODEC_PMR1, SB_LINE); //SB_LINE
			codec_reg_set(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
			codec_reg_set(CODEC_CR3, INSEL(3)); //INSEL(3)
			codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
			codec_reg_set(CODEC_CR4, ADC_R_ONLY); //ADC_R_ONLY
			break;

		default:
			/* set dac to btl*/
			codec_reg_clear(CODEC_PMR1, SB); //SB
			codec_reg_clear(CODEC_PMR1, SB_SLEEP); //SB_SLEEP
			codec_reg_clear(CODEC_PMR2, SB_DAC); //SB_DAC
			codec_reg_clear(CODEC_PMR2, SB_HP); //SB_HP
			codec_reg_clear(CODEC_PMR2, SB_LOUT); //SB_LOUT
			codec_reg_clear(CODEC_PMR2, SB_BTL); //SB_BTL
			codec_reg_set(CODEC_CR1, OUTSEL(3)); //OUTSEL(3)
			codec_reg_clear(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
			codec_reg_clear(CODEC_CR2, DAC_MUTE); //DAC_MUTE
			codec_reg_clear(CODEC_CR1, HP_MUTE); //HP_MUTE
			codec_reg_clear(CODEC_CR1, LINEOUT_MUTE);//LINEOUT_MUTE
			codec_reg_clear(CODEC_CR1, BTL_MUTE); //BTL_MUTE
			codec_reg_clear(CODEC_CR2, NOMAD); //NOMAD

			/* reduce power set */
			codec_reg_set(CODEC_PMR2, SB_ADC); //SB_ADC
			codec_reg_set(CODEC_PMR1, SB_MICBIAS); //SB_MICBIAS
			codec_reg_set(CODEC_PMR1, SB_MIC1); //SB_MIC1
			codec_reg_set(CODEC_PMR1, SB_MIC2); //SB_MIC2
			codec_reg_set(CODEC_PMR1, SB_LINE); //SB_LINE
			codec_reg_set(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
			codec_reg_set(CODEC_CR3, INSEL(3)); //INSEL(3)
			codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
			codec_reg_set(CODEC_CR4, ADC_R_ONLY); //ADC_R_ONLY
			break;
		}
		codec_reg_clear(CODEC_CR4, ADC_HPF); //ADC_HPF
		break;
	case PCM_SET_RECORD:
		kprintf("pcm_ioctl, PCM_SET_RECORD: %d\n",arg);
		jz_audio_reset(); /* audio reset */
		switch (arg)
		{
		case MONO_MIC1_IN:
			kprintf("mono mic1 in record \n");
			/* set mono mic1 int*/
			codec_reg_clear(CODEC_PMR1, SB_AIP); //SB_AIP
			codec_reg_clear(CODEC_PMR1, SB); //SB
			codec_reg_clear(CODEC_PMR1, SB_SLEEP); // SB_SLEEP
			codec_reg_clear(CODEC_PMR2, SB_ADC); //SB_ADC
			codec_reg_clear(CODEC_AGC1, AGC_EN);
			codec_reg_clear(CODEC_PMR1, SB_MICBIAS);//SB_MICBIAS
			codec_reg_clear(CODEC_PMR1, SB_MIC1); //SB_MIC1
			codec_reg_set(CODEC_CR3, INSEL(0)); //INSEL(0)
			codec_reg_clear(CODEC_CR3, MICSTEREO); //MICSTEREO
			codec_reg_set(CODEC_CR4, ADC_R_ONLY); //ADC_R_ONLY

			/* reduce power set*/
			codec_reg_set(CODEC_PMR2, SB_DAC); //SB_DAC
			codec_reg_set(CODEC_PMR2, SB_HP); //SB_HP
			codec_reg_set(CODEC_PMR2, SB_LOUT); //SB_LOUT
			codec_reg_set(CODEC_PMR2, SB_BTL); //SB_BTL
			codec_reg_set(CODEC_PMR1, SB_MIC2); //SB_MIC2
			codec_reg_set(CODEC_PMR1, SB_LINE); //SB_LINE
			codec_reg_set(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
			codec_reg_set(CODEC_CR1, OUTSEL(3)); //OUTSEL(3)
			codec_reg_set(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
			codec_reg_set(CODEC_CR2, DAC_MUTE); //DAC_MUTE
			codec_reg_set(CODEC_CR1, HP_MUTE); //HP_MUTE
			codec_reg_set(CODEC_CR1, LINEOUT_MUTE);//LINEOUT_MUTE
			codec_reg_set(CODEC_CR1, BTL_MUTE); //BTL_MUTE
			codec_reg_set(CODEC_CR2, NOMAD); //NOMAD
			break;
		case LINE_IN:
			kprintf("line in record \n");
			/* set line in*/
			codec_reg_clear(CODEC_PMR1, SB_AIP); //SB_AIP
			codec_reg_clear(CODEC_PMR1, SB); //SB
			codec_reg_clear(CODEC_PMR1, SB_SLEEP); // SB_SLEEP
			codec_reg_clear(CODEC_PMR2, SB_ADC); //SB_ADC
			codec_reg_clear(CODEC_PMR1, SB_LINE); //SB_LINE
			codec_reg_clear(CODEC_AGC1, AGC_EN);
			codec_reg_set(CODEC_CR3, INSEL(2)); //INSEL(2)
			codec_reg_clear(CODEC_CR4, ADC_R_ONLY);//ADC_R_ONLY

			/* reduce power set*/
			codec_reg_set(CODEC_PMR2, SB_DAC); //SB_DAC
			codec_reg_set(CODEC_PMR2, SB_HP); //SB_HP
			codec_reg_set(CODEC_PMR2, SB_LOUT); //SB_LOUT
			codec_reg_set(CODEC_PMR2, SB_BTL); //SB_BTL
			codec_reg_set(CODEC_PMR1, SB_MIC1); //SB_MIC1
			codec_reg_set(CODEC_PMR1, SB_MIC2); //SB_MIC2
			codec_reg_set(CODEC_PMR1, SB_MICBIAS); //SB_MICBIAS
			codec_reg_set(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
			codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
			codec_reg_set(CODEC_CR1, OUTSEL(3)); //OUTSEL(3)
			codec_reg_set(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
			codec_reg_set(CODEC_CR2, DAC_MUTE); //DAC_MUTE
			codec_reg_set(CODEC_CR1, HP_MUTE); //HP_MUTE
			codec_reg_set(CODEC_CR1, LINEOUT_MUTE);//LINEOUT_MUTE
			codec_reg_set(CODEC_CR1, BTL_MUTE); //BTL_MUTE
			codec_reg_set(CODEC_CR2, NOMAD); //NOMAD
			break;
		case STEREO_IN:
			kprintf("stero in record \n");
			/* set stereo in*/
			codec_reg_clear(CODEC_PMR1, SB_AIP); //SB_AIP
			codec_reg_clear(CODEC_PMR1, SB); //SB
			codec_reg_clear(CODEC_PMR1, SB_SLEEP); // SB_SLEEP
			codec_reg_clear(CODEC_PMR2, SB_ADC); //SB_ADC
			codec_reg_clear(CODEC_AGC1, AGC_EN);
			codec_reg_clear(CODEC_PMR1, SB_MICBIAS);//SB_MICBIAS
			codec_reg_clear(CODEC_PMR1, SB_MIC1); //SB_MIC1
			codec_reg_clear(CODEC_PMR1, SB_MIC2); //SB_MIC2
			codec_reg_set(CODEC_CR3, INSEL(0)); //INSEL(0)
			codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
			codec_reg_clear(CODEC_CR4, ADC_R_ONLY);//ADC_R_ONLY

			/* reduce power set*/
			codec_reg_set(CODEC_PMR2, SB_DAC); //SB_DAC
			codec_reg_set(CODEC_PMR2, SB_HP); //SB_HP
			codec_reg_set(CODEC_PMR2, SB_LOUT); //SB_LOUT
			codec_reg_set(CODEC_PMR2, SB_BTL); //SB_BTL
			codec_reg_set(CODEC_PMR1, SB_LINE); //SB_LINE
			codec_reg_set(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
			codec_reg_set(CODEC_CR1, OUTSEL(3)); //OUTSEL(3)
			codec_reg_set(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
			codec_reg_set(CODEC_CR2, DAC_MUTE); //DAC_MUTE
			codec_reg_set(CODEC_CR1, HP_MUTE); //HP_MUTE
			codec_reg_set(CODEC_CR1, LINEOUT_MUTE);//LINEOUT_MUTE
			codec_reg_set(CODEC_CR1, BTL_MUTE); //BTL_MUTE
			codec_reg_set(CODEC_CR2, NOMAD); //NOMAD
			break;

		default:
			kprintf("default record \n");

			codec_reg_clear(CODEC_PMR1, SB_AIP); //SB_AIP
			codec_reg_clear(CODEC_PMR1, SB); //SB
			codec_reg_clear(CODEC_PMR1, SB_SLEEP); // SB_SLEEP
			codec_reg_clear(CODEC_PMR2, SB_ADC); //SB_ADC
			codec_reg_clear(CODEC_AGC1, AGC_EN);
			
			codec_reg_clear(CODEC_PMR1, SB_MIC1); //SB_MIC1
			codec_reg_clear(CODEC_PMR1, SB_MIC2); //SB_MIC2
			codec_reg_clear(CODEC_PMR1, SB_MICBIAS); //SB_MICBIAS
			break;
		}

		codec_reg_write(CODEC_CGR3, 0x1f);
		codec_reg_write(CODEC_CGR4, 0x1f);
		codec_reg_write(CODEC_CGR7, 0x3f);
		codec_reg_write(CODEC_CGR8, 0x1f);
		codec_reg_write(CODEC_CGR9, 0x1f);
		GetDmaInfo();
		break;
	case PCM_RESET:
		kprintf("pcm_ioctl, PCM_RESET\n");
		REG_DMAC_DCCSR(PLAYBACK_CHANNEL) &= ~(DMAC_DCCSR_AR | DMAC_DCCSR_HLT | DMAC_DCCSR_EN | DMAC_DCCSR_CT);/* reset DMA channel */
		break;

	case PCM_REINIT:
		kprintf("pcm_ioctl, PCM_REINIT\n");
		break;

	case PCM_GET_HP_VOL:
		return get_volume_reg();

	case PCM_GET_VOL:
		return get_volume_reg();

	case PCM_GET_SAMPLE_MAX:
		kprintf("pcm_ioctl, PCM_GET_SAMPLE_MAX\n");
		return 65535;

	case PCM_POWER_OFF:
		kprintf("pcm_ioctl, PCM_POWER_OFF\n");
		codec_reg_set(CODEC_PMR1, SB); /* power down active */
		codec_reg_set(CODEC_PMR1, SB_SLEEP); /* sleep active */
		return 1;

	case PCM_POWER_ON:
		kprintf("pcm_ioctl, PCM_POWER_ON\n");
		codec_reg_clear(CODEC_PMR1, SB); /* power down inactive */
		codec_reg_clear(CODEC_PMR1, SB_SLEEP); /* sleep inactive */
		return 1;

	case PCM_DATA_FINISH:
		kprintf("pcm_ioctl, PCM_DATA_FINISH\n");
		break;

	case PCM_BYPASS_LL:
	case PCM_BYPASS_LH:
		/* bypass line to headphone and line out*/
		codec_reg_clear(CODEC_PMR1, SB); //SB
		codec_reg_clear(CODEC_PMR1, SB_SLEEP); //SB_SLEEP
		codec_reg_clear(CODEC_PMR2, SB_HP); //SB_HP
		codec_reg_set(CODEC_PMR2, SB_BTL); //SB_BTL
		codec_reg_set(CODEC_PMR2, SB_LOUT); //SB_LOUT
		codec_reg_clear(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
		codec_reg_set(CODEC_CR1, OUTSEL(2)); //OUTSEL(2)
		codec_reg_clear(CODEC_CR1, HP_MUTE); //HP_MUTE

		/* reduce power set */
		codec_reg_set(CODEC_PMR2, SB_DAC); //SB_DAC
		codec_reg_set(CODEC_PMR2, SB_ADC); //SB_ADC
		codec_reg_set(CODEC_PMR1, SB_MICBIAS); //SB_MICBIAS
		codec_reg_set(CODEC_PMR1, SB_MIC1); //SB_MIC1
		codec_reg_set(CODEC_PMR1, SB_MIC2); //SB_MIC2
		codec_reg_set(CODEC_PMR1, SB_LINE); //SB_LINE
		codec_reg_set(CODEC_CR3, INSEL(3)); //INSEL(3)
		codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
		codec_reg_set(CODEC_CR4, ADC_R_ONLY); //ADC_R_ONLY
		codec_reg_set(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
		codec_reg_set(CODEC_PMR2, SB_DAC); //SB_DAC
		codec_reg_set(CODEC_CR1, LINEOUT_MUTE); //LINEOUT_MUTE
		codec_reg_set(CODEC_CR1, BTL_MUTE); //BTL_MUTE

		if (cmd == PCM_BYPASS_LH)
			codec_reg_set(CODEC_CR2, NOMAD); //NOMAD
		else
			codec_reg_clear(CODEC_CR2, NOMAD); //NOMAD
		break;

	case PCM_BYPASS_LB:
	case PCM_BYPASS_LD:
		/* bypass line to differential line out and BTL out*/
		codec_reg_clear(CODEC_PMR1, SB); //SB
		codec_reg_clear(CODEC_PMR1, SB_SLEEP); //SB_SLEEP
		codec_reg_clear(CODEC_PMR2, SB_HP); //SB_HP
		codec_reg_set(CODEC_PMR2, SB_BTL); //SB_BTL
		codec_reg_clear(CODEC_PMR2, SB_LOUT); //SB_LOUT
		codec_reg_clear(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
		codec_reg_set(CODEC_CR1, OUTSEL(2)); //OUTSEL(2)
		codec_reg_clear(CODEC_CR1, HP_MUTE); //HP_MUTE
		codec_reg_clear(CODEC_CR1, LINEOUT_MUTE);//LINEOUT_MUTE
		codec_reg_clear(CODEC_CR2, NOMAD); //NOMAD

		/* reduce power set */
		codec_reg_set(CODEC_PMR2, SB_DAC); //SB_DAC
		codec_reg_set(CODEC_PMR2, SB_ADC); //SB_ADC
		codec_reg_set(CODEC_PMR1, SB_MICBIAS); //SB_MICBIAS
		codec_reg_set(CODEC_PMR1, SB_MIC1); //SB_MIC1
		codec_reg_set(CODEC_PMR1, SB_MIC2); //SB_MIC2
		codec_reg_set(CODEC_PMR1, SB_LINE); //SB_LINE
		codec_reg_set(CODEC_CR3, INSEL(3)); //INSEL(3)
		codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
		codec_reg_set(CODEC_CR4, ADC_R_ONLY); //ADC_R_ONLY
		codec_reg_set(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
		codec_reg_set(CODEC_PMR2, SB_DAC); //SB_DAC
		codec_reg_set(CODEC_CR1, BTL_MUTE); //BTL_MUTE

		if (cmd == PCM_BYPASS_LB)
		{
			codec_reg_clear(CODEC_PMR2, SB_BTL); //SB_BTL
			codec_reg_clear(CODEC_CR1, BTL_MUTE); //BTL_MUTE
		}
		break;

	case PCM_SIDETONE_SB:
	case PCM_SIDETONE_SR:
		/* stereo mic to 16Ohm headphone out mic1 to right channel
		 stereo mic to BTL out mic2 to right channel */
		codec_reg_clear(CODEC_PMR1, SB); //SB
		codec_reg_clear(CODEC_PMR1, SB_SLEEP); //SB_SLEEP
		codec_reg_clear(CODEC_PMR2, SB_HP); //SB_HP
		codec_reg_set(CODEC_PMR2, SB_LOUT); //SB_LOUT
		codec_reg_set(CODEC_PMR2, SB_BTL); //SB_BTL
		codec_reg_clear(CODEC_PMR1, SB_MICBIAS); //SB_MICBIAS
		codec_reg_clear(CODEC_PMR1, SB_MIC1); //SB_MIC1
		codec_reg_clear(CODEC_PMR1, SB_MIC2); //SB_MIC2
		codec_reg_set(CODEC_CR1, OUTSEL(0)); //OUTSEL(0)
		codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
		codec_reg_clear(CODEC_CR1, HP_MUTE); //HP_MUTE
		codec_reg_set(CODEC_CR2, NOMAD); //NOMAD

		/* reduce power set*/
		codec_reg_set(CODEC_PMR2, SB_DAC); //SB_DAC
		codec_reg_set(CODEC_PMR2, SB_ADC); //SB_ADC
		codec_reg_set(CODEC_PMR1, SB_LINE); //SB_LINE
		codec_reg_set(CODEC_PMR1, SB_BYPASS); //SB_BYPASS
		codec_reg_set(CODEC_CR3, INSEL(3)); //INSEL(3)
		codec_reg_set(CODEC_CR3, MICSTEREO); //MICSTEREO
		codec_reg_set(CODEC_CR4, ADC_R_ONLY); //ADC_R_ONLY
		codec_reg_set(CODEC_CR2, DAC_R_ONLY); //DAC_R_ONLY
		codec_reg_set(CODEC_CR1, HP_MUTE); //HP_MUTE
		codec_reg_set(CODEC_CR1, LINEOUT_MUTE); //LINEOUT_MUTE

		if (cmd == PCM_SIDETONE_SB)
		{
			codec_reg_clear(CODEC_PMR2, SB_LOUT); //SB_LOUT
			codec_reg_clear(CODEC_PMR2, SB_BTL); //SB_BTL
			codec_reg_clear(CODEC_CR1, LINEOUT_MUTE); //LINEOUT_MUTE
			codec_reg_clear(CODEC_CR2, NOMAD); //NOMAD
		}
		break;
	default:
		kprintf("pcm_ioctl:Unsupported I/O command: %08x\n", cmd);
		return -1;
	}

	return 0;
}


////////////////////////////////////////////////////
// 功能: 打开或者关闭声音
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void SetMuteMode(char mode)
{
	if( mode )
		codec_reg_clear(CODEC_CR2, DAC_MUTE);
	else
		codec_reg_set(CODEC_CR2, DAC_MUTE);
}


////////////////////////////////////////////////////
// 功能: 设置功放状态
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void SetPowerAmplifier(char io)
{
#if defined(CONFIG_AMP_IO)
	__gpio_as_output(CONFIG_AMP_IO);
	if(io)
	{
#if CONFIG_AMP_ENA
		__gpio_set_pin(CONFIG_AMP_IO);
#else
		__gpio_clear_pin(CONFIG_AMP_IO);
#endif		
	}
	else
	{
#if CONFIG_AMP_ENA
		__gpio_clear_pin(CONFIG_AMP_IO);
#else
		__gpio_set_pin(CONFIG_AMP_IO);
#endif		
	}
#endif
}


////////////////////////////////////////////////////
// 功能: 设置mos管状态
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void SetMoseCe(char io)
{
#if defined(CONFIG_PIPO_IO)
	__gpio_as_output(CONFIG_PIPO_IO);
	if(io)
	{
#if CONFIG_PIPO_ENA
		__gpio_set_pin(CONFIG_PIPO_IO);
#else
		__gpio_clear_pin(CONFIG_PIPO_IO);
#endif		
	}
	else
	{
#if CONFIG_PIPO_ENA
		__gpio_clear_pin(CONFIG_PIPO_IO);
#else
		__gpio_set_pin(CONFIG_PIPO_IO);
#endif		
	}
#endif
}

////////////////////////////////////////////////////
// 功能: 关闭codec设备
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void CloseMediaCodecDevice()
{
	fDeviceStatus = DEVICE_CLOSE_STATUS;
	kdebug(mod_media, PRINT_INFO, "close meida device\n");

	//close codec
	codec_reg_write(CODEC_CGR8, 0x1f);
	codec_reg_write(CODEC_CGR9, 0x1f);
	codec_reg_write(CODEC_CGR1, 0xff);


	MillinsecoundDelay(10);
	codec_reg_set(CODEC_CCR2, SB_LOUT);
	codec_reg_set(CODEC_CCR2, SB_BTL);
//	codec_reg_set(CODEC_PMR2, SB_HP);
	MillinsecoundDelay(20);
	codec_reg_set(CODEC_PMR1, SB_SLEEP);
	MillinsecoundDelay(10);
	codec_reg_set(CODEC_PMR1, SB);
	MillinsecoundDelay(10);
	codec_reg_set(CODEC_PMR2, SB_DAC | SB_ADC);
}

////////////////////////////////////////////////////
// 功能: 打开codec设备
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void OpenMediaCodecDevice()
{
	kdebug(mod_media, PRINT_INFO, "open media device\n");
	fDeviceStatus = DEVICE_OPEN_STATUS;

	//codec寄存器设置
	codec_reg_write(CODEC_AICR, 0xFF);
	codec_reg_write(CODEC_TR1, 0x00);
	codec_reg_write(CODEC_IFR, 0xFF);
	codec_reg_write(CODEC_ICR, 0x2F);

	codec_reg_write(CODEC_CGR1, 0x1f); //GOL
	codec_reg_write(CODEC_CGR2, 0x1f);
	codec_reg_write(CODEC_CGR5, 0x1f); //GODL
	codec_reg_write(CODEC_CGR6, 0x1f);

	codec_reg_write(CODEC_CR1, 0x1b); 
	codec_reg_write(CODEC_CR2, 0x20); 
	codec_reg_write(CODEC_CR3, 0x00); 
	codec_reg_write(CODEC_CR4, 0x80); 
	MillinsecoundDelay(20);

	// 关闭音量开关
	codec_reg_set(CODEC_CR2, DAC_MUTE);		//DAC_MUTE
}

////////////////////////////////////////////////////
// 功能: 关闭声音设备
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void CloseMediaDevice(char channel)
{
	int arg,group;
	int time;
	arg = ((channel == 0) ? RECORD_CHANNEL : PLAYBACK_CHANNEL );

	//等待DMA结束
	time = 0;
	while( GetDmaCount() )
	{
#ifdef KPRINTF_DEF
		kprintf("close media device : count = %x\n",GetDmaCount() );
#endif
		sTimerSleep(10, NULL);

		//加入延迟处理，防止死锁
		time++;
		if( time > 10 )
		{
			kdebug(mod_media, PRINT_WARNING, "close media device timer out\n");
			break;
		}
	}

	//stop dma
	group   = (arg - EIRQ_DMA_BASE) / HALF_DMA_NUM;
	REG_DMAC_DCCSR(PLAYBACK_CHANNEL) &= ~(DMAC_DCCSR_EN | DMAC_DCCSR_CT | DMAC_DCCSR_TT);

	//close aic
	jz_audio_reset();

	CloseMediaCodecDevice();
}

////////////////////////////////////////////////////
// 功能: 开打声音设备
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void OpenMediaDevice(unsigned long sample,unsigned long channle , char mode)
{
	//统一设置成48000，双声道，16BIT PCM模式
	pcm_ioctl(PCM_SET_SAMPLE_RATE,sample);
	pcm_ioctl(PCM_SET_CHANNEL,channle);
	pcm_ioctl(PCM_SET_FORMAT,16);

	if( !mode )
	{
 		pcm_ioctl(PCM_SET_REPLAY,0);
		if( hPlayer == NULL )
			hPlayer = IrqAttach(EIRQ_DMA_BASE + PLAYBACK_CHANNEL, IPL_AUDIO, 0, intr_handler_playback_dma, NULL);
	}
	else
	{
		pcm_ioctl(PCM_SET_RECORD,0);
		if( hRecord == NULL )
			hRecord = IrqAttach(EIRQ_DMA_BASE + RECORD_CHANNEL, IPL_AUDIO, 0, intr_handler_record_dma, NULL);
	}
	
	if( !mode )
		pcm_ioctl(PCM_SET_VOL,24);
	else
		pcm_ioctl(PCM_SET_VOL,0);
}

////////////////////////////////////////////////////
// 功能: 开打声音设备
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void MediaSysInit()
{
	/* Start AIC and DMA Clock */
	REG_CPM_CLKGR0 &= ~CLKGR0_AIC;
	REG_CPM_CLKGR0 &= ~CLKGR0_DMAC;
	REG_CPM_CLKGR0 &= ~CLKGR0_MDMA;

	/* set AIC clk div 12MHz extclk */
	REG_CPM_I2SCDR = 0x00;
	REG_CPM_CPCCR |= CPCCR_CE;

	/* AIC Init */
	REG_AIC_FR |= AIC_FR_ICDC; //select internal codec
	REG_AIC_FR |= AIC_FR_AUSEL; //select I2S/MSL mode
	REG_AIC_FR &= ~(AIC_FR_SYNCD | AIC_FR_BCKD);

	REG_AIC_I2SCR &= ~AIC_I2SCR_AMSL; //select I2S mode
	REG_AIC_I2SCR |= AIC_I2SCR_ESCLK; //enable clk
	REG_AIC_I2SCR |= (1 << 17);
	REG_AIC_FR |= AIC_FR_ENB;

	/* AIC Configure */
	REG_AIC_FR |= (AIC_FR_RFTH(16) | AIC_FR_TFTH(32));
	REG_AIC_CR = (AIC_CR_OSS_16BIT | AIC_CR_ISS_16BIT);
	REG_AIC_CR |= (AIC_CR_RDMS | AIC_CR_TDMS | AIC_CR_RFLUSH | AIC_CR_TFLUSH);

	REG_AIC_ACSR = 0x00000000;
	REG_AIC_FR &= ~AIC_FR_LSMP;

	fDeviceStatus = DEVICE_CLOSE_STATUS;

	OpenMediaCodecDevice();
}

////////////////////////////////////////////////////
// 功能: 开打声音设备
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void MediaDraveInit(char channel)
{
#ifdef KPRINTF_DEF
	kprintf("pcm_init start!\n");
#endif

	MediaSysInit();
	//DMA初始化
	if( channel )
	{
		dma_init(PLAYBACK_CHANNEL,1);
	}
	else
	{
		dma_init(RECORD_CHANNEL,0);
	}

	interrupt_count = 0;
#ifdef KPRINTF_DEF
	kprintf("===============================================\n  pcm_init finished!  \n===============================================\n");
#endif
}

////////////////////////////////////////////////////
// 功能: 删除media建立的中断向量表
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void DestroyMediaDevoice(char mode)
{
	if( !mode )
	{
		IrqDetach(hPlayer);
		hPlayer = NULL;
	}
	else
	{
		IrqDetach(hRecord);
		hRecord = NULL;
	}
}

////////////////////////////////////////////////////
// 功能: 触发DMA中断，传送数据
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void DmaDataToAic(int arg,unsigned int sour_addr, unsigned int len,unsigned char mode)
{

	if( !arg )
	{	//放音DMA
		dma_cache_wback_inv(sour_addr, len);

		REG_AIC_ACSR &= ~AIC_SR_TUR;
		dma_start(PLAYBACK_CHANNEL,	PHYADDR(sour_addr), PHYADDR(AIC_DR),len,mode);
		REG_AIC_CR |= AIC_CR_ERPL;

		if (REG_AIC_ACSR & AIC_SR_TUR)
			REG_AIC_ACSR &= ~AIC_SR_TUR;
	}
	else
	{	//录音DMA
		__dcache_inv((unsigned long)sour_addr, len);

		REG_AIC_CR |= AIC_CR_EREC;
		dma_start(RECORD_CHANNEL, PHYADDR(AIC_DR), PHYADDR(sour_addr),len,mode);

		if (REG_AIC_ACSR & AIC_SR_ROR)
			REG_AIC_ACSR &= ~AIC_SR_ROR;
	}
}
