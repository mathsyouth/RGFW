# CUSTOM ARGS :
# RGFW_WAYLAND=1 -> use wayland 
# NO_VULKAN=1 -> do not compile the vulkan example
# NO_GLES=1 -> do not compile the gles example (on by default for non-linux OSes)

CC = gcc
AR = ar

# used for compiling RGFW.o
CUSTOM_CFLAGS =
# used for the examples
CFLAGS = 

DX11_LIBS = -static -lgdi32 -lm -lwinmm -ldxgi -ld3d11 -luuid -ld3dcompiler
VULKAN_LIBS = -lgdi32 -lm -lwinmm -I $(VULKAN_SDK)\Include -L $(VULKAN_SDK)\Lib -lvulkan-1
LIBS := -static -lgdi32 -lm -lopengl32 -lwinmm -ggdb
EXT = .exe
LIB_EXT = .dll

LIBS += -D _WIN32_WINNT="0x0501"

WARNINGS =  -Wall -Wstrict-prototypes -Wextra -Wstrict-prototypes -Wold-style-definition -Wno-missing-field-initializers -Wno-unknown-pragmas -Wno-missing-braces -Wno-missing-variable-declarations -Wno-redundant-decls -Wno-unused-function -Wno-unused-label -Wno-unused-result -Wno-incompatible-pointer-types -Wno-format -Wno-format-extra-args -Wno-implicit-function-declaration -Wno-implicit-int -Wno-pointer-sign -Wno-switch -Wno-switch-default -Wno-switch-enum -Wno-unused-value -Wno-type-limits
OS_DIR = \\

NO_GLES = 1
detected_OS = windows

OBJ_FILE = .o

# not using a cross compiler
ifeq (,$(filter $(CC),x86_64-w64-mingw32-gcc i686-w64-mingw32-gcc x86_64-w64-mingw32-g++ /opt/msvc/bin/x64/cl.exe /opt/msvc/bin/x86/cl.exe))
	detected_OS := $(shell uname 2>/dev/null || echo Unknown)

	ifeq ($(detected_OS),Darwin)        # Mac OS X
		LIBS := -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo
		VULKAN_LIBS = -lm  -framework Foundation -framework AppKit --framework CoreVideo -lvulkan
		EXT =
		LIB_EXT = .dylib
		OS_DIR = /
	endif
	ifeq ($(detected_OS),Linux)
    	LIBS := -lXrandr -lX11 -lm -lGL -ldl -lpthread
		VULKAN_LIBS = -lX11 -lXrandr -lm -ldl -lpthread -lvulkan
		EXT =
		LIB_EXT = .so
		OS_DIR = /
		NO_GLES = 0
	endif
else
	OS_DIR = /
endif

ifeq ($(RGFW_WAYLAND),1)
	LIBS = -D RGFW_WAYLAND relative-pointer-unstable-v1-client-protocol.c xdg-decoration-unstable-v1.c xdg-shell.c -lwayland-cursor -lwayland-client -lEGL -lxkbcommon -lGL -lwayland-egl -lm
endif

LINK_GL1 =
LINK_GL3 =
LINK_GL2 =

ifneq (,$(filter $(CC),cl /opt/msvc/bin/x64/cl.exe /opt/msvc/bin/x86/cl.exe))
	WARNINGS =
	LIBS = /static
	DX11_LIBS =
	VULKAN_LIBS = 
	OBJ_FILE = .obj
else ifeq ($(CC),emcc)
	LINK_GL1 = -s LEGACY_GL_EMULATION -D LEGACY_GL_EMULATION -sGL_UNSAFE_OPTS=0
	LINK_GL3 = -s FULL_ES3 -s USE_WEBGL2 
	LINK_GL2 = -s FULL_ES2 -s USE_WEBGL2 
	EXPORTED_JS = -s EXPORTED_RUNTIME_METHODS="['stringToNewUTF8']"
	LIBS = -s WASM=1 -s ASYNCIFY -s GL_SUPPORT_EXPLICIT_SWAP_CONTROL=1 $(EXPORTED_JS)
	EXT = .js
	NO_GLES = 0
	detected_OS = web
else
	LIBS += -std=c99 
endif

VULKAN_CHECK := $(shell command -v vulkan)
ifneq ($(VULKAN_CHECK),)
	NO_VULKAN = 0
else
	NO_VULKAN = 1
endif

EXAMPLE_OUTPUTS = \
    examples/basic/basic \
    examples/buffer/buffer \
	examples/silk/silk \
	examples/events/events \
	examples/callbacks/callbacks \
	examples/first-person-camera/camera

EXAMPLE_OUTPUTS_CUSTOM = \
	examples/microui_demo/microui_demo \
	examples/gl33/gl33 \
	examples/portableGL/pgl \
	examples/gles2/gles2 \
	examples/vk10/vk10 \
	examples/dx11/dx11 \
	examples/metal/metal

all: xdg-shell.c $(EXAMPLE_OUTPUTS) $(EXAMPLE_OUTPUTS_CUSTOM) libRGFW$(LIB_EXT) libRGFW.a

examples: $(EXAMPLE_OUTPUTS) $(EXAMPLE_OUTPUTS_CUSTOM)

examples/portableGL/pgl: examples/portableGL/pgl.c RGFW.h
ifneq ($(CC), emcc)
	$(CC)  -w $(CFLAGS) -I. $< $(LIBS) -o $@ 
