TARGET = 3ds_receiver

BUILD_PRX = 1
PSP_FW_VERSION = 661

OBJS = \
main.o \
dialog.o \
network.o

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = \
-lpspdebug \
-lpspdisplay \
-lpspctrl \
-lpspnet \
-lpspnet_inet \
-lpspnet_apctl \
-lpspnet_resolver \
-lpsputility \
-lpspgu \
-lpspgum \
-lpspge \
-lpspvfpu \
-lpspkernel

EXTRA_TARGETS = EBOOT.PBP

PSP_EBOOT_TITLE = 3DS App Receiver v0.5

PSPSDK := $(shell psp-config --pspsdk-path)

include $(PSPSDK)/lib/build.mak
