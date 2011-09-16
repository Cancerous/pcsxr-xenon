#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITXENON)),)
$(error "Please set DEVKITXENON in your environment. export DEVKITXENON=<path to>devkitPPC")
endif

include $(DEVKITXENON)/rules
MACHDEP =  -DXENON -m32 -mno-altivec -fno-pic  -fno-pic -mpowerpc64 -mhard-float -L$(DEVKITXENON)/xenon/lib/32 -u read -u _start -u exc_base 
#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=  $(notdir $(CURDIR))
BUILD		:=  build
#SOURCES		:=  source/shaders lib/zlib source/libpcsxcore_df source/main  source/main/usb source/plugins/xenon_input source/plugins/xenon_audio_repair source/fakegl   source/plugins/cdr   source/plugins/xenon_gfx source/ppc #  source/plugins/gxvideo # source/dynarec 
PLUGINS		:=  source/plugins/dfinput source/shaders source/plugins/xenon_input source/plugins/xenon_audio source/plugins/peopsxgl source/plugins/cdrcimg # source/plugins/xenon_gfx
CORE		:=  lib/zlib source/libpcsxcore source/ppcr	 # source/libpcsxcore/ppc #source/ppcr	
#CORE		:=  lib/zlib source/libpcsxcore_df source/ppc
#LIB		:=  source/fakegl
SOURCES		:=  source/main  source/main/usb $(PLUGINS) $(CORE)
DATA		:=  
INCLUDES	:=  shaders include lib/zlib source/libpcsxcore

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ASFLAGS	= -Wa,$(INCLUDE) -Wa,-a32
CFLAGS	= -ffunction-sections -fdata-sections -g -Ofast -fno-tree-vectorize -fno-tree-slp-vectorize -ftree-vectorizer-verbose=1 -Wall $(MACHDEP) $(INCLUDE) -DLOG_STDOUT -DLIBXENON -D__BIG_ENDIAN__ -D__ppc__ -D__powerpc__ -D__POWERPC__ -DELF -D__BIGENDIAN__ -D__PPC__ -D__BIGENDIAN__ # -DLZX_GUI
#CFLAGS	= -ffunction-sections -fdata-sections -g -Ofast -ftree-vectorizer-verbose=2 -Wall $(MACHDEP) $(INCLUDE) -DLOG_STDOUT -DLIBXENON -D__BIG_ENDIAN__ -D__ppc__ -D__powerpc__ -D__POWERPC__ -DELF -D__BIGENDIAN__ -D__PPC__ -D__BIGENDIAN__
CXXFLAGS	=	$(CFLAGS)

LDFLAGS	=	-g $(MACHDEP) -Wl,--gc-sections -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	-lxenon -lm -lz -lpng -lbz2 

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
					$(sFILES:.s=.o) $(SFILES:.S=.o) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					

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
