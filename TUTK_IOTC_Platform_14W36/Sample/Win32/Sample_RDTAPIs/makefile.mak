all:
	cl  /D "UNICODE" /D "_UNICODE" /W3 /O2 RDTServer.cpp /I ..\..\..\Include /I . /I .\ /c
	link /OUT:"RDTServer.exe" /NOLOGO /LIBPATH:..\..\..\Lib\Win32 IOTCAPIs.lib RDTAPIs.lib RDTServer.obj
	cl  /D "UNICODE" /D "_UNICODE" /W3 /O2 RDTClient.cpp /I ..\..\..\Include /I . /I .\ /c
	link /OUT:"RDTClient.exe" /NOLOGO /LIBPATH:..\..\..\Lib\Win32 IOTCAPIs.lib RDTAPIs.lib RDTClient.obj

CLEAN :
	-@erase "$(INTDIR)*.obj"
	-@erase "$(OUTDIR)*.exe"