TARGET = 3ds_receiver

BUILD_PRX = 1
PSP_FW_VERSION = 661

OBJS = main.o

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = \
-lpspdebug \
-lpspkernel \
-lpspdisplay

EXTRA_TARGETS = EBOOT.PBP

PSP_EBOOT_TITLE = 3DS App Receiver v0.5.1

PSPSDK := $(shell psp-config --pspsdk-path)

include $(PSPSDK)/lib/build.mak
