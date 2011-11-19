
JZC_LINUX       = 0
JZC_MINIOS      = 1
JZC_FPGA        = 0 #tcsm0 must init to zero because tcsm0 fifo using gp0 clear to zero
JZC_RTL         = 0

# -------- Generated by configure -----------

# Ensure that locale settings do not interfere with shell commands.
export LC_ALL = C

CONFIGURATION = --host-cc=gcc --cc=mipsel-linux-gcc -mips32 --target=mips-linux --enable-cross-compile --disable-x11 --disable-xinerama --disable-vm --disable-termcap --disable-freetype --enable-fbdev --enable-ossaudio --disable-esd --disable-ivtv --enable-hardcoded-tables --disable-mencoder --disable-mp3lib --enable-mad --disable-faac --disable-faac-lavc --enable-pvr --enable-tv-v4l1 --enable-tv-v4l2 --extra-ldflags=-L./libmad --extra-cflags=-I./libmad/libmad-0.15.1b/ -imacros libjzcommon/com_config.h

CHARSET = UTF-8
DOC_LANGS = en
DOC_LANG_ALL = cs de en es fr hu it pl ru zh_CN
MAN_LANGS = en
MAN_LANG_ALL = cs de en es fr hu it pl ru zh_CN

prefix  = $(DESTDIR)/usr/local
BINDIR  = $(DESTDIR)/usr/local/bin
DATADIR = $(DESTDIR)/usr/local/share/mplayer
LIBDIR  = $(DESTDIR)/usr/local/lib
MANDIR  = $(DESTDIR)/usr/local/share/man
CONFDIR = $(DESTDIR)/usr/local/etc/mplayer


AR      = /gcc-mpis/bin/noahos-ar rcsv
AS      = /gcc-mpis/bin/noahos-gcc -mips32
LD	= /gcc-mpis/bin/noahos-ld
COPY	= /gcc-mpis/bin/noahos-objcopy
NM	= /gcc-mpis/bin/noahos-nm
DUMP	= /gcc-mpis/bin/noahos-objdump

ifeq    ($(JZC_MINIOS),1)
CC      = /gcc-mpis/bin/noahos-gcc -mips32 -D__MINIOS__ -O2 -mno-abicalls -fno-pic -fno-builtin        \
          -fno-exceptions -ffunction-sections -falign-functions=32                             \
          -I./uc_inc/ -I../uc_inc/ -I../../uc_inc/ -I../../ -I../ -I./\
	    -I../Include/ -I../Include/libc/ 		\
	   -I../Include/libm/ -I../../Include/ -I../../Include/libc/ -I../../Include/libm/		 	\
	      -I../../../Include/ -I../../../Include/libc/ -I../../../Include/libm/		 	\
          -mlong-calls -fshort-wchar -fomit-frame-pointer -ffast-math -G 0 -Wall
CXX     = /gcc-mpis/bin/noahos-gcc -mips32 -D__MINIOS__ -O2 -mno-abicalls -fno-pic -fno-builtin        \
          -fno-exceptions -ffunction-sections -falign-functions=32                             \
          -I./uc_inc -I../uc_inc -I../ -I./     \
	   -I../Include/ -I../Include/libc -I../Include/libm                			\
          -mlong-calls -fshort-wchar -fomit-frame-pointer -ffast-math -G 0 -Wall
else
CC      = /gcc-mpis/bin/noahos-gcc -mips32
CXX     = /gcc-mpis/bin/noahos-gcc -mips32
endif

HOST_CC = gcc
INSTALL = install
INSTALLSTRIP = -s
WINDRES = windres

