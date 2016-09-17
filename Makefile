PLATFORM = nspire

ifeq ($(PLATFORM), gcw0)
	CC		:= /opt/gcw0-toolchain/usr/bin/mipsel-linux-gcc
	STRIP		:= /opt/gcw0-toolchain/usr/bin/mipsel-linux-strip
	SYSROOT		:= $(shell $(CC) --print-sysroot)
	CFLAGS		:= $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
	CFLAGS		+= -DNO_FRAMELIMIT -DSCREEN_SCALE=1 -DHOME_DIR
	LDFLAGS		:= $(shell $(SYSROOT)/usr/bin/sdl-config --libs) -lm
	RELEASEDIR	:= release
endif

ifeq ($(PLATFORM), a320)
	CC		:= /opt/opendingux-toolchain/usr/bin/mipsel-linux-gcc
	STRIP		:= /opt/opendingux-toolchain/usr/bin/mipsel-linux-strip
	SYSROOT		:= $(shell $(CC) --print-sysroot)
	CFLAGS		:= $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
	CFLAGS		+= -DNO_FRAMELIMIT -DSCREEN_SCALE=1 -DHOME_DIR
	LDFLAGS		:= $(shell $(SYSROOT)/usr/bin/sdl-config --libs) -lm
	TARGET		:= fever.dge
endif

ifeq ($(PLATFORM), mingw32)
	CC		:= i486-mingw32-gcc
	STRIP		:= i486-mingw32-strip
	SYSROOT		:= $(shell $(CC) --print-sysroot)
	CFLAGS		:= -I/usr/i486-mingw32/include -I/usr/i486-mingw32/include/SDL
	LDFLAGS		:= -lmingw32 -lSDLmain -lSDL -lm -mwindows
	TARGET		:= fever.exe
endif

ifeq ($(PLATFORM), nspire)
	CC		:= nspire-gcc
	STRIP		:= 
	SYSROOT		:= $(shell $(CC) --print-sysroot)
	CFLAGS		:= -I$(HOME)/Ndless/ndless-sdk/include/SDL -DSCREEN_SCALE=1
	CFLAGS	+= -Ofast -fdata-sections -ffunction-sections 
	CFLAGS	+= -marm -march=armv5te -mtune=arm926ej-s -fno-ipa-sra -Wall -Wextra
	LDFLAGS		:= -Wl,--as-needed -Wl,--gc-sections -flto -lSDL -lm
	TARGET		:= fever.elf
endif

ifeq ($(PLATFORM), unix)
	CC		= gcc-6
	STRIP		?= strip
	SYSROOT		:= $(shell $(CC) --print-sysroot)
	CFLAGS		?= $(shell sdl-config --cflags) -DSCREEN_SCALE=1 -DJOYSTICK=0
	LDFLAGS		?= $(shell sdl-config --libs) -lm
	TARGET		?= fever.elf
endif

SRCDIR		:= src
OBJDIR		:= obj
SRC		:= $(wildcard $(SRCDIR)/*.c)
OBJ		:= $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

ifeq ($(PLATFORM), nspire)
endif

.PHONY: all opk clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
ifeq ($(PLATFORM), nspire)
	genzehn --input $@ --output fever.t --compress
	make-prg fever.t fever.tns
	rm fever.t $@
endif

$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $@  ${LDFLAGS}

$(OBJDIR):
	mkdir -p $@

opk: $(TARGET)
ifeq ($(PLATFORM), gcw0)
	mkdir -p		$(RELEASEDIR)
	cp $(TARGET)		$(RELEASEDIR)
	cp -R data		$(RELEASEDIR)
	cp platform/gcw0/*	$(RELEASEDIR)
	cp LICENSE.txt		$(RELEASEDIR)
	cp README.md		$(RELEASEDIR)
	mksquashfs		$(RELEASEDIR) homingFever.opk -all-root -noappend -no-exports -no-xattrs
endif

clean:
	rm -Rf $(TARGET) $(OBJDIR) $(RELEASEDIR)

