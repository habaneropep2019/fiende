# Microsoft Developer Studio Project File - Name="gdkmm_static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=gdkmm_static - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gdkmm_static.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gdkmm_static.mak" CFG="gdkmm_static - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gdkmm_static - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "gdkmm_static - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gdkmm_static - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "." /I "c:\gnu\src" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"gdk--1.2.static.lib"

!ELSEIF  "$(CFG)" == "gdkmm_static - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /Zi /Od /I "c:\gnu\src\\" /I "." /I "c:\gnu\src" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"gdk--1.2.static.debug.lib"

!ENDIF 

# Begin Target

# Name "gdkmm_static - Win32 Release"
# Name "gdkmm_static - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\gdk--\bitmap.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\color.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\colormap.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\cursor.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\drawable.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\font.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\gc.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\image.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\main.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\pixmap.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\visual.cpp"
# End Source File
# Begin Source File

SOURCE=".\gdk--\window.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=".\gdk--\bitmap.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\color.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\colormap.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\cursor.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\draw.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\drawable.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\font.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\gc.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\image.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\list.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\main.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\pixmap.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\types.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\visual.h"
# End Source File
# Begin Source File

SOURCE=".\gdk--\window.h"
# End Source File
# End Group
# End Target
# End Project