ifeq    ($(JZC_MINIOS),1)
CFLAGS  += -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wdisabled-optimization -Wno-pointer-sign -Wdeclaration-after-statement -std=gnu99 -Wall -Wno-switch -Wno-parentheses -Wpointer-arith -Wredundant-decls -O4   -pipe -ffast-math -fomit-frame-pointer -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -Ilibdvdread4 -I. -I./libmad/libmad-0.15.1b/ -I../libmpdemux/zlib -I./libmpdemux/zlib -imacros $(TOP)libjzcommon/com_config.h -D_REENTRANT 
CFLAGS  += -DNOAH_OS
ASFLAGS  = $(CFLAGS)
CXXFLAGS =  -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wdisabled-optimization -Wno-pointer-sign -Wdeclaration-after-statement -std=gnu99 -Wall -Wno-switch -Wno-parentheses -Wpointer-arith -Wredundant-decls -O4   -pipe -ffast-math -fomit-frame-pointer -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -Ilibdvdread4 -I. -I./libmad/libmad-0.15.1b/ -I../libmpdemux/zlib -I./libmpdemux/zlib -imacros $(TOP)libjzcommon/com_config.h -D_REENTRANT  
else
CFLAGS   = -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wdisabled-optimization -Wno-pointer-sign -Wdeclaration-after-statement -std=gnu99 -Wall -Wno-switch -Wno-parentheses -Wpointer-arith -Wredundant-decls -O4   -pipe -ffast-math -fomit-frame-pointer -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -Ilibdvdread4 -I. -I./libmad/libmad-0.15.1b/ -imacros libjzcommon/com_config.h -DPIC -D_REENTRANT 
CFLAGS  += -DNOAH_OS
ASFLAGS  = $(CFLAGS)
CXXFLAGS =  -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wdisabled-optimization -Wno-pointer-sign -Wdeclaration-after-statement -std=gnu99 -Wall -Wno-switch -Wno-parentheses -Wpointer-arith -Wredundant-decls -O4   -pipe -ffast-math -fomit-frame-pointer -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -Ilibdvdread4 -I. -I./libmad/libmad-0.15.1b/ -imacros libjzcommon/com_config.h -DPIC -D_REENTRANT  
endif
#CC_DEPFLAGS = -MMD -MF $(@:.mid=.d) -MT $(@:.mid=.o)
#AS_DEPFLAGS = -MMD -MF $(@:.mid=.d) -MT $(@:.mid=.o)

CFLAGS_DHAHELPER         = 
CFLAGS_FAAD_FIXED        = 
CFLAGS_LIBDVDCSS         = 
CFLAGS_LIBDVDCSS_DVDREAD = -Ilibdvdcss
CFLAGS_LIBDVDNAV         = -Ilibdvdnav
CFLAGS_NO_OMIT_LEAF_FRAME_POINTER = 
CFLAGS_STACKREALIGN      = 
CFLAGS_SVGALIB_HELPER    = 
CFLAGS_TREMOR_LOW        = 

ifeq    ($(JZC_MINIOS),1)
EXTRALIBS          = -Wl,-z,noexecstack -L./libmad -ffast-math -lmad -rdynamic  -lm  -lc -lnss_files -lnss_dns -lresolv
else
EXTRALIBS          = -Wl,-z,noexecstack -L./libmad -ffast-math   -lpng -lz -lmng -lz -ljpeg -lasound -ldl -lpthread -lz -lmad -lpthread -ldl -rdynamic  -lm 
endif
EXTRALIBS_MPLAYER  = 
EXTRALIBS_MENCODER = 

GETCH = getch2.c
HELP_FILE = help/help_mp-en.h

ifeq    ($(JZC_MINIOS),1)
TIMER = timer_minios_jz4760.c
else
TIMER = timer-linux.c
endif

EXESUF      = 
EXESUFS_ALL = .exe

ARCH = mips
ARCH_MIPS = yes

HAVE_FAST_CLZ = yes

MENCODER = no
MPLAYER  = yes

NEED_GETTIMEOFDAY = no
NEED_GLOB         = no
NEED_MMAP         = 
NEED_SETENV       = no
NEED_SHMEM        = no
NEED_STRSEP       = no
NEED_SWAB         = no
NEED_VSSCANF      = no

