# To debug or not to debug.  Set to 0 for Release mode.
DEBUG=0

# To trace or not to trace.  Set to 1 for tracing.
TRACE=0

# Version information.
MAJOR_VERSION=1
FULL_VERSION=1.5.0

# Figure out what kind of machine this is.
OS := $(shell uname)
MACHINE := $(shell uname -m)
ARCH := $(OS)-$(MACHINE)

# Set up dummy versions of OS-specific variables so that if no OS is found
# then it's an obvious error.
EXE_EXT=ERROR_BAD_EXE_EXT
DLL_EXT=ERROR_BAD_DLL_EXT
DLL_NAME_ARG_NAME=ERROR_BAD_DLL_NAME_ARG_NAME
DLL_NAME_EXT=ERROR_BAD_DLL_NAME_EXT
RM=ERROR_BAD_RM


# OS-specific variables for Windows.
ifeq ($(OS), Windows)
EXE_EXT=.exe
DLL_EXT=.dll
DLL_NAME_ARG_NAME=
DLL_NAME_EXT=
USE_GCC=0
RM=del
EXTRA_RELEASE_LINKER_ARGS=
endif

# OS-specific variables for Linux.
ifeq ($(OS), Linux)
EXE_EXT=
DLL_EXT=.so.$(FULL_VERSION)
DLL_NAME_ARG_NAME=-soname
DLL_NAME_EXT=.so.$(MAJOR_VERSION)
USE_GCC=1
# rt for clock_gettime, used by libcurl.
EXTRA_LIBS=-lrt -lm 
EXTRA_DEFINES=-DNP1_NEED_FSTAT_AS_SYSCALL
RM=rm -f
EXTRA_RELEASE_LINKER_ARGS=
endif

# OS-specific variables for Mac OS X.
ifeq ($(OS), Darwin)
EXE_EXT=
DLL_EXT=.$(FULL_VERSION).$dynlink
DLL_NAME_ARG_NAME=-dynlib_install_name
DLL_NAME_EXT=.$(MAJOR_VERSION).dynlink
USE_GCC=1
EXTRA_LIBS=-lm 
EXTRA_DEFINES=
RM=rm -f
EXTRA_RELEASE_LINKER_ARGS=-Xlinker -unexported_symbol -Xlinker "*"
endif

# OS-specific variables for FreeBSD.
ifeq ($(OS), FreeBSD)
EXE_EXT=
DLL_EXT=.so.$(FULL_VERSION)
DLL_NAME_ARG_NAME=-soname
DLL_NAME_EXT=.so.$(MAJOR_VERSION)
USE_GCC=1
EXTRA_LIBS=-lexecinfo -lm 
EXTRA_DEFINES=
RM=rm -f
EXTRA_RELEASE_LINKER_ARGS=
endif

# Misc files & directories.
ROOT_SRC_DIR=../../..
NPLUS1_SRC_DIR=../..
LIB_SRC_DIR=$(ROOT_SRC_DIR)/lib/liberal
ZLIB_SRC_DIR=$(LIB_SRC_DIR)/zlib/src/zlib-1.2.5
ZLIB_BIN_DIR=$(LIB_SRC_DIR)/zlib/bin
ZLIB_ARCHIVE=$(ZLIB_BIN_DIR)/libz-$(ARCH).a
SNAPPY_SRC_DIR=$(LIB_SRC_DIR)/snappy/src/snappy-1.0.2
SNAPPY_BIN_DIR=$(LIB_SRC_DIR)/snappy/bin
SNAPPY_ARCHIVE=$(SNAPPY_BIN_DIR)/libsnappy-$(ARCH).a
LIBCURL_SRC_DIR=$(LIB_SRC_DIR)/curl/src/curl-7.21.4
LIBCURL_BIN_DIR=$(LIB_SRC_DIR)/curl/bin
LIBCURL_ARCHIVE=$(LIBCURL_BIN_DIR)/libcurl-$(ARCH).a
LIBPCRE_SRC_DIR=$(LIB_SRC_DIR)/pcre/src/pcre-8.12
LIBPCRE_BIN_DIR=$(LIB_SRC_DIR)/pcre/bin
LIBPCRE_ARCHIVE=$(LIBPCRE_BIN_DIR)/libpcre-$(ARCH).a

