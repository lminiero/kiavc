# Linux
CC = gcc
DEPS = `pkg-config --cflags SDL2_image SDL2_mixer SDL2_ttf glib-2.0 lua`
DEPS_LIBS = `pkg-config --libs SDL2_image SDL2_mixer SDL2_ttf glib-2.0 lua` -lm
OPTS = -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused #-Werror #-O2
GDB = -g -ggdb
#~ ASAN = -O1 -g3 -ggdb3 -fno-omit-frame-pointer -fsanitize=address -fno-sanitize-recover=all -fsanitize-address-use-after-scope
#~ ASAN_LIBS = -fsanitize=address
K_OBJS = src/helloworld.o

K_DEPS = $(K_OBJS:.o=.d)

linux: libkiavc_helloworld.so

libkiavc_helloworld.so: $(K_OBJS)
	$(CC) $(GDB) -fPIC -shared -o $@ $(K_OBJS) $(ASAN_LIBS) $(DEPS_LIBS)

%.o: %.c
	$(CC) $(ASAN) $(DEPS) $(GDB) -fPIC -MMD -MP -c $< -o $@ $(OPTS)

-include $(K_DEPS)

# Windows (FIXME)
W32_CC = i686-w64-mingw32-gcc
W32_DEPS = `mingw32-pkg-config --cflags SDL2_image SDL2_mixer SDL2_ttf glib-2.0`
W32_DEPS_LIBS = -w -Wl,-subsystem,windows `mingw32-pkg-config --libs SDL2_image SDL2_mixer SDL2_ttf glib-2.0` -lws2_32 -llua54 -lm -lssp
W32_OPTS = -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused #-Werror #-O2
W32_GDB = -g -ggdb
W32_K_OBJS = src/helloworld.obj

win32: libkiavc_helloworld.dll

libkiavc_helloworld.dll: $(W32_K_OBJS)
	$(W32_CC) $(W32_GDB) -fPIC -shared -o $@ $(W32_K_OBJS) $(W32_DEPS_LIBS)

%.obj: %.c
	$(W32_CC) $(W32_DEPS) -fPIC -shared $(W32_GDB) -c $< -o $@ $(W32_OPTS)

# Mac OS (TODO)


# All targets
all: linux

clean:
	rm -f *.so *.dll src/*.d src/*.o src/*.obj
