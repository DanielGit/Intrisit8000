################################## PLATFORM #######################################
#JZC_LINUX 	= 0
#JZC_UCOS 	= 0
#JZC_FPGA	= 1
#JZC_RTL		= 0

################################## TOOL CHAIN #####################################
#AR 		= mipsel-linux-ar
#LD 		= mipsel-linux-ld
#CC 		= mipsel-linux-gcc 
#RANLIB 		= mipsel-linux-ranlib

include ../config.mak

CFLAGS 		= -mips32 -std=gnu99 -O2 -fno-builtin \
		  -fno-exceptions -ffunction-sections -falign-functions=32 \
	   	  -fshort-wchar -fomit-frame-pointer -ffast-math -Wall \
                     -mlong-calls -Wdisabled-optimization -Wno-switch \
		  -Wpointer-arith -Wredundant-decls -pipe -fomit-frame-pointer \
                     -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
                     -D_LARGEFILE64_SOURCE -DHAVE_CONFIG_H -DHAVE_AV_CONFIG_H \
                     -D_ISOC9X_SOURCE

ifeq 		($(JZC_LINUX),1)
CFLAGS		+= -fPIC
CFLAGS		+= -D_LINUX_
endif

ifeq 		($(JZC_MINIOS),1)
CFLAGS		+= -mno-abicalls -fno-pic -G 0
CFLAGS		+= -D__MINIOS__ -D_UCOS_
endif

ifeq 		($(JZC_FPGA),1)
CFLAGS		+= -fPIC 
CFLAGS		+= -D_FPGA_TEST_
endif

ifeq 		($(JZC_RTL),1)
CFLAGS		+= -fPIC 
CFLAGS		+= -D_RTL_SIM_
endif

################################## COMPILE  DEF #####################################
VC1_DIR         =./

JZC_CFG 	= vc1_config.h
LIB		= libvc1.a

JZSOC_HEAD 	= jzsoc/vc1_dcore.h jzsoc/vc1_tcsm0.h
SRCS_VD_VC1 	= $(VC1_DIR)/vc1.c  $(VC1_DIR)/vc1data.c\
	          $(VC1_DIR)/vc1dsp.c $(VC1_DIR)/vc1_parser.c

OBJS_VD_VC1 	= $(addsuffix .o, $(basename $(SRCS_VD_VC1)) )

OBJS 		= $(OBJS_VD_VC1)
MIDS_VD_VC1 	= $(OBJS_VD_VC1:.o=.s)
ASMS_VD_VC1 	= $(OBJS_VD_VC1:.o=.mid)
OBJS		+=vc1dec.o
MIDS_VD_VC1 	+=vc1dec.s

CFLAGS 		+= -imacros $(JZC_CFG) -I$(VC1_DIR)/jzsoc -I. -I.. \
                   -I../libavcodec -I../libavutil

STATIC_OBJS 	:= $(OBJS)

EXT_BIN_DIR 	:= ./jzsoc/
EXT_BIN_NAME 	:= vc1_p1.bin
EXT_BIN		= $(join $(EXT_BIN_DIR),$(EXT_BIN_NAME))

all: 		$(LIB) $(EXT_BIN)

$(LIB):		$(STATIC_OBJS) $(JZSOC_HEAD)
		rm -f $@
		$(AR) $@ $(STATIC_OBJS)
		$(RANLIB) $@

ifneq 		($(JZC_MINIOS),1)
$(OBJS)	      :%.o:%.c $(JZSOC_HEAD)
		$(CC) $(CFLAGS) -c -o $@ $<
else
$(OBJS)	      :%.o:%.s
		$(CC) $(CFLAGS) -c -o $@ $<
$(MIDS_VD_VC1):%.s:%.mid
		mxu_as $< > $@
vc1dec.mid    :vc1dec.c  $(JZSOC_HEAD)
		$(CC) $(CFLAGS) -S -o $@ $<
$(ASMS_VD_VC1):%.mid:%.c
		$(CC) $(CFLAGS) -S -o $@ $<
endif

$(EXT_BIN):	$(EXT_BIN_DIR)
		rm -f $(EXT_BIN:.bin=.s)
		$(MAKE) -C $(EXT_BIN_DIR)
#		cp -f $(EXT_BIN) ../
		@echo "Information: vc1 p1 bin generated successfully!"


clean::
		-rm -f *.o *~ *.a *.lib *.so *.so.* *.dylib *.dll *.s *.mid\
	      	*.def *.dll.a *.exp
		$(MAKE) -C $(EXT_BIN_DIR) clean
