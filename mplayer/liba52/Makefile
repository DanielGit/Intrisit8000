include ../config.mak
LIBA52 = liba52.a
CFLAGS += -DLIBA52_FIXED -DA52_STEREO3
SRCS_LIBA52 = bit_allocate.c\
	      bitstream.c \
	      cpu_accel.c \
	      cpu_state.c \
	      crc.c \
              downmix.c \
              imdct.c \
              parse.c \
	      resample.c \

OBJS_LIBA52   = $(addsuffix .o, $(basename $(SRCS_LIBA52)) )
MIDS_LIBA52   = $(OBJS_LIBA52:.o=.s)
ASMS_LIBA52   = $(OBJS_LIBA52:.o=.mid)

$(LIBA52):	$(OBJS_LIBA52)
		rm -f $@
		$(AR) $@ $^

ifneq           ($(JZC_MINIOS),1)
$(OBJS_LIBA52):%.o:%.c
		$(CC) $(CFLAGS) -c -o $@ $<
else
$(OBJS_LIBA52):%.o:%.s
		$(CC) $(CFLAGS) -c -o $@ $<
$(MIDS_LIBA52):%.s:%.mid
		mxu_as $< > $@
$(ASMS_LIBA52):%.mid:%.c
		$(CC) $(CFLAGS) -S -o $@ $<
endif

clean::
		rm -f *.o *~ *.a *.lib *.so *.so.* *.dylib *.dll *.s *.mid\
		*.def *.dll.a *.exp