# File names.
R17_TEST_FILE_NAME=r17_test$(EXE_EXT)
R17_EXE_FILE_NAME=r17$(EXE_EXT)


# Basic compiler and linker arguments.  We use the VC++ compiler on Windows, 
# GCC on everything else.
# NOTE that if we ever support multiple threads within the same process then
# we'll need to remove -fno-threadsafe-statics and deal with the resulting
# undefined reference problems.

GCC_DEBUG_RELEASE_BASE=g++ -Wall -Werror -Wfatal-errors -fno-strict-aliasing -Wstrict-aliasing -std=c++0x -nostdinc++ -fno-rtti -fno-exceptions -fno-threadsafe-statics -I . -I $(ROOT_SRC_DIR) -I $(NPLUS1_SRC_DIR)/include -I $(NPLUS1_SRC_DIR)/include/std -I $(ZLIB_SRC_DIR) -I $(SNAPPY_SRC_DIR) -I $(LIBCURL_SRC_DIR)/include -I $(LIBPCRE_SRC_DIR) -DNP1_TRACE_ON=$(TRACE) -DNP1_VERSION_STRING=\"$(FULL_VERSION)\" $(EXTRA_DEFINES)
GCC_DEBUG_RELEASE_LINK_BASE=g++ -nodefaultlibs -lc -lgcc -L$(ZLIB_BIN_DIR) -L$(SNAPPY_BIN_DIR) -L$(LIBCURL_BIN_DIR) -L$(LIBPCRE_BIN_DIR)
MSVC_DEBUG_RELEASE_BASE=cl.exe -X /I"." -I"std" /Zi /nologo /W3 /WX- /Od /Oy- /D "_CRT_SECURE_NO_WARNINGS" /D "WIN32" /D "NP1_TRACE_ON=$(TRACE)" /Gm- /GS /fp:precise /Zc:forScope /Fd"vc100.pdb" /Gd /TP /analyze-
MSVC_DEBUG_RELEASE_LINK_BASE=link.exe /NOLOGO "kernel32.lib" "Ws2_32.lib" /MANIFEST:NO /TLBID:1 /DYNAMICBASE /NXCOMPAT


ifeq ($(DEBUG), 1)
GCC_EXE_BASE=$(GCC_DEBUG_RELEASE_BASE) -g3 -ggdb -D NP1_DEBUG
GCC_DLL_BASE=$(GCC_EXE_BASE) -fpic
GCC_EXE_LINK_BASE=$(GCC_DEBUG_RELEASE_LINK_BASE)
GCC_DLL_LINK_BASE=$(GCC_EXE_LINK_BASE)
MSVC_EXE_BASE=$(MSVC_DEBUG_RELEASE_BASE) /Od /D "_DEBUG" /RTC1 /MTd /D "NP1_DEBUG"
MSVC_DLL_BASE=$(MSVC_EXE_BASE)
MSVC_LINK_BASE=$(MSVC_DEBUG_RELEASE_LINK_BASE) /DEBUG
MSVC_EXE_LINK_BASE=$(MSVC_LINK_BASE) /SUBSYSTEM:CONSOLE
MSVC_DLL_LINK_BASE=$(MSVC_LINK_BASE) /DLL
else
GCC_EXE_BASE=$(GCC_DEBUG_RELEASE_BASE) -O3 -s $(EXTRA_RELEASE_LINKER_ARGS)
GCC_DLL_BASE=$(GCC_EXE_BASE) -fpic
GCC_EXE_LINK_BASE=$(GCC_DEBUG_RELEASE_LINK_BASE) -s $(EXTRA_RELEASE_LINKER_ARGS)
GCC_DLL_LINK_BASE=$(GCC_EXE_LINK_BASE)
MSVC_EXE_BASE=$(MSVC_DEBUG_RELEASE_BASE) /O3 /Oi /GL /D "NDEBUG" /MT /Gy
MSVC_DLL_BASE=$(MSVC_EXE_BASE)
MSVC_LINK_BASE=$(MSVC_DEBUG_RELEASE_LINK_BASE) /DEBUG
MSVC_EXE_LINK_BASE=$(MSVC_LINK_BASE) /SUBSYSTEM:CONSOLE
MSVC_DLL_LINK_BASE=$(MSVC_LINK_BASE) /DLL
endif

