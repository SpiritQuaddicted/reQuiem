# Microsoft Developer Studio Project File - Name="reQuiem" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=REQUIEM - WIN32 GL DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "reQuiem.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "reQuiem.mak" CFG="REQUIEM - WIN32 GL DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "reQuiem - Win32 GL Release" (based on "Win32 (x86) Application")
!MESSAGE "reQuiem - Win32 GL Debug" (based on "Win32 (x86) Application")
!MESSAGE "reQuiem - Win32 GL Client Debug" (based on "Win32 (x86) Application")
!MESSAGE "reQuiem - Win32 Server Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\release_gl"
# PROP BASE Intermediate_Dir ".\release_gl"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release_gl"
# PROP Intermediate_Dir ".\release_gl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /WX /GX /Ot /Og /Oi /Oy /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "id386" /GT /GA /GF PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /Op /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "id386" /D "GLQUAKE" /FR /GT /GA /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /fo".\release_gl/reQuiem.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dxguid.lib opengl32.lib wsock32.lib winmm.lib libpng.lib zlib.lib /nologo /subsystem:windows /pdb:".\release_gl\reQuiem-gl.pdb" /machine:IX86 /pdbtype:sept /release
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib dxguid.lib opengl32.lib wsock32.lib winmm.lib /nologo /subsystem:windows /machine:IX86 /out:"I:\Quake\Prog\reQuiem.exe" /pdbtype:sept /release
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "reQuiem___Win32_GL_Debug"
# PROP BASE Intermediate_Dir "reQuiem___Win32_GL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\DebugGL"
# PROP Intermediate_Dir ".\DebugGL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /WX /GX /Ot /Og /Oi /Oy /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "id386" /GT /GA /GF PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MLd /W3 /GX /ZI /Od /Oy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "id386" /D "_MBCS" /D "GLQUAKE" /FR"DebugGL/" /GT /GA /GF /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /fo"DebugGL/reQuiem.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"DebugGL/reQuiem.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dxguid.lib opengl32.lib wsock32.lib winmm.lib libpng.lib zlib.lib /nologo /subsystem:windows /pdb:".\release_gl\reQuiem-gl.pdb" /machine:IX86 /pdbtype:sept /release
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib dxguid.lib opengl32.lib wsock32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"DebugGL\reQuiemd.pdb" /debug /machine:IX86 /nodefaultlib:"LIBC" /out:"DebugGL\reQuiemd.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "reQuiem___Win32_GL_Client_Debug"
# PROP BASE Intermediate_Dir "reQuiem___Win32_GL_Client_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\DebugGL"
# PROP Intermediate_Dir ".\DebugGL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MLd /W3 /GX /ZI /Od /Oy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "id386" /D "_MBCS" /FR"DebugGL/" /GT /GA /GF /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MLd /W3 /GX /ZI /Od /Oy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "id386" /D "_MBCS" /D "GLQUAKE" /D "RQM_CL_ONLY" /FR"DebugGL/" /GT /GA /GF /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /fo"DebugGL/reQuiem.res" /d "_DEBUG"
# ADD RSC /l 0x409 /fo"DebugGL/reQuiem.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"DebugGL/reQuiem.bsc"
# ADD BSC32 /nologo /o"Client_Debug/reQuiem.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib dxguid.lib opengl32.lib wsock32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"DebugGL\reQuiemd.pdb" /debug /machine:IX86 /nodefaultlib:"LIBC" /out:"DebugGL\reQuiemd.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib dxguid.lib opengl32.lib wsock32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"DebugGL\reQuiemd.pdb" /debug /machine:IX86 /nodefaultlib:"LIBC" /out:"DebugGL\reQuiemcl.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "reQuiem___Win32_Server_Debug"
# PROP BASE Intermediate_Dir "reQuiem___Win32_Server_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Server_Debug"
# PROP Intermediate_Dir "Server_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MLd /W3 /GX /ZI /Od /Oy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "id386" /D "_MBCS" /D "RQM_SERVER" /D "RQM_CLIENT" /FR"DebugGL/" /GT /GA /GF /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MLd /W3 /GX /ZI /Od /Oy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "id386" /D "_MBCS" /D "RQM_SV_ONLY" /FR"DebugGL/" /GT /GA /GF /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /fo"DebugGL/reQuiem.res" /d "_DEBUG"
# ADD RSC /l 0x409 /fo"DebugGL/reQuiem.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"DebugGL/reQuiem.bsc"
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib dxguid.lib opengl32.lib wsock32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"DebugGL\reQuiemd.pdb" /debug /machine:IX86 /nodefaultlib:"LIBC" /out:"DebugGL\reQuiemd.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib dxguid.lib wsock32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"Server_Debug\reQuiemd.pdb" /debug /machine:IX86 /nodefaultlib:"LIBC" /out:"Server_Debug\reQuiem_svd.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "reQuiem - Win32 GL Release"
# Name "reQuiem - Win32 GL Debug"
# Name "reQuiem - Win32 GL Client Debug"
# Name "reQuiem - Win32 Server Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "OpenGL"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\gl_alias.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_AL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_AL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_AL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_AL=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_AL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_AL=\
	".\gl_model.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_brush.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_BR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_BR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_BR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_BR=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_GL_BR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gl_draw.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_DR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_DR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_DR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_DR=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_drawalias.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_DRA=\
	".\anorm_dots.h"\
	".\anorms.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_DRA=\
	".\anorm_dots.h"\
	".\anorms.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_DRA=\
	".\anorm_dots.h"\
	".\anorms.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_DRA=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_drawsprite.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_DRAW=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_DRAW=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_DRAW=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_DRAW=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_fog.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_FO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_FO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_FO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_FO=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_md2.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_MD=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_MD=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_MD=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_MD=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_MD=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_MD=\
	".\gl_model.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_md3.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_MD3=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_MD3=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_MD3=\
	".\anorm_dots.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_MD3=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_MD3=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_MD3=\
	".\gl_model.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gl_mesh.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_ME=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_ME=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_ME=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_ME=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_part.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_PA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_part_H2.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_PAR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_part_qmb.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_PART=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gl_refrag.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_RE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_RE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_RE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RE=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gl_rlight.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_RL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_RL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_RL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RL=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gl_rmain.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_RM=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_RM=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_RM=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RM=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gl_rmisc.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_RMI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_RMI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_RMI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RMI=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gl_rsurf.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_RS=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_RS=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_RS=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RS=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gl_screen.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_SC=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_SC=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_SC=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_SC=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_sky.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_SK=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_SK=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_SK=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_SK=\
	".\gl_model.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_sprite.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_SP=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_SP=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_SP=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_SP=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_SP=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_SP=\
	".\gl_model.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_texture.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_TE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_TE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_TE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_TE=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=gl_warp.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_GL_WA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_GL_WA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_GL_WA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_GL_WA=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\cd_common.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CD_CO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CD_CO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CD_CO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CD_CO=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cd_win.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CD_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CD_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CD_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CD_WI=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\music.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_MUSIC=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\fmod.h"\
	".\fmod_errors.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_MUSIC=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\fmod.h"\
	".\fmod_errors.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_MUSIC=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\fmod.h"\
	".\fmod_errors.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_MUSIC=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=snd_dma.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SND_D=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SND_D=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_SND_D=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_SND_D=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=snd_mem.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SND_M=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SND_M=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_SND_M=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_SND_M=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=snd_mix.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SND_MI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SND_MI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_SND_MI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_SND_MI=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=snd_win.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SND_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SND_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_SND_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_SND_W=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=cl_input.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CL_IN=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CL_IN=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CL_IN=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CL_IN=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=in_win.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_IN_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_IN_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_IN_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_IN_WI=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=keys.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_KEYS_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_KEYS_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_KEYS_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_KEYS_=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_KEYS_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# End Group