else
	@echo "the portableGL example doesn't support html5"
endif

examples/gles2/gles2: examples/gles2/gles2.c RGFW.h
ifneq ($(NO_GLES), 1)
	$(CC)  $(CFLAGS) -I. $< $(LIBS) $(LINK_GL2) -lEGL -o $@$(EXT)
else
	@echo gles has been disabled
endif

examples/vk10/vk10: examples/vk10/vk10.c RGFW.h
ifneq ($(NO_VULKAN), 1)
	glslangValidator -V examples/vk10/shaders/vert.vert -o examples/vk10/shaders/vert.h --vn vert_code
	glslangValidator -V examples/vk10/shaders/frag.frag -o examples/vk10/shaders/frag.h --vn frag_code

	$(CC)  $(CFLAGS) -I. $< $(VULKAN_LIBS) -o $@
else
	@echo vulkan has been disabled
endif


examples/dx11/dx11: examples/dx11/dx11.c RGFW.h
ifneq (,$(filter $(detected_OS), windows Windows_NT))
	$(CC) $(CFLAGS) -I. $<  $(DX11_LIBS) -o $@
else
	@echo directX is not supported on $(detected_OS)
endif


examples/metal/metal: examples/metal/metal.m RGFW.h
ifeq ($(detected_OS),Darwin)        # Mac OS X
	gcc $(CUSTOM_CFLAGS) -x c -c RGFW.h -D RGFW_NO_API -D RGFW_EXPORT -D RGFW_IMPLEMENTATION -o RGFW.o
	gcc $(CUSTOM_CFLAGS) examples/metal/metal.m RGFW.o -I. -lm -framework Metal -framework Foundation -framework AppKit -framework Cocoa -framework CoreVideo -framework QuartzCore -o $@
else
	@echo metal is not supported on $(detected_OS)
endif


examples/microui_demo/microui_demo: examples/microui_demo/microui_demo.c RGFW.h
ifneq ($(CC), emcc)
	$(CC) $(CFLAGS) -I. $< $(LIBS) $(LINK_GL1) -o $@$(EXT)
else
	$(CC) $(CFLAGS) -I. $< -s USE_WEBGL2 $(LIBS) $(LINK_GL1) -o $@$(EXT)
endif

examples/gl33/gl33: examples/gl33/gl33.c RGFW.h
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< $(LIBS) $(LINK_GL3) -o $@$(EXT)

$(EXAMPLE_OUTPUTS): %: %.c RGFW.h
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< $(LIBS) $(LINK_GL1)  -o $@$(EXT)

debug: all
	@for exe in $(EXAMPLE_OUTPUTS); do \
		echo "Running $$exe..."; \
		.$(OS_DIR)$$exe$(EXT); \
	done

	./examples/portableGL/pgl$(EXT)
	./examples/gl33/gl33$(EXT)
ifneq ($(NO_GLES), 1)
		./examples/gles2/gles2$(EXT)
endif
ifneq ($(NO_VULKAN), 1)
		./examples/vk10/vk10$(EXT)
endif
ifeq ($(detected_OS), windows)
		./examples/dx11/dx11.exe
endif
	make clean


RGFW$(OBJ_FILE): RGFW.h
	make initwayland
	#$(CC) -x c $(CUSTOM_CFLAGS) -c RGFW.h -D RGFW_IMPLEMENTATION -fPIC -D RGFW_EXPORT
	cp RGFW.h RGFW.c
	$(CC) $(CUSTOM_CFLAGS) -c RGFW.c -D RGFW_IMPLEMENTATION -fPIC -D RGFW_EXPORT
	rm RGFW.c

libRGFW$(LIB_EXT): RGFW.h RGFW$(OBJ_FILE)
	make RGFW$(OBJ_FILE)
ifeq ($(CC), cl)
	link /DLL /OUT:libRGFW.dll RGFW.obj
else
	$(CC) $(CUSTOM_CFLAGS) -shared RGFW$(OBJ_FILE) $(LIBS) -o libRGFW$(LIB_EXT)
endif

libRGFW.a: RGFW.h RGFW$(OBJ_FILE)
	make RGFW$(OBJ_FILE)
	$(AR) rcs libRGFW.a RGFW$(OBJ_FILE)

xdg-shell.c:
	make initwayland

initwayland:
ifeq ($(RGFW_WAYLAND),1)
	wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell.h
	wayland-scanner public-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell.c
	wayland-scanner client-header /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml xdg-decoration-unstable-v1.h
	wayland-scanner public-code /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml xdg-decoration-unstable-v1.c	
	wayland-scanner client-header /usr/share/wayland-protocols/unstable/relative-pointer/relative-pointer-unstable-v1.xml relative-pointer-unstable-v1-client-protocol.h 
	wayland-scanner client-header /usr/share/wayland-protocols/unstable/relative-pointer/relative-pointer-unstable-v1.xml relative-pointer-unstable-v1-client-protocol.c
else
		
endif

clean:
	rm -f *.o *.obj *.dll .dylib *.a *.so $(EXAMPLE_OUTPUTS) $(EXAMPLE_OUTPUTS_CUSTOM)  .$(OS_DIR)examples$(OS_DIR)*$(OS_DIR)*.exe .$(OS_DIR)examples$(OS_DIR)*$(OS_DIR)*.js .$(OS_DIR)examples$(OS_DIR)*$(OS_DIR)*.wasm 
	

.PHONY: all examples clean

