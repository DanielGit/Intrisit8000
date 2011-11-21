############################################################################### 
# 				             NOAH-TECH Co,LTD
#                               2009-01-07
# ʹ�÷���: 
#    make            ���벢���ӳ���
#    make objs       ����Ŀ���ļ����������ӳ���
#    make clean      ���Ŀ���ļ��������ļ��ͳ����ļ�
#    make rebuild    ��ͬ��make clean && make all
#    make lib	     ���ɿ��ļ�
############################################################################### 

# �����ļ�����
TARGET  := _noahos

# ���ӽű��ļ�
LDS     := link.xn

# Դ�ļ�Ŀ¼
TOP	:= ../..
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cfgsys)
ifneq ($(MAKECMDGOALS),cfgbios)
ifneq ($(MAKECMDGOALS),cfgburn)
SRCDIRS := 

SRCDIRS += $(TOP)/Direct
SRCDIRS += $(TOP)/Direct/Media/JZ4760B
SRCDIRS += $(TOP)/Direct/Media/MediaLib
SRCDIRS += $(TOP)/Direct/Media/MediaLib/Mp3Dec
SRCDIRS += $(TOP)/Direct/Media/MediaLib/Midi

endif
endif
endif
endif


# Դ�ļ����ͣ�֧������Դ�ļ����ͣ�
# .s, .S, .c, .C, .cc, .cpp, .CPP, .c++, .cp, or .cxx. 
SRCEXTS    := .S .c

# Ԥ�������
CPPFLAGS   := 

# ���������
ASFLAGS    := -D_ASSEMBLER_ -D__ASSEMBLY__

# C�������
CFLAGS     := -mips32 -O2 -mno-abicalls -fno-pic -fno-builtin 
CFLAGS     += -fno-exceptions -Wall  
CFLAGS     += -fomit-frame-pointer -msoft-float -G 0 
CFLAGS     += -I../../include -I../../include/font
CFLAGS     += -DSTC_EXP -DKERNEL

# C++�������
CXXFLAGS   := 
CXXFLAGS   += 

# ���Ӳ���
LDFLAGS    := -nostdlib -T $(LDS)

# ���ļ�
MLIBS := mplayer_lib.a libavcodec.a libavcore.a libavformat.a libswscale.a libavutil.a libpostproc.a libmad.a libh264.a libmpeg4.a librv9.a libvc1.a spc.a fipop.a libmpeg2.a liba52.a

LIBS  = $(addprefix $(TOP)/UsrLib/libout/JZ4760B/, $(MLIBS))
LIBS 	+= _noahos.a $(TOP)/Usrlib/libout/libgcc_mipsel.a $(TOP)/Usrlib/libout/mips_klibc.a

ifeq ($(MAKECMDGOALS), drv)
LIBS 	+= $(TARGET).a
endif

# ���빤��·��
CC      := /gcc-mpis/bin/noahos-gcc 
CXX     := /gcc-mpis/bin/noahos-g++ 
AR		:= /gcc-mpis/bin/noahos-ar rcsv
LD		:= /gcc-mpis/bin/noahos-ld
OBJCOPY	:= /gcc-mpis/bin/noahos-objcopy
NM		:= /gcc-mpis/bin/noahos-nm
OBJDUMP	:= /gcc-mpis/bin/noahos-objdump
RM      := rm -fr 

# �����������У�����C�ļ���C++����
#CC       = $(CXX) 
#CFLAGS   = $(CXXFLAGS) 