# Begin Group "Server"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=sv_main.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SV_MA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SV_MA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_SV_MA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=sv_move.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SV_MO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SV_MO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_SV_MO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=sv_phys.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SV_PH=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SV_PH=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_SV_PH=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=sv_user.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SV_US=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SV_US=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_SV_US=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# End Group
# Begin Group "VM"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=pr_cmds.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_PR_CM=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_PR_CM=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_PR_CM=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=pr_edict.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_PR_ED=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progdefs_H2.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_PR_ED=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progdefs_H2.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_PR_ED=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progdefs_H2.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=pr_exec.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_PR_EX=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_PR_EX=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_PR_EX=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\progs_H2.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_PROGS=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_PROGS=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_PROGS=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ENDIF 

# End Source File
# End Group
# Begin Group "Video"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=vid_common_gl.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_VID_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_VID_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_VID_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_VID_C=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=vid_wgl.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_VID_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_VID_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_VID_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_VID_W=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# End Group
# Begin Group "Client"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=cl_demo.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CL_DE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzip.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CL_DE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzip.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CL_DE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzip.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CL_DE=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_effect_H2.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CL_EF=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CL_EF=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CL_EF=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CL_EF=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_CL_EF=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cl_main.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CL_MA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CL_MA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CL_MA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CL_MA=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cl_parse.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CL_PA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CL_PA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CL_PA=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CL_PA=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_parse_H2.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CL_PAR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CL_PAR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CL_PAR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CL_PAR=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_parse_qw.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CL_PARS=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CL_PARS=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CL_PARS=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CL_PARS=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cl_tent.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CL_TE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CL_TE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CL_TE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CL_TE=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=net_dgrm.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_NET_D=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_dgrm.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_NET_D=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_dgrm.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_NET_D=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_dgrm.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_NET_D=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_NET_D=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_dgrm.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=net_loop.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_NET_L=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_loop.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_NET_L=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_loop.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_NET_L=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_loop.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_NET_L=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_NET_L=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_loop.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=net_main.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_NET_M=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_vcr.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_NET_M=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_vcr.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_NET_M=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_vcr.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_NET_M=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_NET_M=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_vcr.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=net_vcr.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_NET_V=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_vcr.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_NET_V=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_vcr.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_NET_V=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_vcr.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_NET_V=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_NET_V=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_vcr.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=net_win.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_NET_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_dgrm.h"\
	".\net_loop.h"\
	".\net_wins.h"\
	".\net_wipx.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_NET_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_dgrm.h"\
	".\net_loop.h"\
	".\net_wins.h"\
	".\net_wipx.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_NET_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_dgrm.h"\
	".\net_loop.h"\
	".\net_wins.h"\
	".\net_wipx.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_NET_W=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_NET_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_dgrm.h"\
	".\net_loop.h"\
	".\net_wins.h"\
	".\net_wipx.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=net_wins.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_NET_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_wins.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_NET_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_wins.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_NET_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_wins.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_NET_WI=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_NET_WI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_wins.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=net_wipx.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_NET_WIP=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_wipx.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_NET_WIP=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_wipx.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_NET_WIP=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_wipx.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_NET_WIP=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_NET_WIP=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\net_wipx.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=chase.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CHASE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CHASE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CHASE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CHASE=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cmd.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CMD_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CMD_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CMD_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CMD_C=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_CMD_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=common.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_COMMO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_COMMO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_COMMO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_COMMO=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_COMMO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common_file.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_COMMON=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzip.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_COMMON=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzip.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_COMMON=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzip.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_COMMON=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_COMMON=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzip.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=conproc.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CONPR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\conproc.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CONPR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\conproc.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CONPR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\conproc.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CONPR=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_CONPR=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\conproc.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=console.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CONSO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CONSO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CONSO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CONSO=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_CONSO=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=crc.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CRC_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CRC_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CRC_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CRC_C=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_CRC_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cvar.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_CVAR_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_CVAR_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_CVAR_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_CVAR_=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_CVAR_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dzip.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_DZIP_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzlib.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_DZIP_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzlib.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_DZIP_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzlib.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_DZIP_=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_DZIP_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\dzlib.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hexen2.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_HEXEN=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_HEXEN=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_HEXEN=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_HEXEN=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_HEXEN=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=host.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_HOST_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_HOST_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_HOST_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_HOST_=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_HOST_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=host_cmd.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_HOST_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_HOST_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_HOST_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_HOST_C=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_HOST_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\host_saveload.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_HOST_S=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_HOST_S=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_HOST_S=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_HOST_S=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_HOST_S=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=image.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_IMAGE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\jconfig.h"\
	".\jmorecfg.h"\
	".\jpeglib.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\png.h"\
	".\pngconf.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zone.h"\
	
