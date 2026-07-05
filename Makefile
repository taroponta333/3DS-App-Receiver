TARGET = 3ds_receiver
OBJS = main.o

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

# ネットワーク機能を使うために必要なライブラリ
LIBS = -lpspnet_apctl -lpspnet_resolver -lpspnet_inet -lpspnet -lpsputility

# 最終成果物として EBOOT.PBP を生成する命令
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = 3DS File Receiver

# ✨ 最新コンテナの環境変数をそのまま使う最も安全な指示
include /usr/local/pspdev/psp/sdk/lib/build.make
