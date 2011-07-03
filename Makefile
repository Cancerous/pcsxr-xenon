#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITXENON)),)
$(error "Please set DEVKITXENON in your environment. export DEVKITXENON=<path to>devkitPPC")
endif

include $(DEVKITXENON)/rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=  $(notdir $(CURDIR))
BUILD		:=  build
SOURCES		:=  source/libpcsxcore source/plugins/xenon_gfx source/plugins/xenon_cdrom source/plugins/xenon_input source/main source/plugins/xenon_audio source/libpcsxcore/ppc
DATA		:=  data
INCLUDES	:=  include source/libpcsxcore/ppc source/libpcsxcore source source/main source/plugins/xenon_cdrom

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ASFLAGS	= -Wa,$(INCLUDE) -Wa,-a32
CFLAGS	= -ffunction-sections -fdata-sections -g -O3 -Wall $(MACHDEP) $(INCLUDE) -DLOG_STDOUT -DLIBXENON -DPSXREC -D__BIG_ENDIAN__ -D__ppc__ -D__powerpc__ -D__POWERPC__ -DELF -D__BIGENDIAN__ -D__PPC__ -D__BIGENDIAN__
CXXFLAGS	=	$(CFLAGS)

LDFLAGS	=	-g $(MACHDEP) -Wl,--gc-sections -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	-lxenon -lm  -lpng -lz

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= libs

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
PSUFILES        :=      $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.psu)))
VSUFILES        :=      $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.vsu)))
BINFILES        :=      $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.bin)))
PNGFILES        :=      $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.png)))
ABCFILES        :=      $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.abc)))
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBXENON_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBXENON_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).elf32

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).elf32: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)
%.vsu.o : %.vsu
	@echo $(notdir $<)
	@$(bin2o)
%.psu.o : %.psu
	@echo $(notdir $<)
	@$(bin2o)
%.bin.o : %.bin
	@echo $(notdir $<)
	@$(bin2o)
%.png.o : %.png
	@echo $(notdir $<)
	@$(bin2o)
%.abc.o : %.abc
	@echo $(notdir $<)
	@$(bin2o)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------


run: $(BUILD) $(OUTPUT).elf32
	cp $(OUTPUT).elf32 /var/lib/tftpboot/tftpboot/xenon
	$(PREFIX)strip /var/lib/tftpboot/tftpboot/xenon