NODEP_CPP_IMAGE=\
	".\jerror.h"\
	".\jpegint.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_IMAGE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\jconfig.h"\
	".\jmorecfg.h"\
	".\jpeglib.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\png.h"\
	".\pngconf.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zone.h"\
	
NODEP_CPP_IMAGE=\
	".\jerror.h"\
	".\jpegint.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_IMAGE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\jconfig.h"\
	".\jmorecfg.h"\
	".\jpeglib.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\png.h"\
	".\pngconf.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zone.h"\
	
NODEP_CPP_IMAGE=\
	".\gl_model.h"\
	".\jerror.h"\
	".\jpegint.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=mathlib.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_MATHL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_MATHL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_MATHL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_MATHL=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_MATHL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=menu.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_MENU_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\menu_defs.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_MENU_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\menu_defs.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_MENU_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\menu_defs.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_MENU_=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\menu_H2.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_MENU_H=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\menu_defs.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_MENU_H=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\menu_defs.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_MENU_H=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\menu_defs.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_MENU_H=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\model.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_MODEL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_MODEL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_MODEL=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=movie.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_MOVIE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\movie_avi.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_MOVIE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\movie_avi.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_MOVIE=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\movie_avi.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_MOVIE=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\movie_avi.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_MOVIE_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie_avi.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_MOVIE_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie_avi.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
