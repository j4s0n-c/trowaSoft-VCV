SHELL:=/bin/bash -O extglob

#1.3 Remove SLUG and VERSION from the Makefile, and remove p->slug = ... and p->version = ... from your plugin’s main .cpp file, since they are now defined in plugin.json.
RACK_DIR ?= ../..

#FLAGS += -w

# Add .cpp and .c files to the build
SOURCES = \
		$(wildcard lib/oscpack/ip/*.cpp) \
		$(wildcard lib/oscpack/osc/*.cpp) \
		$(wildcard src/*.cpp) \
		$(wildcard src/*/*.cpp) \

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
include $(RACK_DIR)/arch.mk

ifeq ($(ARCH), win)
	SOURCES += $(wildcard lib/oscpack/ip/win32/*.cpp) 
	LDFLAGS += -lws2_32 -lwinmm
	LDFLAGS +=  -L$(RACK_DIR)/dep/lib #-lglew32 -lglfw3dll
	#LDFLAGS += -lrtmidi
else
	SOURCES += $(wildcard lib/oscpack/ip/posix/*.cpp) 
endif

DISTRIBUTABLES += $(wildcard LICENSE*) res \
 pd other
# ^ add our other folders (supplementary files)

include $(RACK_DIR)/plugin.mk
