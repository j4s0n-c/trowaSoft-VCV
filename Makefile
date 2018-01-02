SHELL:=/bin/bash -O extglob

SLUG = trowaSoft
VERSION = 0.5.5.1

# FLAGS will be passed to both C and C++ compiler
#	-Werror=implicit-function-declaration \
FLAGS = \
	-o0 \
	-w \
	-Isrc \
	-Ilib/oscpack \
	-Ilib/oscpack/ip \
	-Ilib/oscpack/osc \
	-I../../dep 

#FLAGS = \
#	-w \
#	-Isrc \
#	-Isrc/include \
#	-Ilib/oscpack/ip \
#	-Ilib/oscpack/osc \
#	-I../../dep 
#	-I..\..\dep\bin

CFLAGS += 
CXXFLAGS +=

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
include ../../arch.mk

#ifeq ($(ARCH), lin)
#	LDFLAGS += -L../../dep/lib -lGLEW -lglfw
#endif

#ifeq ($(ARCH), mac)
#	LDFLAGS += -L../../dep/lib -lGLEW -lglfw  
#endif

ifeq ($(ARCH), win)	
	LDFLAGS += -L../../dep/lib -lglew32 -lglfw3dll 
endif

# Add .cpp and .c files to the build
SOURCES = \
		$(wildcard lib/oscpack/ip/*.cpp) \
		$(wildcard lib/oscpack/osc/*.cpp) \
		$(wildcard src/*.cpp) \
		$(wildcard src/*/*.cpp) \
		  
ifeq ($(ARCH), win)
	SOURCES += $(wildcard lib/oscpack/ip/win32/*.cpp) 
	LDFLAGS += -lws2_32 -lwinmm
else
	SOURCES += $(wildcard lib/oscpack/ip/posix/*.cpp) 
endif

# Must include the VCV plugin Makefile framework
include ../../plugin.mk

# Convenience target for including files in the distributable release
DIST_NAME = trowaSoft
.PHONY: dist
dist: all
	mkdir -p dist/$(DIST_NAME)
	cp LICENSE* dist/$(DIST_NAME)/
	cp plugin.* dist/$(DIST_NAME)/
	cp -R res dist/$(DIST_NAME)/
	mkdir -p dist/$(DIST_NAME)/pd
	cp pd/*.pd dist/$(DIST_NAME)/pd/
	cd dist && zip -5 -r $(DIST_NAME)-$(VERSION)-$(ARCH).zip $(DIST_NAME)
