TARGET = 3ds_receiver
OBJS = main.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -lpspnet_apctl -lpspnet_resolver -lpspnet_inet -lpspnet

# ✨ ここで公式の見た目に完全偽装します！
PSP_EBOOT_TITLE = デジタルコミックリーダー
# もし公式のICON0.PNGを用意したら、下の行の「#」を消して同じフォルダに置いてください
# PSP_EBOOT_ICON = ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