## ��ȡ����Դ�ļ�/Ŀ���ļ�
#--------------------------------------------------- 
SHELL   = /bin/sh 
VPATH   := $(SRCDIRS)
PROGRAM = $(addsuffix .elf, $(TARGET))
PROGRAM = $(addsuffix .elf, $(TARGET))
SOURCES = $(foreach d, $(SRCDIRS), $(wildcard $(addprefix $(d)/*, $(SRCEXTS))))
OBJS    = boot.o $(addsuffix .o, $(basename $(notdir $(SOURCES))))
DEPS    = $(patsubst %.o, %.d, $(OBJS)) 

.PHONY : all objs clean cleanall rebuild lib bios cfgbios

# ִ������. 
#---------------------------------------------- 
all : $(PROGRAM)
	@echo 
	@echo Making logo application...
	@cp $(TOP)/AppLogo/main.c $(TOP)/AppLogo/main.tmp
	@mv $(TOP)/AppLogo/main.tmp $(TOP)/AppLogo/main.c
	@cd $(TOP)/AppLogo/build/MIPS && make
	@cp -p $(TOP)/AppLogo/build/MIPS/logo.bin ./
#mplayerû����ֲ���, ����������5��.
#	make_3.81 -C plugin
#	@echo 
#	@echo Creating binary...
#	cp ./plugin/mplayer_plugin.bin ./
#########
	$(OBJCOPY) --remove-section .plugin -O binary $(PROGRAM) $(TARGET).bin
#	$(OBJDUMP) -d $(PROGRAM) > $(TARGET).dump
	$(NM) $(PROGRAM) | sort > $(TARGET).sym
	$(OBJDUMP) -h $(PROGRAM) > $(TARGET).map
	@sh _release
#	cp $(TARGET).bin $(TOP)/ToolChain/uboot-fix/JZ4760B/

boot.o: boot.xo
	cp -fv boot.xo boot.o	

cfgsys : 
	cp -p $(TOP)/Include/config/jz4760b.h $(TOP)/Include/config.h

bios : $(PROGRAM) Loader.bin
	@echo 
	@echo Making bootloader...
	@make -C $(TOP)/Bootloader/JZ4760B/build_boot clean
	@make -C $(TOP)/Bootloader/JZ4760B/build_boot
	@cp $(TOP)/Bootloader/JZ4760B/build_boot/boot.bin ./

	@make -C $(TOP)/Bootloader/JZ4760B/build_loader clean
	@make -C $(TOP)/Bootloader/JZ4760B/build_loader
	@cp $(TOP)/Bootloader/JZ4760B/build_loader/loader.bin ./

	@echo 
	@echo Making bios application...
	@cd $(TOP)/AppLogo/build/MIPS && make bios
	@cp $(TOP)/AppLogo/bios/build/MIPS/BiosApp.bin ./
	@echo 
	@echo Creating binary...
	$(OBJCOPY) -O binary $(PROGRAM) $(TARGET).bin
#	$(OBJDUMP) -d $(PROGRAM) > $(TARGET).dump
	$(NM) $(PROGRAM) | sort > $(TARGET).sym
	$(OBJDUMP) -h $(PROGRAM) > $(TARGET).map
	@sh _bios
#	cp $(TARGET).bin $(TOP)/ToolChain/uboot-fix/JZ4760B/
	
cfgbios :
	mv -f $(TOP)/Include/config.h $(TOP)/Include/config.h.old
	sed 's/^\/\/#define CONFIG_MAKE_BIOS\|^\/\/#define CONFIG_MAKE_BURN_TOOL\|^#define CONFIG_MAKE_BURN_TOOL/#define CONFIG_MAKE_BIOS/' $(TOP)/Include/config.h.old > $(TOP)/include/config.h
	cat $(TOP)/include/config.h | grep CONFIG_MAKE


$(PROGRAM) : $(OBJS)  $(LIBS)
	@echo 
	@echo Linking...
	$(CC) -o $(PROGRAM) $(OBJS) $(LIBS) $(LIBS) $(LDFLAGS) 

lib : $(OBJS)
	$(AR) $(TARGET).a $(OBJS)

drv : $(PROGRAM) 
	@echo 
	@echo Making logo application...
	@cp $(TOP)/AppLogo/main.c $(TOP)/AppLogo/main.tmp
	@mv $(TOP)/AppLogo/main.tmp $(TOP)/AppLogo/main.c
	@cd $(TOP)/AppLogo/build/MIPS && make
	@cp -p $(TOP)/AppLogo/build/MIPS/logo.bin ./
	@echo 
	@echo Creating binary...
	$(OBJCOPY) -O binary $(PROGRAM) $(TARGET).bin
	$(OBJDUMP) -d $(PROGRAM) > $(TARGET).dump
	$(NM) $(PROGRAM) | sort > $(TARGET).sym
	$(OBJDUMP) -h $(PROGRAM) > $(TARGET).map
	@sh _release

objs : $(OBJS) 

rebuild: clean all 

clean : 
	$(RM) *.o *.d $(TARGET).elf $(TARGET).bin *.bin *.sym *.map #$(TARGET).*

# ���������ļ�����
#--------------------------------------------------- 
%.d : %.s 
	@echo DEPENDING $<
	@$(CC) -MM -MD $(CFLAGS)$(ASFLAGS) $< 

%.d : %.S 
	@echo DEPENDING $<
	@$(CC) -MM -MD $(CFLAGS) $(ASFLAGS) $< 

%.d : %.c 
	@echo DEPENDING $<
	@$(CC) -MM -MD $(CFLAGS) $< 

%.d : %.C 
	@echo DEPENDING $<
	@$(CC) -MM -MD $(CXXFLAGS) $< 

%.d : %.cc 
	$(CC) -MM -MD $(CXXFLAGS) $< 

%.d : %.cpp 
	$(CC) -MM -MD $(CXXFLAGS) $< 

%.d : %.CPP 
	$(CC) -MM -MD $(CXXFLAGS) $< 

%.d : %.c++ 
	$(CC) -MM -MD $(CXXFLAGS) $< 

%.d : %.cp 
	$(CC) -MM -MD $(CXXFLAGS) $< 

%.d : %.cxx 
	$(CC) -MM -MD $(CXXFLAGS) $< 


# ����Ŀ���ļ�����
#--------------------------------------------------- 
%.o : %.s
	@echo COMPILING $<
	@$(CC) $(CFLAGS) $(ASFLAGS) -c $<

%.o : %.S
	@echo COMPILING $<
	@$(CC) $(CFLAGS) $(ASFLAGS) -c $<

%.o : %.c
	@echo COMPILING $<
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

%.o : %.C 
	@echo COMPILING $<
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

%.o : %.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

%.o : %.cpp 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

%.o : %.CPP 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

%.o : %.c++ 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

%.o : %.cp 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

%.o : %.cxx 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

# ȱʡ����
#--------------------------------------------------- 
.DEFAULT:;

-include $(DEPS) 
