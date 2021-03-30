#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# Wildcard use to get recursively all the .cpp
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Qt path
Qt_PATH := Z:/Qt/5.11.2-x64

# define the Cpp compiler to use
CXX = g++

# define the moc
MOC = $(Qt_PATH)/bin/moc.exe

# define any compile-time flags
CXXFLAGS	:= -std=c++11 -Wall -Wextra -g

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS =

# define output directory
OUTPUT	:= output

# define source directory
SRC		:= src

# define include directory
INCLUDE	:= include \
		   $(Qt_PATH)/include \
		   $(Qt_PATH)/include/QtCore \
		   $(Qt_PATH)/include/QtGui

# define lib directory
LIB		:= lib \
		   $(Qt_PATH)/lib

DEPENDENCIES := dependencies

ifeq ($(OS),Windows_NT)
MAIN		 	 := main.exe
SOURCEDIRS		 := $(SRC)
INCLUDEDIRS		 := $(INCLUDE)
LIBDIRS			 := $(LIB)
DEPENDENCIESDIRS := $(DEPENDENCIES)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /f
MD	:= mkdir
else
MAIN			 := main
SOURCEDIRS		 := $(shell find $(SRC) -type d)
INCLUDEDIRS		 := $(shell find $(INCLUDE) -type d)
LIBDIRS			 := $(shell find $(LIB) -type d)
DEPENDENCIESDIRS := $(shell find $(DEPENDENCIES) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p
endif

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%)) \
			   -lQt5Gui \
			   -lQt5Core

# define the C source files
SOURCES		:= $(call rwildcard,$(SOURCEDIRS), *.cpp)
MOC_SOURCES	:= $(call rwildcard,$(SOURCEDIRS), *.h)

# define the C object files 
OBJECTS		:= $(SOURCES:.cpp=.o) $(MOC_SOURCES:.h=.moc.o) 

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS)
	@echo Building Main ...
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS)

	@echo Copy the ddl dependencies
	xcopy $(DEPENDENCIESDIRS) $(OUTPUT) /v /f /s /y /d

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
%.o: %.cpp
	@echo Converting $< to $@
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

%.moc.cpp: %.h
	@echo Creating $@
	$(MOC) $(INCLUDES) $< -o $@

.PHONY: clean

clean:
	$(RM) $(call FIXPATH, $(call rwildcard,$(SOURCEDIRS),*.o))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!