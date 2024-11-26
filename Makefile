# Linux
CC = gcc
DEPS = `pkg-config --cflags SDL2_image SDL2_mixer SDL2_ttf glib-2.0 lua`
DEPS_LIBS = `pkg-config --libs SDL2_image SDL2_mixer SDL2_ttf glib-2.0 lua` -lm
OPTS = -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused #-Werror #-O2
GDB = -g -ggdb
#~ ASAN = -O1 -g3 -ggdb3 -fno-omit-frame-pointer -fsanitize=address -fno-sanitize-recover=all -fsanitize-address-use-after-scope
#~ ASAN_LIBS = -fsanitize=address
K_OBJS = src/kiavc.o src/engine.o src/map.o src/list.o src/scripts.o \
	src/cursor.o src/font.o src/room.o src/actor.o src/costume.o \
	src/object.o src/animation.o src/audio.o src/bag.o \
	src/pathfinding.o src/dialog.o src/utils.o src/logger.o src/plugin.o
KB_OBJS = src/tools/kiavc-bag.o src/bag.o src/map.o src/list.o
KUB_OBJS = src/tools/kiavc-unbag.o src/bag.o src/map.o src/list.o

K_DEPS = $(K_OBJS:.o=.d)

linux: kiavc kiavc-bag kiavc-unbag
linuxso:
	$(MAKE) -C plugins linux

kiavc: $(K_OBJS)
	$(CC) $(GDB) -o $@ $(K_OBJS) $(ASAN_LIBS) $(DEPS_LIBS)

kiavc-bag: $(KB_OBJS)
	$(CC) $(GDB) -o $@ $(KB_OBJS) $(ASAN_LIBS) $(DEPS_LIBS)

kiavc-unbag: $(KUB_OBJS)
	$(CC) $(GDB) -o $@ $(KUB_OBJS) $(ASAN_LIBS) $(DEPS_LIBS)

%.o: %.c
	$(CC) $(ASAN) $(DEPS) $(GDB) -MMD -MP -c $< -o $@ $(OPTS)

-include $(K_DEPS)

# Windows (FIXME)
W32_CC = i686-w64-mingw32-gcc
W32_DEPS = `mingw32-pkg-config --cflags SDL2_image SDL2_mixer SDL2_ttf glib-2.0`
W32_DEPS_LIBS = -w -Wl,-subsystem,windows `mingw32-pkg-config --libs SDL2_image SDL2_mixer SDL2_ttf glib-2.0` -lws2_32 -llua54 -lm -lssp
W32_OPTS = -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused #-Werror #-O2
W32_GDB = -g -ggdb
W32_K_OBJS = src/kiavc.obj src/engine.obj src/map.obj src/list.obj \
	src/scripts.obj src/cursor.obj src/font.obj src/room.obj \
	src/actor.obj src/costume.obj src/object.obj src/animation.obj \
	src/audio.obj src/bag.obj src/pathfinding.obj src/dialog.obj \
	src/utils.obj src/logger.obj src/plugin.obj
W32_KB_OBJS = src/tools/kiavc-bag.obj src/bag.obj src/map.obj src/list.obj
W32_KUB_OBJS = src/tools/kiavc-unbag.obj src/bag.obj src/map.obj src/list.obj

win32: kiavc.exe kiavc-bag.exe kiavc-unbag.exe
win32dll:
	$(MAKE) -C plugins win32

kiavc.exe: $(W32_K_OBJS)
	$(W32_CC) $(W32_GDB) -o $@ $(W32_K_OBJS) $(W32_DEPS_LIBS) -ldl

kiavc-bag.exe: $(W32_KB_OBJS)
	$(W32_CC) $(W32_GDB) -o $@ $(W32_KB_OBJS) $(W32_DEPS_LIBS)

kiavc-unbag.exe: $(W32_KUB_OBJS)
	$(W32_CC) $(W32_GDB) -o $@ $(W32_KUB_OBJS) $(W32_DEPS_LIBS)

%.obj: %.c
	$(W32_CC) $(W32_DEPS) $(W32_GDB) -c $< -o $@ $(W32_OPTS)

# Mac OS (TODO)


# All targets
all: linux

clean:
	rm -f kiavc kiavc-bag kiavc-unbag kiavc.exe kiavc-bag.exe kiavc-unbag.exe \
		src/*.d src/*.o src/*.obj \
		src/tools/*.d src/tools/*.o src/tools/*.obj && make -C plugins clean