DEP_CPP_MOVIE_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\movie_avi.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_MOVIE_=\
	".\gl_model.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\movie_win.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_MOVIE_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie_avi.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_MOVIE_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie_avi.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
DEP_CPP_MOVIE_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\movie_avi.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_MOVIE_W=\
	".\gl_model.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=nehahra.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_NEHAH=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_NEHAH=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_NEHAH=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_NEHAH=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_NEHAH=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\music.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=sbar.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SBAR_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SBAR_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_SBAR_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_SBAR_=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sbar_H2.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SBAR_H=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SBAR_H=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_SBAR_H=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_SBAR_H=\
	".\gl_model.h"\
	

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=version.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_VERSI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_VERSI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_VERSI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_VERSI=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_VERSI=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=view.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_VIEW_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_VIEW_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\movie.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_VIEW_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_VIEW_=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_VIEW_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=wad.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_WAD_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_WAD_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_WAD_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_WAD_C=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP Exclude_From_Build 1
DEP_CPP_WAD_C=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_WAD_C=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=world.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_WORLD=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_WORLD=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_WORLD=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_WORLD=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_WORLD=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=zone.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_ZONE_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_ZONE_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_ZONE_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_ZONE_=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_ZONE_=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# End Group
# Begin Group "System"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=sys_win.c

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

DEP_CPP_SYS_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\conproc.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

DEP_CPP_SYS_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\conproc.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

DEP_CPP_SYS_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\conproc.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
NODEP_CPP_SYS_W=\
	".\gl_model.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

DEP_CPP_SYS_W=\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\cl_effect_H2.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\conproc.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\draw.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\nehahra.h"\
	".\net.h"\
	".\pr_comp.h"\
	".\prfields.h"\
	".\prglobals.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\server.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	
# ADD BASE CPP /nologo /GX
# ADD CPP /nologo /GX

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Group "OpenGL_h"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=glquake.h
# End Source File
# End Group
# Begin Group "Sound_h"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=cdaudio.h
# End Source File
# Begin Source File

SOURCE=.\fmod.h
# End Source File
# Begin Source File

SOURCE=.\fmod_errors.h
# End Source File
# Begin Source File

SOURCE=.\music.h
# End Source File
# Begin Source File

SOURCE=sound.h
# End Source File
# End Group
# Begin Group "Server_h"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=server.h
# End Source File
# End Group
# Begin Group "Draw_h"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=draw.h
# End Source File
# Begin Source File

SOURCE=render.h
# End Source File
# End Group
# Begin Group "Main_h"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=quakedef.h
# End Source File
# End Group
# Begin Group "Client_h"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\cl_effect_H2.h
# End Source File
# Begin Source File

SOURCE=client.h
# End Source File
# Begin Source File

SOURCE=console.h
# End Source File
# Begin Source File

SOURCE=menu.h
# End Source File
# Begin Source File

SOURCE=.\menu_defs.h
# End Source File
# Begin Source File

SOURCE=view.h
# End Source File
# End Group
# Begin Group "Net_h"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=net.h
# End Source File
# Begin Source File