# Functions to generate the full compile/link command line.  Use the 'call'
# Make builtin to call,
# eg $(call COMPILE_FOR_EXE,file.c,file.o)
ifeq ($(USE_GCC), 1)
COMPILE_FOR_EXE = $(GCC_EXE_BASE) -c $(1) -o $(2)
COMPILE_FOR_DLL = $(GCC_DLL_BASE) -c $(1) -o $(2)
EXE_LINK = $(GCC_EXE_LINK_BASE) -o $(1) 
# args = dll_name dll_file_name
# This is currently broken for Mac OS X.  Looks like we'd need to use gcc -dynamiclib -current_version 1.0 instead of the -shared -Wl stuff.
DLL_LINK = $(GCC_DLL_LINK_BASE) -shared -Wl,$(DLL_NAME_ARG_NAME),$(1) -o $(2) -lc  
else
COMPILE_FOR_DLL = $(MSVC_DLL_BASE) /Fo$(2) /c $(1)
COMPILE_FOR_EXE = $(MSVC_EXE_BASE) /Fo$(2) /c $(1)
# args = dll_name dll_file_name
EXE_LINK = $(MSVC_EXE_LINK_BASE) /OUT:$(1) /PDB:$(1).pdb 
DLL_LINK = $(MSVC_DLL_LINK_BASE) /OUT:$(2) /PDB:$(2).pdb /IMPLIB:$(1).lib 
endif



# Non-header source files.
OBJECT_FILES = \
  $(LIB_SRC_DIR)/int64/int64.o
  

# GCC (at least) requires different compilation for DLLs vs EXEs.
DLL_OBJECT_FILES = $(OBJECT_FILES:.o=.odll)

# TODO: testing
TEST_OBJECT_FILES = \
  r17_test.o 


# GCC (at least) requires different compilation for DLLs vs EXEs.
DLL_TEST_OBJECT_FILES = $(TEST_OBJECT_FILES:.o=.odll)


# Windows' del command spits the dummy on file names with forward slashes.
GENERATED_FILES_BASE = \
  $(R17_TEST_FILE_NAME) *.pdb *.ilk $(OBJECT_FILES) \
  $(TEST_OBJECT_FILES) $(DLL_OBJECT_FILES) $(DLL_TEST_OBJECT_FILES) r17.o \
  $(R17_EXE_FILE_NAME)

ifeq ($(OS), Windows)
GENERATED_FILES=$(subst /,\,$(GENERATED_FILES_BASE))
else
GENERATED_FILES=$(GENERATED_FILES_BASE)
endif



# Compile the r17 executable.
all: $(R17_EXE_FILE_NAME)

# Clean up intermediate files.
clean:
	$(RM) $(GENERATED_FILES)


# Unit testing	
test: $(OBJECT_FILES) $(TEST_OBJECT_FILES)
	$(call EXE_LINK,$(R17_TEST_FILE_NAME)) $(OBJECT_FILES) $(TEST_OBJECT_FILES) $(ZLIB_ARCHIVE) $(SNAPPY_ARCHIVE) $(LIBCURL_ARCHIVE) $(LIBPCRE_ARCHIVE) $(EXTRA_LIBS)


# The main executable.
$(R17_EXE_FILE_NAME): $(OBJECT_FILES) r17.o 
	$(call EXE_LINK,$(R17_EXE_FILE_NAME)) $(OBJECT_FILES) r17.o $(ZLIB_ARCHIVE) $(SNAPPY_ARCHIVE) $(LIBCURL_ARCHIVE) $(LIBPCRE_ARCHIVE) $(EXTRA_LIBS)


# Compile and then run the unit tests.
check: test
	rm -rf /tmp/np1_test_reliable_storage_local_root/ /var/www/np1_test_reliable_storage_remote_root/*
	./$(R17_TEST_FILE_NAME) 

# How to compile C++ files.
.cpp.o:
	$(call COMPILE_FOR_EXE,$<,$@)


%.odll : %.c
	$(call COMPILE_FOR_DLL,$<,$@)

