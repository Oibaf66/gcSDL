##################################################
# Makefile to build the SDL library for GameCube #
#  by infact <infact@quantentunnel.de>  03/2012  #
##################################################

# clear the implicit built in rules
.SUFFIXES:

# we need a devkitPro/devkitPPC environment
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

# load the gamecube specific rules
include $(DEVKITPPC)/gamecube_rules

# TARGET is the name of the final library
TARGET		:= libSDL.a

# directories
#  BUILD is the directory where object files & intermediate files will be placed
#  SOURCES is a list of directories containing source code
#  INCLUDES is a list of directories containing extra header files
BUILD		:= build
SOURCES		:= \
			src src/audio src/cdrom src/cpuinfo src/events \
			src/file src/joystick src/stdlib src/thread \
			src/timer src/video src/cdrom/dummy src/loadso/dummy \
			src/audio/cube src/video/cube src/joystick/cube \
			src/thread/cube src/timer/cube src/main/cube
INCLUDES	:= include

# options for code generation
CFLAGS		= -O2 -Wall $(MACHDEP) -DHW_DOL $(INCLUDE)

# for debugging
ifeq ($(strip $(CUBESDL_DEBUG)),1)
CFLAGS		= -g -Wall $(MACHDEP) -DHW_DOL -DCUBESDL_DEBUG=1 $(INCLUDE)
endif

# determine if first or second stage
ifneq ($(BUILD),$(notdir $(CURDIR)))
#  first stage

export OUTPUT	:= $(CURDIR)/$(TARGET)
export VPATH	:= $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR	:= $(CURDIR)/$(BUILD)

# automatically build a list of object files for our project
CFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export OFILES	:= $(CFILES:.c=.o)

# build a list of include paths
export INCLUDE	:= \
			$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) -I$(LIBOGC_INC)

export OUTPUT	:= $(CURDIR)/$(TARGET)

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@echo all done.

clean:
	@echo -n cleaning...
	@rm -fr $(BUILD) $(OUTPUT)
	@echo done

else
#  second stage

DEPENDS		:= $(OFILES:.o=.d)

# main target
$(OUTPUT): $(OFILES)
	@echo archiving library: $(notdir $@)
	@rm -f $@
	@$(AR) -rcs $@ $^

-include $(DEPENDS)

endif
# end of stages
