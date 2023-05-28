#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# Wildcard use to get recursively all the .cpp
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# define the Cpp compiler to use
CXX = g++

DebugActive ?= $(DEBUG)

# define any compile-time flags -mwindows to make the app launch without a command prompt

CXXFLAGS	:= -std=c++17 -Wall -Wextra -g -pthread

ifeq ($(DebugActive),True)
CXXFLAGS    += -DDEBUG # --coverage
else
CXXFLAGS	+= -O2 -DNDEBUG # -mwindows
endif

TESTFLAGS := $(CXXFLAGS)

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS =

# Add a flag to know if we are in production or not
# (to remove some checks that should always be true, eg: static_assert and such)
ifeq ($(ProductionActive), True)
CXXFLAGS    += -DPROD
else
CXXFLAGS    += -DNPROD
endif

# define output directory
ifeq ($(DebugActive),True)
OUTPUT	:= debug_build
else
OUTPUT	:= release_build
endif

# define source directory
SRC		:= src

# define include directory
INCLUDE	:= include

# define import directory
IMPORT 	:= import

# define lib directory
LIB		:= lib \
		#    $(Qt_PATH)/lib

# define dependency directory
DEPENDENCIES := dependencies

# TODO merge the shader directory inside of the ressource directory
# define shader directory
SHADER := shader

# define ressource directory
RESSOURCES := res

# intermediate build directory
BUILD := build

# define test directory
TEST := test

ifeq ($(OS),Windows_NT)
MAIN		 	 := main.exe
SOURCEDIRS		 := $(SRC)
INCLUDEDIRS		 := $(INCLUDE)
IMPORTDIRS 		 := $(IMPORT)
LIBDIRS			 := $(LIB)
DEPENDENCIESDIRS := $(DEPENDENCIES)
SHADERDIR 		 := $(SHADER)
RESSOURCESDIR 	 := $(RESSOURCES)
BUILDDIR 	  	 := $(BUILD)
TESTDIR 	  	 := $(TEST)
FIXPATH 		  = $(subst /,\,$1)
RM				 := powershell rm -r -fo
MD				 := powershell mkdir
else
MAIN			 := main
SOURCEDIRS		 := $(shell find $(SRC) -type d)
INCLUDEDIRS		 := $(shell find $(INCLUDE) -type d)
IMPORTDIRS 		 := $(shell find $(IMPORT) -type d)
LIBDIRS			 := $(shell find $(LIB) -type d)
DEPENDENCIESDIRS := $(shell find $(DEPENDENCIES) -type d)
SHADERDIR 		 := $(shell find $(SHADER) -type d)
RESSOURCESDIR 	 := $(shell find $(RESSOURCES) -type d)
BUILDDIR 	  	 := $(shell find $(BUILD) -type d)
TESTDIR 	  	 := $(shell find $(TEST) -type d)
FIXPATH			  = $1
RM 				 := rm -f
MD				 := mkdir -p
endif

