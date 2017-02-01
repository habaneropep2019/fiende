# -*- Makefile -*-
#
# Borland C++Builder 4 makefile for libsigc++ (0.8.5)
#

# Let Borland Make handle dependencies for us.
#
.AUTODEPEND


##########################################
#
# Environment (directories and commands)
#
##########################################
SIGC_ROOT=..
SRCDIR=$(SIGC_ROOT)\sigc++
INCLUDES=-I.;$(SIGC_ROOT);$(SIGC_ROOT)\sigc++\config
CXX=$(MAKEDIR)\bcc32.exe
MAKE=$(MAKEDIR)\make.exe


##############################################################
#
# DLL Settings
#
# This flags are changed according to the DLL settings
# Default settings. 
#    Compile a libsigc++ DLL
#    Link with BCB4 C/C++ runtime library
#
# Remarks:
#   The -P option is necessary (Comopile *.cc as C++)
#   To build a VCL compatible DLL add '-tWV' to RTL_FLAGS
#   To build a statically linked DLL remove '-tWR' 
#
##############################################################
RTL_FLAGS=-tWR -tWM
DLL_CXXFLAGS= -w-inl -w-par -q -P -O2 -DLIBSIGC_EXPORTS -tWD
DLL_LFLAGS=$(RTL_FLAGS) -tWD
DLL_BASENAME=sigc
DLL_NAME=$(DLL_BASENAME).dll
DLL_IMPORTLIB=$(DLL_BASENAME).lib
TEST_CXXFLAGS=-P -q -w-inl -w-par $(RTL_FLAGS) -tWC



########################################
#
# Object files for building the DLL
#
#######################################
DLLOBJS=adaptor.obj \
	basic_signal.obj \
	marshal.obj \
	object.obj \
	scope.obj \
	slot.obj \
	thread.obj



#################
#
# DLL targets
#
#################
all: $(DLL_NAME)

$(DLL_NAME): $(DLLOBJS)
	$(CXX) $(DLL_LFLAGS) -e"$@" $(DLLOBJS) $(LIBCXX)
	implib $(DLL_IMPORTLIB) $(DLL_NAME)

.cc.obj:
	$(CXX) -c $(INCLUDES) $(DLL_CXXFLAGS) { $< }


# object files dependencies
adaptor.obj:       $(SRCDIR)\adaptor.cc
basic_signal.obj:  $(SRCDIR)\basic_signal.cc
marshal.obj:	   $(SRCDIR)\marshal.cc
object.obj:	   $(SRCDIR)\object.cc
scope.obj:	   $(SRCDIR)\scope.cc
slot.obj:	   $(SRCDIR)\slot.cc
thread.obj:        $(SRCDIR)\thread.cc




##################
#
# wintest program
#
##################
wintest: wintest.exe

wintest.exe: $(DLL_NAME)
	$(CXX) $(INCLUDES) $(TEST_CXXFLAGS) -e"$@" wintest.cpp $(DLL_IMPORTLIB)



#####################################
#
#  Test programs from SIGC_ROOT\tests
#
#####################################
tests: $(DLL_NAME)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\signals\array_test.cc $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\signals\basic.cc      $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\signals\bind_slot_test.cc    $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\signals\convert_slot_test.cc $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\signals\disconnect.cc        $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\signals\func_slot_test.cc    $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\signals\object_slot_test.cc  $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\signals\rettype_slot_test.cc $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\signals\vbasic.cc    $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\handles\ctor.cc	     $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\handles\handle.cc    $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\handles\list_test.cc $(DLL_IMPORTLIB)
   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\misc\marshal_test.cc $(DLL_IMPORTLIB)
#   $(CXX) $(INCLUDES) $(TEST_CXXFLAGS) ..\tests\misc\thread_test.cc  $(DLL_IMPORTLIB)


##############
#
#  Cleanup
#
##############
clean:
	@if exist *.obj del *.obj
	@if exist *.tds del *.tds
	@if exist *.il* del *.il*
	@if exist *~    del *~

distclean realclean: clean
	@if exist *.lib del *.lib
	@if exist *.dll del *.dll
	@if exist *.exe del *.exe