# features
3DFX = no
AA = no
ifeq    ($(JZC_MINIOS),1)
ALSA1X =
else
ALSA1X = yes
endif
ALSA9 = 
ALSA5 = 
APPLE_IR = no
APPLE_REMOTE = no
ARTS = no
AUDIO_INPUT = no
BITMAP_FONT = yes
BL = no
CACA = no
CDDA = 
CDDB = no
COREAUDIO = no
COREVIDEO = no
DART = no
DFBMGA = 
DGA = no
DIRECT3D = no
DIRECTFB = no
DIRECTX = no
DVBIN = no
DVDNAV = no
DVDNAV_INTERNAL = no
DVDREAD = no
DVDREAD_INTERNAL = no
DXR2 = no
DXR3 = no
ESD = no
FAAC= no
FAAD = no
FAAD_INTERNAL = no
FASTMEMCPY = no
FBDEV = yes
FREETYPE = no
FTP = no
GIF = no
GGI = no
GL = no
GL_WIN32 = 
GL_X11 = 
GL_SDL = 
MATRIXVIEW = no
GUI = no
GUI_GTK = 
GUI_WIN32 = 
HAVE_POSIX_SELECT = no
HAVE_SYS_MMAN_H = no
IVTV = no
JACK = no
JOYSTICK = no
ifeq    ($(JZC_MINIOS),1)
JPEG = no
else
JPEG = yes
endif
KAI = no
KVA = no
LADSPA = no
LIBA52 = no
LIBASS = no
LIBASS_INTERNAL = no
LIBBLURAY = no
LIBBS2B = no
LIBDCA = no
LIBDV = no
LIBDVDCSS_INTERNAL = no
LIBLZO = no
LIBMAD = yes
LIBMENU = no
LIBMENU_DVBIN = no
LIBMPEG2 = yes
LIBMPEG2_INTERNAL = no
LIBNEMESI = no
LIBNUT = no
LIBSMBCLIENT = no
LIBTHEORA = no
LIRC = no
LIVE555 = no
MACOSX_FINDER = no
MD5SUM = yes
MGA = no
ifeq    ($(JZC_MINIOS),1)
MNG = no
else
MNG = yes
endif
MP3LAME = no
MP3LIB = no
MPG123 = no
MUSEPACK = no
NAS = no
NATIVE_RTSP = no
NETWORKING = no
OPENAL = no
OSS = no
PE_EXECUTABLE = 
ifeq    ($(JZC_MINIOS),1)
PNG = no
PNM = no
else
PNG = yes
PNM = yes
endif
PRIORITY = no
PULSE = no
PVR = no
QTX_CODECS = no
QTX_CODECS_WIN32 = 
QTX_EMULATION = no
QUARTZ = no
RADIO=no
RADIO_CAPTURE=no
REAL_CODECS = yes
S3FB = no
SDL = no
SPEEX = no
STREAM_CACHE = no
SGIAUDIO = no
SUNAUDIO = no
SVGA = no
TDFXFB = no
TDFXVID = no
TGA = no
TOOLAME=no
TREMOR_INTERNAL = yes
TV = no
TV_BSDBT848 = no
TV_DSHOW = no
TV_V4L  = no
TV_V4L1 = no
TV_V4L2 = no
TWOLAME= no
UNRAR_EXEC = no
V4L2 = no
VCD = no
VDPAU = no
VESA = no
VIDIX = no
VIDIX_PCIDB = 
VIDIX_CYBERBLADE=no
VIDIX_IVTV=no
VIDIX_MACH64=no
VIDIX_MGA=no
VIDIX_MGA_CRTC2=no
VIDIX_NVIDIA=no
VIDIX_PM2=no
VIDIX_PM3=no
VIDIX_RADEON=no
VIDIX_RAGE128=no
VIDIX_S3=no
VIDIX_SH_VEU=no
VIDIX_SIS=no
VIDIX_UNICHROME=no
VORBIS = yes
VSTREAM = no
WII = no
WIN32DLL = no
WIN32WAVEOUT = no
WIN32_EMULATION = 
WINVIDIX = 
X11 = no
X264 = no
XANIM_CODECS = no
XMGA = no
XMMS_PLUGINS = no
XV = no
XVID4 = no
XVIDIX = 
XVMC = no
XVR100 = no
YUV4MPEG = yes
ZR = no

# FFmpeg
LIBAVUTIL      = yes
LIBAVUTIL_A    = yes
LIBAVUTIL_SO   = no
LIBAVCODEC     = yes
LIBAVCODEC_A   = yes
LIBAVCODEC_SO  = no
LIBAVCORE      = yes
LIBAVCORE_A    = yes
LIBAVCORE_SO   = no
LIBAVFORMAT    = yes
LIBAVFORMAT_A  = yes
LIBAVFORMAT_SO = no
LIBPOSTPROC    = yes
LIBPOSTPROC_A  = yes
LIBPOSTPROC_SO = no
LIBSWSCALE     = yes
LIBSWSCALE_A   = yes
LIBSWSCALE_SO  = no

HOSTCC     = $(HOST_CC)
HOSTCFLAGS = -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -O3
HOSTLIBS   = -lm
CC_O       = -o $@
LD         = /gcc-mpis/bin/noahos-gcc -mips32
RANLIB     = true
YASM       = yasm
YASMDEP    = yasm
YASMFLAGS  = 

CONFIG_STATIC = yes
SRC_PATH      = ..
BUILD_ROOT    = ..
LIBPREF       = lib
LIBSUF        = .a
LIBNAME       = $(LIBPREF)$(NAME)$(LIBSUF)
FULLNAME      = $(NAME)$(BUILDSUF)

# Some FFmpeg codecs depend on these. Enable them unconditionally for now.
CONFIG_AANDCT  = yes
CONFIG_DCT     = yes
CONFIG_DWT     = yes
CONFIG_FFT     = yes
CONFIG_GOLOMB  = yes
CONFIG_H264DSP = yes
CONFIG_H264PRED= yes
CONFIG_HUFFMAN = yes
CONFIG_LPC     = yes
CONFIG_LSP     = yes
CONFIG_MDCT    = yes
CONFIG_RDFT    = yes

CONFIG_HARDCODED_TABLES = yes
CONFIG_MPEGAUDIO_HP = yes
CONFIG_LIBRTMP  = no

