################################## PLATFORM #######################################
#JZC_LINUX 	= 1
#JZC_UCOS 	= 0
#JZC_FPGA        = 0
#JZC_RTL		= 0
################################## TOOL CHAIN #####################################
#AR 		= mipsel-linux-ar
#LD 		= mipsel-linux-ld
#CC 		= mipsel-linux-gcc 
#RANLIB 		= mipsel-linux-ranlib

include ../config.mak

CFLAGS 		= -mips32 -O2 -fno-builtin \
		  -fno-exceptions -ffunction-sections -falign-functions=32 \
	   	  -fshort-wchar -fomit-frame-pointer -ffast-math -Wall \
                     -mlong-calls -Wdisabled-optimization -Wno-switch \
		  -Wpointer-arith -Wredundant-decls -pipe -fomit-frame-pointer \
                     -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
                     -D_LARGEFILE64_SOURCE -DHAVE_CONFIG_H -DHAVE_AV_CONFIG_H \
                     -D_ISOC9X_SOURCE -DJZC_MXU_OPT 

ifeq 		($(JZC_LINUX),1)
CFLAGS		+= -fPIC
CFLAGS		+= -D_LINUX_
endif

ifeq 		($(JZC_MINIOS),1)
CFLAGS             += -mno-abicalls -fno-pic -G 0
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
SRCS_DIR 	= .
REL_DIR 		=./rel

LIB		= libmpeg2.a

SRCS_VD_MPEG2 	= $(SRCS_DIR)/slice.c $(SRCS_DIR)/alloc.c  $(SRCS_DIR)/idct_mmx.c $(SRCS_DIR)/motion_comp.c\
		  $(SRCS_DIR)/cpu_accel.c $(SRCS_DIR)/cpu_state.c $(SRCS_DIR)/decode.c  $(SRCS_DIR)/header.c $(SRCS_DIR)/idct.c
OBJS_VD_MPEG2 	= $(addsuffix .o, $(basename $(SRCS_VD_MPEG2)) )

OBJS 		= $(OBJS_VD_MPEG2)
MIDS_VD_MPEG2 	= $(OBJS_VD_MPEG2:.o=.s)
ASMS_VD_MPEG2 	= $(OBJS_VD_MPEG2:.o=.mid)

CFLAGS 		+= -I. -I.. -I../libavcodec -I../libavutil

STATIC_OBJS 	:= $(OBJS)


all: 		$(REL_DIR) $(LIB) $(EXT_BIN) Makefile
		@test -d $(REL_DIR)
#		cp $(LIB) $(REL_DIR)

$(LIB): 		$(STATIC_OBJS)
		rm -f $@
		$(AR) $@ $^ 
		$(RANLIB) $@

$(REL_DIR):
		test -d $(REL_DIR) || mkdir $(REL_DIR)

ifneq 		($(JZC_MINIOS),1)
$(OBJS_VD_MPEG2):%.o:%.c
		$(CC) $(CFLAGS) -c -o $@ $<
else
$(OBJS_VD_MPEG2):%.o:%.s
		$(CC) $(CFLAGS) -c -o $@ $<
$(MIDS_VD_MPEG2):%.s:%.mid
		mxu_as $< > $@
$(ASMS_VD_MPEG2):%.mid:%.c
		$(CC) $(CFLAGS) -S -o $@ $<
endif

clean::
		rm -f *.o *~ *.a *.lib *.so *.so.* *.dylib *.dll *.s *.mid\
	      	*.def *.dll.a *.exp
		rm -fr $(REL_DIR)
#		@for d in $(EXT_BIN_DIR);do make -C $$d  clean;done