# Look, up to 4 nested directories, to create an include tree of the include files in the source directory
SOURCESDIRTREE := ${sort ${dir ${wildcard ${SOURCEDIRS}/*/ ${SOURCEDIRS}/*/*/ ${SOURCEDIRS}/*/*/*/ ${SOURCEDIRS}/*/*/*/*/}}}

# define Taskflow directory
TASKFLOWDIR := $(IMPORTDIRS)/taskflow/taskflow
TASKFLOWALG := $(TASKFLOWDIR)/algorithm
TASKFLOWCOR := $(TASKFLOWDIR)/core

SDLDIR := $(IMPORTDIRS)/SDL2-2.0.10
SDLLIBDIR := $(SDLDIR)/lib/x64
SDLINCDIR := $(SDLDIR)/include

SDLTTFDIR := $(IMPORTDIRS)/SDL2_ttf-2.0.15
SDLTTFLIBDIR := $(SDLTTFDIR)/lib/x64
SDLTTFINCDIR := $(SDLTTFDIR)/include

SDLMIXERDIR := $(IMPORTDIRS)/SDL2_mixer-2.0.2
SDLMIXERLIBDIR := $(SDLMIXERDIR)/lib/x64
SDLMIXERINCDIR := $(SDLMIXERDIR)/include

GLEWDIR := $(IMPORTDIRS)/glew-2.1.0
GLEWLIBDIR := $(GLEWDIR)/lib/Release/x64
GLEWINCDIR := $(GLEWDIR)/include

GLMDIR := $(IMPORTDIRS)/glm
GLMINCDIR := $(GLMDIR)/glm

ifeq ($(OS),Windows_NT)
# define any directories containing header files other than /usr/include
INCLUDES	 := $(patsubst %,-I%, $(INCLUDEDIRS:%/=%)) \
			    $(patsubst %,-I%, $(SOURCESDIRTREE:%/=%)) \
				$(patsubst %,-I%, $(TASKFLOWDIR:%/=%)) \
				$(patsubst %,-I%, $(TASKFLOWALG:%/=%)) \
				$(patsubst %,-I%, $(TASKFLOWCOR:%/=%)) \
				$(patsubst %,-I%, $(SDLINCDIR:%/=%)) \
				$(patsubst %,-I%, $(SDLTTFINCDIR:%/=%)) \
				$(patsubst %,-I%, $(SDLMIXERINCDIR:%/=%)) \
				$(patsubst %,-I%, $(GLEWINCDIR:%/=%)) \
				$(patsubst %,-I%, $(GLMINCDIR:%/=%))

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%)) \
			   $(patsubst %,-L%, $(SDLLIBDIR:%/=%)) \
			   $(patsubst %,-L%, $(SDLTTFLIBDIR:%/=%)) \
			   $(patsubst %,-L%, $(SDLMIXERLIBDIR:%/=%)) \
			   $(patsubst %,-L%, $(GLEWLIBDIR:%/=%)) \
			   -lSDL2main \
			   -lSDL2 \
			   -lSDL2_ttf \
			   -lSDL2_mixer \
			   -lglew32 \
			   -lzlib1 \
			   -lopengl32
else
# define any directories containing header files other than /usr/include
INCLUDES	 := $(patsubst %,-I%, $(INCLUDEDIRS:%/=%)) \
			    $(patsubst %,-I%, $(SOURCESDIRTREE:%/=%)) \
				$(patsubst %,-I%, $(TASKFLOWDIR:%/=%)) \
				$(patsubst %,-I%, $(TASKFLOWALG:%/=%)) \
				$(patsubst %,-I%, $(TASKFLOWCOR:%/=%)) \
				$(patsubst %,-I%, $(GLMINCDIR:%/=%))

# define the C libs
LIBS		:= -lSDL2main \
			   -lSDL2 \
			   -lSDL2_ttf \
			   -lSDL2_mixer \
			   -lGLEW \
			   -lGLU \
			   -lGL

endif
# define GTest directory
GTESTDIR := $(IMPORTDIRS)/googletest/googletest

# All Google Test headers.  Usually you shouldn't change this definition.
GTEST_HEADERS = $(GTESTDIR)/include/gtest/*.h \
                $(GTESTDIR)/include/gtest/internal/*.h

TESTFLAGS += -isystem $(GTESTDIR)/include

TEST_INCLUDES := $(patsubst %,-I%, $(TESTDIR:%/=%))

# define the C source files
SOURCES		 := $(call rwildcard,$(SOURCEDIRS), *.cpp)

GTEST_SOURCES = $(GTESTDIR)/src/*.cc $(GTESTDIR)/src/*.h $(GTEST_HEADERS)

TEST_SOURCES := $(call rwildcard,$(TESTDIR), *.cc) 

# define the C object files 
OBJECTS		 := $(SOURCES:%.cpp=$(BUILDDIR)/%.o) \

TEST_OBJECTS := $(SOURCES:%.cpp=$(BUILDDIR)/%.o) \
				$(TEST_SOURCES:%.cc=$(BUILDDIR)/%.o)

TEST_OBJECTS := $(filter-out build/src/main.o, $(TEST_OBJECTS))

DEP := $(OBJECTS:%.o=%.d)

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

ifeq ($(OS),Windows_NT)

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))
TESTMAIN 	:= $(call FIXPATH,$(OUTPUT)/test.exe)

# Compile the executable
all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

# Compile the test
test: $(TEST_OBJECTS) gtest-all.o
	@echo Building Test ...
	$(CXX) $(TESTFLAGS) $(INCLUDES) $(TEST_INCLUDE) -o $(TESTMAIN) $(TEST_OBJECTS) gtest-all.o $(LFLAGS) $(LIBS)

	ifeq ($(DebugActive),True)
		./debug_build/test.exe
	else
		./release_build/test.exe
	endif

# Create the output hierarchy
$(OUTPUT):
	$(MD) $(OUTPUT)

# Build main and copy dependencies
$(MAIN): $(OBJECTS)
	@echo Building Main ...
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS)

	@echo Copy the ddl dependencies
	xcopy $(DEPENDENCIESDIRS)\x64 $(OUTPUT) /v /f /s /y /d

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)

# Compile the test files
$(BUILDDIR)/%.o: %.cc
	@echo Converting $< to $@
	$(CXX) $(TESTFLAGS) $(TEST_INCLUDE) $(INCLUDES) -MMD  -MP -c $< -o $@

gtest-all.o : $(GTEST_SOURCES)
	$(CXX) $(TESTFLAGS) -I$(GTESTDIR) $(TESTFLAGS) -c \
            $(GTESTDIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SOURCES)
	$(CXX) $(TESTFLAGS) -I$(GTESTDIR) $(TESTFLAGS) -c \
            $(GTESTDIR)/src/gtest_main.cc

# Compile all the remaining source files
$(BUILDDIR)/%.o: %.cpp
	@echo Converting $< to $@
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD  -MP -c $< -o $@

$(OBJECTS): | $(BUILDDIR)

$(BUILDDIR):
	$(MD) $(foreach dir, $(SOURCESDIRTREE), $(BUILDDIR)/$(dir),) build/test

else

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))
TESTMAIN 	:= $(call FIXPATH,$(OUTPUT)/test)

# Compile the executable
all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

# Compile the test
test: $(TEST_OBJECTS) gtest-all.o
	@echo Building Test ...
	$(CXX) $(TESTFLAGS) $(INCLUDES) $(TEST_INCLUDE) -o $(TESTMAIN) $(TEST_OBJECTS) gtest-all.o $(LFLAGS) $(LIBS)

	ifeq ($(DebugActive),True)
		./debug_build/test
	else
		./release_build/test
	endif

# Create the output hierarchy
$(OUTPUT):
	$(MD) $(OUTPUT)

# Build main and copy dependencies
$(MAIN): $(OBJECTS)
	@echo Building Main ...
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS)

	@echo Copy the ddl dependencies
	cp $(DEPENDENCIESDIRS) $(OUTPUT) /v /f /s /y /d

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)

# Compile the test files
$(BUILDDIR)/%.o: %.cc
	@echo Converting $< to $@
	$(CXX) $(TESTFLAGS) $(TEST_INCLUDE) $(INCLUDES) -MMD  -MP -c $< -o $@

gtest-all.o : $(GTEST_SOURCES)
	$(CXX) $(TESTFLAGS) -I$(GTESTDIR) $(TESTFLAGS) -c \
            $(GTESTDIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SOURCES)
	$(CXX) $(TESTFLAGS) -I$(GTESTDIR) $(TESTFLAGS) -c \
            $(GTESTDIR)/src/gtest_main.cc

# Compile all the remaining source files
$(BUILDDIR)/%.o: %.cpp
	@echo Converting $< to $@
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD  -MP -c $< -o $@

$(OBJECTS): | $(BUILDDIR)

$(BUILDDIR):
	$(MD) $(foreach dir, $(SOURCESDIRTREE), $(BUILDDIR)/$(dir),) build/test

endif

.PHONY: clean

clean:
	@echo Cleaning...
	$(RM) $(BUILDDIR)
	$(RM) gtest-all.o
#	$(RM) $(call FIXPATH, $(call rwildcard,$(SOURCEDIRS),*.o))
#	$(RM) $(call FIXPATH, $(call rwildcard,$(SOURCEDIRS),*.d))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!

-include $(DEP)