CONFIG_BZLIB    = no
CONFIG_ENCODERS = no
CONFIG_GPL      = yes
CONFIG_MLIB     = no
CONFIG_MUXERS   = no
CONFIG_POSTPROC = yes
CONFIG_VDPAU    = no
CONFIG_XVMC     = no
CONFIG_ZLIB     = yes

HAVE_GNU_AS     = 
HAVE_PTHREADS   = no
HAVE_SHM        = no
HAVE_W32THREADS = no
HAVE_YASM       = 

CONFIG_AASC_DECODER=no
CONFIG_AMV_DECODER=yes
CONFIG_ANM_DECODER=yes
CONFIG_ANSI_DECODER=no
CONFIG_ASV1_DECODER=yes
CONFIG_ASV2_DECODER=yes
CONFIG_AURA_DECODER=no
CONFIG_AURA2_DECODER=no
CONFIG_AVS_DECODER=yes
CONFIG_BETHSOFTVID_DECODER=no
CONFIG_BFI_DECODER=no
CONFIG_BINK_DECODER=no
CONFIG_BMP_DECODER=yes
CONFIG_C93_DECODER=no
CONFIG_CAVS_DECODER=yes
CONFIG_CDGRAPHICS_DECODER=no
CONFIG_CINEPAK_DECODER=no
CONFIG_CLJR_DECODER=no
CONFIG_CSCD_DECODER=no
CONFIG_CYUV_DECODER=yes
CONFIG_DNXHD_DECODER=no
CONFIG_DPX_DECODER=no
CONFIG_DSICINVIDEO_DECODER=no
CONFIG_DVVIDEO_DECODER=yes
CONFIG_DXA_DECODER=no
CONFIG_EACMV_DECODER=no
CONFIG_EAMAD_DECODER=no
CONFIG_EATGQ_DECODER=no
CONFIG_EATGV_DECODER=no
CONFIG_EATQI_DECODER=no
CONFIG_EIGHTBPS_DECODER=no
CONFIG_EIGHTSVX_EXP_DECODER=no
CONFIG_EIGHTSVX_FIB_DECODER=no
CONFIG_ESCAPE124_DECODER=no
CONFIG_FFV1_DECODER=yes
CONFIG_FFVHUFF_DECODER=yes
CONFIG_FLASHSV_DECODER=yes
CONFIG_FLIC_DECODER=yes
CONFIG_FLV_DECODER=yes
CONFIG_FOURXM_DECODER=no
CONFIG_FRAPS_DECODER=no
CONFIG_FRWU_DECODER=no
CONFIG_GIF_DECODER=yes
CONFIG_H261_DECODER=yes
CONFIG_H263_DECODER=yes
CONFIG_H263I_DECODER=yes
CONFIG_H264_DECODER=yes
CONFIG_HUFFYUV_DECODER=yes
CONFIG_IDCIN_DECODER=no
CONFIG_IFF_BYTERUN1_DECODER=no
CONFIG_IFF_ILBM_DECODER=no
CONFIG_INDEO2_DECODER=yes
CONFIG_INDEO3_DECODER=yes
CONFIG_INDEO5_DECODER=yes
CONFIG_INTERPLAY_VIDEO_DECODER=yes
CONFIG_JPEGLS_DECODER=yes
CONFIG_KGV1_DECODER=no
CONFIG_KMVC_DECODER=no
CONFIG_LOCO_DECODER=no
CONFIG_MDEC_DECODER=no
CONFIG_MIMIC_DECODER=no
CONFIG_MJPEG_DECODER=yes
CONFIG_MJPEGB_DECODER=yes
CONFIG_MMVIDEO_DECODER=yes
CONFIG_MOTIONPIXELS_DECODER=yes
CONFIG_MPEG1VIDEO_DECODER=yes
CONFIG_MPEG2VIDEO_DECODER=yes
CONFIG_MPEG4_DECODER=yes
CONFIG_MPEGVIDEO_DECODER=yes
CONFIG_MSMPEG4V1_DECODER=yes
CONFIG_MSMPEG4V2_DECODER=yes
CONFIG_MSMPEG4V3_DECODER=yes
CONFIG_MSRLE_DECODER=yes
CONFIG_MSVIDEO1_DECODER=yes
CONFIG_MSZH_DECODER=no
CONFIG_NUV_DECODER=no
CONFIG_PAM_DECODER=no
CONFIG_PBM_DECODER=no
CONFIG_PCX_DECODER=no
CONFIG_PGM_DECODER=yes
CONFIG_PGMYUV_DECODER=yes
CONFIG_PICTOR_DECODER=yes
CONFIG_PNG_DECODER=no
CONFIG_PPM_DECODER=yes
CONFIG_PTX_DECODER=no
CONFIG_QDRAW_DECODER=no
CONFIG_QPEG_DECODER=no
CONFIG_QTRLE_DECODER=no
CONFIG_R210_DECODER=no
CONFIG_RAWVIDEO_DECODER=yes
CONFIG_RL2_DECODER=no
CONFIG_ROQ_DECODER=no
CONFIG_RPZA_DECODER=no
CONFIG_RV10_DECODER=yes
CONFIG_RV20_DECODER=yes
CONFIG_RV30_DECODER=yes
CONFIG_RV40_DECODER=yes
CONFIG_SGI_DECODER=no
CONFIG_SMACKER_DECODER=no
CONFIG_SMC_DECODER=yes
CONFIG_SNOW_DECODER=yes
CONFIG_SP5X_DECODER=no
CONFIG_SUNRAST_DECODER=no
CONFIG_SVQ1_DECODER=yes
CONFIG_SVQ3_DECODER=no
CONFIG_TARGA_DECODER=no
CONFIG_THEORA_DECODER=no
CONFIG_THP_DECODER=no
CONFIG_TIERTEXSEQVIDEO_DECODER=no
CONFIG_TIFF_DECODER=yes
CONFIG_TMV_DECODER=no
CONFIG_TRUEMOTION1_DECODER=no
CONFIG_TRUEMOTION2_DECODER=no
CONFIG_TSCC_DECODER=yes
CONFIG_TXD_DECODER=no
CONFIG_ULTI_DECODER=no
CONFIG_V210_DECODER=no
CONFIG_V210X_DECODER=no
CONFIG_VB_DECODER=no
CONFIG_VC1_DECODER=yes
CONFIG_VCR1_DECODER=yes
CONFIG_VMDVIDEO_DECODER=no
CONFIG_VMNC_DECODER=no
CONFIG_VP3_DECODER=yes
CONFIG_VP5_DECODER=yes
CONFIG_VP6_DECODER=yes
CONFIG_VP6A_DECODER=yes
CONFIG_VP6F_DECODER=yes
CONFIG_VP8_DECODER=yes
CONFIG_VQA_DECODER=yes
CONFIG_WMV1_DECODER=yes
CONFIG_WMV2_DECODER=yes
CONFIG_WMV3_DECODER=yes
CONFIG_WNV1_DECODER=yes
CONFIG_XAN_WC3_DECODER=no
CONFIG_XL_DECODER=no
CONFIG_YOP_DECODER=no
CONFIG_ZLIB_DECODER=yes
CONFIG_ZMBV_DECODER=no
CONFIG_AAC_DECODER=yes
CONFIG_AC3_DECODER=yes
CONFIG_ALAC_DECODER=no
CONFIG_ALS_DECODER=no
CONFIG_AMRNB_DECODER=no
CONFIG_LIBOPENCORE_AMRNB_DECODER=yes
CONFIG_LIBAMR_NB_DECODER=yes
CONFIG_APE_DECODER=yes
CONFIG_ATRAC1_DECODER=yes
CONFIG_ATRAC3_DECODER=yes
CONFIG_BINKAUDIO_DCT_DECODER=no
CONFIG_BINKAUDIO_RDFT_DECODER=no
CONFIG_COOK_DECODER=yes
CONFIG_DCA_DECODER=yes
CONFIG_DSICINAUDIO_DECODER=no
CONFIG_EAC3_DECODER=yes
CONFIG_FLAC_DECODER=yes
CONFIG_GSM_DECODER=no
CONFIG_GSM_MS_DECODER=no
CONFIG_IMC_DECODER=no
CONFIG_MACE3_DECODER=yes
CONFIG_MACE6_DECODER=yes
CONFIG_MLP_DECODER=yes
CONFIG_MP1_DECODER=yes
CONFIG_MP1FLOAT_DECODER=yes
CONFIG_MP2_DECODER=yes
CONFIG_MP2FLOAT_DECODER=yes
CONFIG_MP3_DECODER=yes
CONFIG_MP3FLOAT_DECODER=yes
CONFIG_MP3ADU_DECODER=yes
CONFIG_MP3ADUFLOAT_DECODER=yes
CONFIG_MP3ON4_DECODER=yes
CONFIG_MP3ON4FLOAT_DECODER=yes
CONFIG_MPC7_DECODER=yes
CONFIG_MPC8_DECODER=yes
CONFIG_NELLYMOSER_DECODER=no
CONFIG_QCELP_DECODER=no
CONFIG_QDM2_DECODER=yes
CONFIG_RA_144_DECODER=yes
CONFIG_RA_288_DECODER=yes
CONFIG_SHORTEN_DECODER=no
CONFIG_SIPR_DECODER=no
CONFIG_SMACKAUD_DECODER=no
CONFIG_SONIC_DECODER=no
CONFIG_TRUEHD_DECODER=no
CONFIG_TRUESPEECH_DECODER=no
CONFIG_TTA_DECODER=yes
CONFIG_TWINVQ_DECODER=no
CONFIG_VMDAUDIO_DECODER=yes
CONFIG_VORBIS_DECODER=yes
CONFIG_WAVPACK_DECODER=yes
CONFIG_WMAPRO_DECODER=yes
CONFIG_WMAV1_DECODER=yes
CONFIG_WMAV2_DECODER=yes
CONFIG_WMAVOICE_DECODER=yes
CONFIG_WS_SND1_DECODER=no
CONFIG_PCM_ALAW_DECODER=no
CONFIG_PCM_BLURAY_DECODER=no
CONFIG_PCM_DVD_DECODER=no
CONFIG_PCM_F32BE_DECODER=yes
CONFIG_PCM_F32LE_DECODER=yes
CONFIG_PCM_F64BE_DECODER=yes
CONFIG_PCM_F64LE_DECODER=yes
CONFIG_PCM_MULAW_DECODER=no
CONFIG_PCM_S8_DECODER=yes
CONFIG_PCM_S16BE_DECODER=yes
CONFIG_PCM_S16LE_DECODER=yes
CONFIG_PCM_S16LE_PLANAR_DECODER=yes
CONFIG_PCM_S24BE_DECODER=yes
CONFIG_PCM_S24DAUD_DECODER=yes
CONFIG_PCM_S24LE_DECODER=yes
CONFIG_PCM_S32BE_DECODER=yes
CONFIG_PCM_S32LE_DECODER=yes
CONFIG_PCM_U8_DECODER=yes
CONFIG_PCM_U16BE_DECODER=yes
CONFIG_PCM_U16LE_DECODER=yes
CONFIG_PCM_U24BE_DECODER=yes
CONFIG_PCM_U24LE_DECODER=yes
CONFIG_PCM_U32BE_DECODER=yes
CONFIG_PCM_U32LE_DECODER=yes
CONFIG_PCM_ZORK_DECODER=no
CONFIG_INTERPLAY_DPCM_DECODER=no
CONFIG_ROQ_DPCM_DECODER=no
CONFIG_SOL_DPCM_DECODER=no
CONFIG_XAN_DPCM_DECODER=no
CONFIG_ADPCM_4XM_DECODER=no
CONFIG_ADPCM_ADX_DECODER=no
CONFIG_ADPCM_CT_DECODER=no
CONFIG_ADPCM_EA_DECODER=no
CONFIG_ADPCM_EA_MAXIS_XA_DECODER=no
CONFIG_ADPCM_EA_R1_DECODER=no
CONFIG_ADPCM_EA_R2_DECODER=no
CONFIG_ADPCM_EA_R3_DECODER=no
CONFIG_ADPCM_EA_XAS_DECODER=no
CONFIG_ADPCM_G726_DECODER=no
CONFIG_ADPCM_IMA_AMV_DECODER=yes
CONFIG_ADPCM_IMA_DK3_DECODER=no
CONFIG_ADPCM_IMA_DK4_DECODER=no
CONFIG_ADPCM_IMA_EA_EACS_DECODER=no
CONFIG_ADPCM_IMA_EA_SEAD_DECODER=no
CONFIG_ADPCM_IMA_ISS_DECODER=no
CONFIG_ADPCM_IMA_QT_DECODER=no
CONFIG_ADPCM_IMA_SMJPEG_DECODER=no
CONFIG_ADPCM_IMA_WAV_DECODER=no
CONFIG_ADPCM_IMA_WS_DECODER=no
CONFIG_ADPCM_MS_DECODER=yes
CONFIG_ADPCM_SBPRO_2_DECODER=no
CONFIG_ADPCM_SBPRO_3_DECODER=no
CONFIG_ADPCM_SBPRO_4_DECODER=no
CONFIG_ADPCM_SWF_DECODER=yes
CONFIG_ADPCM_THP_DECODER=no
CONFIG_ADPCM_XA_DECODER=no
CONFIG_ADPCM_YAMAHA_DECODER=no
CONFIG_DVBSUB_DECODER=no
CONFIG_DVDSUB_DECODER=no
CONFIG_PGSSUB_DECODER=no
CONFIG_XSUB_DECODER=no
CONFIG_MPEG1VIDEO_ENCODER=no
CONFIG_SNOW_ENCODER=no
CONFIG_PNG_ENCODER=no
CONFIG_AAC_PARSER=yes
CONFIG_AC3_PARSER=yes
CONFIG_CAVSVIDEO_PARSER=yes
CONFIG_DCA_PARSER=no
CONFIG_DIRAC_PARSER=no
CONFIG_DNXHD_PARSER=no
CONFIG_DVBSUB_PARSER=no
CONFIG_DVDSUB_PARSER=no
CONFIG_H261_PARSER=yes
CONFIG_H263_PARSER=yes
CONFIG_H264_PARSER=yes
CONFIG_MJPEG_PARSER=yes
CONFIG_MLP_PARSER=yes
CONFIG_MPEG4VIDEO_PARSER=yes
CONFIG_MPEGAUDIO_PARSER=yes
CONFIG_MPEGVIDEO_PARSER=yes
CONFIG_PNM_PARSER=no
CONFIG_VC1_PARSER=yes
CONFIG_VP3_PARSER=yes
CONFIG_VP8_PARSER=yes
CONFIG_AAC_DEMUXER=yes
CONFIG_AC3_DEMUXER=yes
CONFIG_AEA_DEMUXER=no
CONFIG_AIFF_DEMUXER=no
CONFIG_AMR_DEMUXER=yes
CONFIG_ANM_DEMUXER=yes
CONFIG_APC_DEMUXER=no
CONFIG_APE_DEMUXER=yes
CONFIG_APPLEHTTP_DEMUXER=no
CONFIG_ASF_DEMUXER=yes
CONFIG_ASS_DEMUXER=yes
CONFIG_AU_DEMUXER=no
CONFIG_AVI_DEMUXER=yes
CONFIG_AVS_DEMUXER=yes
CONFIG_BETHSOFTVID_DEMUXER=no
CONFIG_BFI_DEMUXER=no
CONFIG_BINK_DEMUXER=no
CONFIG_C93_DEMUXER=no
CONFIG_CAF_DEMUXER=no
CONFIG_CAVSVIDEO_DEMUXER=yes
CONFIG_CDG_DEMUXER=no
CONFIG_DAUD_DEMUXER=no
CONFIG_DIRAC_DEMUXER=no
CONFIG_DNXHD_DEMUXER=no
CONFIG_DSICIN_DEMUXER=no
CONFIG_DTS_DEMUXER=yes
CONFIG_DV_DEMUXER=yes
CONFIG_DXA_DEMUXER=no
CONFIG_EA_DEMUXER=no
CONFIG_EA_CDATA_DEMUXER=no
CONFIG_EAC3_DEMUXER=yes
CONFIG_FFM_DEMUXER=no
CONFIG_FILMSTRIP_DEMUXER=no
CONFIG_FLAC_DEMUXER=yes
CONFIG_FLIC_DEMUXER=yes
CONFIG_FLV_DEMUXER=yes
CONFIG_FOURXM_DEMUXER=no
CONFIG_GSM_DEMUXER=no
CONFIG_GXF_DEMUXER=yes
CONFIG_H261_DEMUXER=yes
CONFIG_H263_DEMUXER=yes
CONFIG_H264_DEMUXER=yes
CONFIG_IDCIN_DEMUXER=no
CONFIG_IFF_DEMUXER=no
CONFIG_IMAGE2_DEMUXER=no
CONFIG_IMAGE2PIPE_DEMUXER=no
CONFIG_INGENIENT_DEMUXER=no
CONFIG_IPMOVIE_DEMUXER=no
CONFIG_ISS_DEMUXER=no
CONFIG_IV8_DEMUXER=no
CONFIG_IVF_DEMUXER=no
CONFIG_LMLM4_DEMUXER=no
CONFIG_M4V_DEMUXER=yes
CONFIG_MATROSKA_DEMUXER=yes
CONFIG_MJPEG_DEMUXER=yes
CONFIG_MLP_DEMUXER=no
CONFIG_MM_DEMUXER=yes
CONFIG_MMF_DEMUXER=yes
CONFIG_MOV_DEMUXER=yes
CONFIG_MP3_DEMUXER=yes
CONFIG_MPC_DEMUXER=yes
CONFIG_MPC8_DEMUXER=yes
CONFIG_MPEGPS_DEMUXER=yes
CONFIG_MPEGTS_DEMUXER=yes
CONFIG_MPEGTSRAW_DEMUXER=yes
CONFIG_MPEGVIDEO_DEMUXER=yes
CONFIG_MSNWC_TCP_DEMUXER=no
CONFIG_MTV_DEMUXER=yes
CONFIG_MVI_DEMUXER=yes
CONFIG_MXF_DEMUXER=no
CONFIG_NC_DEMUXER=no
CONFIG_NSV_DEMUXER=no
CONFIG_NUT_DEMUXER=no
CONFIG_NUV_DEMUXER=no
CONFIG_OGG_DEMUXER=yes
CONFIG_OMA_DEMUXER=yes
CONFIG_PCM_ALAW_DEMUXER=yes
CONFIG_PCM_MULAW_DEMUXER=yes
CONFIG_PCM_F64BE_DEMUXER=yes
CONFIG_PCM_F64LE_DEMUXER=yes
CONFIG_PCM_F32BE_DEMUXER=yes
CONFIG_PCM_F32LE_DEMUXER=yes
CONFIG_PCM_S32BE_DEMUXER=yes
CONFIG_PCM_S32LE_DEMUXER=yes
CONFIG_PCM_S24BE_DEMUXER=yes
CONFIG_PCM_S24LE_DEMUXER=yes
CONFIG_PCM_S16BE_DEMUXER=yes
CONFIG_PCM_S16LE_DEMUXER=yes
CONFIG_PCM_S8_DEMUXER=yes
CONFIG_PCM_U32BE_DEMUXER=yes
CONFIG_PCM_U32LE_DEMUXER=yes
CONFIG_PCM_U24BE_DEMUXER=yes
CONFIG_PCM_U24LE_DEMUXER=yes
CONFIG_PCM_U16BE_DEMUXER=yes
CONFIG_PCM_U16LE_DEMUXER=yes
CONFIG_PCM_U8_DEMUXER=yes
CONFIG_PVA_DEMUXER=no
CONFIG_QCP_DEMUXER=no
CONFIG_R3D_DEMUXER=no
CONFIG_RAWVIDEO_DEMUXER=yes
CONFIG_RL2_DEMUXER=no
CONFIG_RM_DEMUXER=yes
CONFIG_ROQ_DEMUXER=no
CONFIG_RPL_DEMUXER=no
CONFIG_RSO_DEMUXER=no
CONFIG_RTSP_DEMUXER=no
CONFIG_SDP_DEMUXER=no
CONFIG_SEGAFILM_DEMUXER=no
CONFIG_SHORTEN_DEMUXER=no
CONFIG_SIFF_DEMUXER=no
CONFIG_SMACKER_DEMUXER=no
CONFIG_SOL_DEMUXER=no
CONFIG_SOX_DEMUXER=no
CONFIG_SRT_DEMUXER=no
CONFIG_STR_DEMUXER=no
CONFIG_SWF_DEMUXER=yes
CONFIG_THP_DEMUXER=no
CONFIG_TIERTEXSEQ_DEMUXER=no
CONFIG_TMV_DEMUXER=no
CONFIG_TRUEHD_DEMUXER=yes
CONFIG_TTA_DEMUXER=yes
CONFIG_TXD_DEMUXER=no
CONFIG_TTY_DEMUXER=no
CONFIG_VC1_DEMUXER=yes
CONFIG_VC1T_DEMUXER=yes
CONFIG_VMD_DEMUXER=no
CONFIG_VOC_DEMUXER=no
CONFIG_VQF_DEMUXER=no
CONFIG_W64_DEMUXER=no
CONFIG_WAV_DEMUXER=yes
CONFIG_WC3_DEMUXER=no
CONFIG_WSAUD_DEMUXER=no
CONFIG_WSVQA_DEMUXER=no
CONFIG_WV_DEMUXER=no
CONFIG_XA_DEMUXER=no
CONFIG_YOP_DEMUXER=no
CONFIG_YUV4MPEGPIPE_DEMUXER=yes
CONFIG_=yes
CONFIG_CONCAT_PROTOCOL=no
CONFIG_FILE_PROTOCOL=no
CONFIG_GOPHER_PROTOCOL=no
CONFIG_HTTP_PROTOCOL=no
CONFIG_MMSH_PROTOCOL=no
CONFIG_MMST_PROTOCOL=no
CONFIG_MD5_PROTOCOL=yes
CONFIG_PIPE_PROTOCOL=no
CONFIG_RTMP_PROTOCOL=no
CONFIG_RTMPT_PROTOCOL=no
CONFIG_RTMPE_PROTOCOL=no
CONFIG_RTMPTE_PROTOCOL=no
CONFIG_RTMPS_PROTOCOL=no
CONFIG_RTP_PROTOCOL=no
CONFIG_TCP_PROTOCOL=no
CONFIG_UDP_PROTOCOL=no
CONFIG_AAC_ADTSTOASC_BSF=yes
CONFIG_CHOMP_BSF=no
CONFIG_DUMP_EXTRADATA_BSF=no
CONFIG_H264_MP4TOANNEXB_BSF=yes
CONFIG_IMX_DUMP_HEADER_BSF=no
CONFIG_MJPEGA_DUMP_HEADER_BSF=yes
CONFIG_MP3_HEADER_COMPRESS_BSF=yes
CONFIG_MP3_HEADER_DECOMPRESS_BSF=yes
CONFIG_MOV2TEXTSUB_BSF=no
CONFIG_NOISE_BSF=no
CONFIG_REMOVE_EXTRADATA_BSF=no
CONFIG_TEXT2MOVSUB_BSF=no
CONFIG_=yes
