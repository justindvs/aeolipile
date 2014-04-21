-include config.mak

# Directories of interest
SRC_DIR := src
MONGOOSE_SRC_DIR := $(SRC_DIR)/mongoose
KEYPRESS_SRC_DIR := $(SRC_DIR)/keypress
WSCMD_DIR := $(SRC_DIR)/wscmd
WEB_DIR := www

# Use SPEED=1 on the cmd line to enable compiler optimizations
ifneq (,$(SPEED))
   ADDL_CC_OPTS := /O2
else
   ADDL_CC_OPTS := /Zi
endif

CC_OPTS := /I "$(VC_DIR)/include" \
           /I "$(WINDOWS_SDK_DIR)/Include/um" \
           /I "$(WINDOWS_SDK_DIR)/Include/shared" \
           /I "$(MONGOOSE_SRC_DIR)" \
           /I "$(KEYPRESS_SRC_DIR)" \
           /I "$(WSCMD_DIR)" \
           /I "$(SRC_DIR)" \
           $(ADDL_CC_OPTS)

CC = "$(VCTOOLS_DIR)/cl" $(CC_OPTS)

LINK_OPTS := /link "$(WINDOWS_SDK_LIB_DIR)/WS2_32.Lib" \
                   "$(WINDOWS_SDK_LIB_DIR)/Uuid.Lib" \
                   "$(WINDOWS_SDK_LIB_DIR)/kernel32.Lib" \
                   "$(WINDOWS_SDK_LIB_DIR)/User32.Lib" \
                   "$(VC_LIB_DIR)/libcmt.lib" \
                   "$(VC_LIB_DIR)/oldnames.lib"

all: config.mak pyroscaphe.exe $(WEB_DIR)/pyroscaphe.js

pyroscaphe.exe: mongoose.obj $(SRC_DIR)/pyroscaphe.cpp $(KEYPRESS_SRC_DIR)/keypress.cpp $(WSCMD_DIR)/wscmd.cpp
	$(CC) /EHs /W4 $(filter %.cpp,$^) mongoose.obj /Fe$@ $(LINK_OPTS)

# Build mongoose separately.  I'm doing this because mongoose isn't
# /W4 clean and I want to use /W4 for my other code.
mongoose.obj: $(wildcard $(MONGOOSE_SRC_DIR)/*)
	$(CC) -c $(filter %.c,$^)

# pyroscaphe.js is just jQuery + jQuery.mobile + my own utils.js.  I
# package it up into one .js file to make everything easier to pull in
# to an HTML page.
$(WEB_DIR)/pyroscaphe.js: $(addprefix $(WEB_DIR)/, utils.js jquery-1.11.0.js jquery.mobile-1.4.2.js)
	@echo Generating $(notdir $@)
	@echo // Generated file.  Do not modify! > $@
	@echo // This file is just the concatenation of these files: $(notdir $^) >> $@
	@echo // The intention of this is to make it easier to pull this stuff in in web pages >> $@
	@cat $^ >> $@

config.mak:
	@echo %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	@echo Generating default $@: Please ensure the paths in this
	@echo   file are correct for your machine config and then
	@echo   run "make" again.
	@echo %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	@echo \# Generated file: Please ensure these paths are correct for your machine > $@
	@echo VC_DIR := 'C:/Program Files (x86)/Microsoft Visual Studio 11.0/VC' >> $@
	@echo VCTOOLS_DIR := '$$(VC_DIR)/BIN/x86_amd64' >> $@
	@echo WINDOWS_SDK_DIR := 'C:/Program Files (x86)/Windows Kits/8.0' >> $@
	@echo WINDOWS_SDK_LIB_DIR := '$$(WINDOWS_SDK_DIR)/Lib/win8/um/x64' >> $@
	@echo VC_LIB_DIR := '$$(VC_DIR)/lib/amd64' >> $@
	@exit 1

.PHONY: clean startserver

clean:
	rm -f $(wildcard *.obj) $(wildcard *.lib) $(wildcard *.dll) $(wildcard *.exe) $(wildcard *.exp) $(wildcard *.pdb) $(wildcard *.ilk) $(WEB_DIR)/pyroscaphe.js

startserver:
	cmd /c start pyroscaphe.exe
