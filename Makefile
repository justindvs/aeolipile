SRC_DIR := src
CC = cl $(CC_OPTS)

WINDOWS_SDK_DIR := C:/Program Files (x86)/Windows Kits/8.0
VC_DIR := C:/Program Files (x86)/Microsoft Visual Studio 11.0/VC
MONGOOSE_SRC_DIR := $(SRC_DIR)/mongoose
KEYPRESS_SRC_DIR := $(SRC_DIR)/keyPressLib
WEBSOCKCMD_DIR := $(SRC_DIR)/webSockCmd

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
           /I "$(WEBSOCKCMD_DIR)" \
           /I "$(SRC_DIR)" \
           $(ADDL_CC_OPTS)

WINDOWS_SDK_LIB_DIR := $(WINDOWS_SDK_DIR)/Lib/win8/um/x64
VC_LIB_DIR := $(VC_DIR)/lib/amd64

LINK_OPTS := /link "$(WINDOWS_SDK_LIB_DIR)/WS2_32.Lib" \
                   "$(WINDOWS_SDK_LIB_DIR)/Uuid.Lib" \
                   "$(WINDOWS_SDK_LIB_DIR)/kernel32.Lib" \
                   "$(WINDOWS_SDK_LIB_DIR)/User32.Lib" \
                   "$(VC_LIB_DIR)/libcmt.lib" \
                   "$(VC_LIB_DIR)/oldnames.lib"

all: aeolipile.exe

aeolipile.exe: mongoose.obj $(SRC_DIR)/aeolipile.cpp $(KEYPRESS_SRC_DIR)/keyPress.cpp $(WEBSOCKCMD_DIR)/webSockCmd.cpp
	$(CC) /EHs /W4 $(SRC_DIR)/aeolipile.cpp $(KEYPRESS_SRC_DIR)/keyPress.cpp $(WEBSOCKCMD_DIR)/webSockCmd.cpp mongoose.obj /Fe$@ $(LINK_OPTS)

mongoose.obj: $(wildcard $(MONGOOSE_SRC_DIR)/*)
	$(CC) -c $(filter %.c,$^)

.PHONY: clean startserver

clean:
	rm -f $(wildcard *.obj) $(wildcard *.lib) $(wildcard *.dll) $(wildcard *.exe) $(wildcard *.exp) $(wildcard *.pdb) $(wildcard *.ilk)

startserver:
	cmd /c start aeolipile.exe
