# Microsoft Developer Studio Project File - Name="gtkmm_static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=gtkmm_static - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gtkmm_static.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gtkmm_static.mak" CFG="gtkmm_static - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gtkmm_static - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "gtkmm_static - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gtkmm_static - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "./src" /I "./gdk--" /I "." /I "c:\gnu\src" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"gtk--1.2.static.lib"

!ELSEIF  "$(CFG)" == "gtkmm_static - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "." /I "c:\gnu\src" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"gtk--1.2.static.debug.lib"

!ENDIF 

# Begin Target

# Name "gtkmm_static - Win32 Release"
# Name "gtkmm_static - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\gtk--\accelgroup.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\accellabel.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\adjustment.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\alignment.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\arrow.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\aspectframe.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\base.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\bin.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\box.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\button.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\buttonbox.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\calendar.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\checkbutton.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\checkmenuitem.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\clist.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\colorselection.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\combo.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\container.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\ctree.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\curve.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\data.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\dialog.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\drawingarea.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\editable.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\entry.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\eventbox.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\fileselection.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\fixed.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\fontselection.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\frame.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\gtk--.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\handlebox.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\image.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\inputdialog.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\invisible.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\item.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\label.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\layout.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\list.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\listitem.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\main.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\marshal.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\menu.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\menubar.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\menuitem.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\menushell.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\misc.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\notebook.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\object.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\optionmenu.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\packer.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\paned.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\pixmap.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\plug.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\preview.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\progress.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\progressbar.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\proxy.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\radiobutton.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\radiomenuitem.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\range.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\ruler.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\scale.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\scrollbar.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\scrolledwindow.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\separator.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\socket.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\spinbutton.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\statusbar.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\style.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\table.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\tearoffmenuitem.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\text.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\tipsquery.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\togglebutton.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\toolbar.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\tooltips.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\tree.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\treeitem.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\viewport.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\widget.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\window.cpp"
# End Source File
# Begin Source File

SOURCE=".\gtk--\wrap.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=".\gtk--\accelgroup.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\accellabel.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\adjustment.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\alignment.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\arrow.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\aspectframe.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\base.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\bin.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\box.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\button.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\buttonbox.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\calendar.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\checkbutton.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\checkmenuitem.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\clist.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\colorselection.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\combo.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\container.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\ctree.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\curve.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\data.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\dialog.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\drawingarea.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\editable.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\entry.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\eventbox.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\fileselection.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\fixed.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\fontselection.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\frame.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\handlebox.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\image.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\inputdialog.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\invisible.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\item.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\label.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\layout.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\list.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\listitem.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\main.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\marshal.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\menu.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\menubar.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\menuitem.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\menushell.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\misc.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\notebook.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\object.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\optionmenu.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\packer.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\paned.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\pixmap.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\plug.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\preview.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\progress.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\progressbar.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\proxy.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\radiobutton.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\radiomenuitem.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\range.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\ruler.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\scale.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\scrollbar.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\scrolledwindow.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\separator.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\socket.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\spinbutton.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\statusbar.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\style.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\table.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\tearoffmenuitem.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\text.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\tipsquery.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\togglebutton.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\toolbar.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\tooltips.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\tree.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\treeitem.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\viewport.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\widget.h"
# End Source File
# Begin Source File

SOURCE=".\gtk--\window.h"
# End Source File
# End Group
# End Target
# End Project