SOURCE=net_dgrm.h
# End Source File
# Begin Source File

SOURCE=net_loop.h
# End Source File
# Begin Source File

SOURCE=net_vcr.h
# End Source File
# Begin Source File

SOURCE=net_wins.h
# End Source File
# Begin Source File

SOURCE=net_wipx.h
# End Source File
# Begin Source File

SOURCE=protocol.h
# End Source File
# End Group
# Begin Group "Misc_h"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=anorm_dots.h
# End Source File
# Begin Source File

SOURCE=anorms.h
# End Source File
# Begin Source File

SOURCE=bspfile.h
# End Source File
# Begin Source File

SOURCE=cmd.h
# End Source File
# Begin Source File

SOURCE=common.h
# End Source File
# Begin Source File

SOURCE=conproc.h
# End Source File
# Begin Source File

SOURCE=crc.h
# End Source File
# Begin Source File

SOURCE=cvar.h
# End Source File
# Begin Source File

SOURCE=.\dzip.h
# End Source File
# Begin Source File

SOURCE=.\dzlib.h
# End Source File
# Begin Source File

SOURCE=input.h
# End Source File
# Begin Source File

SOURCE=keys.h
# End Source File
# Begin Source File

SOURCE=mathlib.h
# End Source File
# Begin Source File

SOURCE=.\model.h
# End Source File
# Begin Source File

SOURCE=modelgen.h
# End Source File
# Begin Source File

SOURCE=movie.h
# End Source File
# Begin Source File

SOURCE=movie_avi.h
# End Source File
# Begin Source File

SOURCE=nehahra.h
# End Source File
# Begin Source File

SOURCE=sbar.h
# End Source File
# Begin Source File

SOURCE=screen.h
# End Source File
# Begin Source File

SOURCE=spritegn.h
# End Source File
# Begin Source File

SOURCE=sys.h
# End Source File
# Begin Source File

SOURCE=version.h
# End Source File
# Begin Source File

SOURCE=vid.h
# End Source File
# Begin Source File

SOURCE=wad.h
# End Source File
# Begin Source File

SOURCE=winquake.h
# End Source File
# Begin Source File

SOURCE=world.h
# End Source File
# Begin Source File

SOURCE=zone.h
# End Source File
# End Group
# Begin Group "VM No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pr_comp.h
# End Source File
# Begin Source File

SOURCE=.\prfields.h
# End Source File
# Begin Source File

SOURCE=.\prglobals.h
# End Source File
# Begin Source File

SOURCE=.\progdefs.h
# End Source File
# Begin Source File

SOURCE=.\progdefs_H2.h
# End Source File
# Begin Source File

SOURCE=.\progs.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# Begin Source File

SOURCE=quake.ico
# End Source File
# Begin Source File

SOURCE=.\quake.rc

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Asm Files"

# PROP Default_Filter "s"
# Begin Source File

SOURCE=math.s

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\release_gl
InputPath=math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\DebugGL
InputPath=math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\DebugGL
InputPath=math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\Server_Debug
InputPath=math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=snd_mixa.s

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\release_gl
InputPath=snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\DebugGL
InputPath=snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\DebugGL
InputPath=snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=sys_x86.s

!IF  "$(CFG)" == "reQuiem - Win32 GL Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\release_gl
InputPath=sys_x86.s
InputName=sys_x86

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\DebugGL
InputPath=sys_x86.s
InputName=sys_x86

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "reQuiem - Win32 GL Client Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\DebugGL
InputPath=sys_x86.s
InputName=sys_x86

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "reQuiem - Win32 Server Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\Server_Debug
InputPath=sys_x86.s
InputName=sys_x86

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > "$(OUTDIR)"\"$(InputName)".spp "$(InputPath)" 
	gas2masm < "$(OUTDIR)"\"$(InputName)".spp >"$(OUTDIR)"\"$(InputName)".asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del "$(OUTDIR)"\"$(InputName)".spp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\changelog.txt
# End Source File
# Begin Source File

SOURCE=.\JoeQuake.readme
# End Source File
# Begin Source File

SOURCE=.\reQuiem.txt
# End Source File
# End Group
# Begin Group "Linux files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cd_linux.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\snd_linux.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\sys_linux.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\vid_glx.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
