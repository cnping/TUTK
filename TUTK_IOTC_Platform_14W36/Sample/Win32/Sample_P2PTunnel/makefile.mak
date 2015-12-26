all:
	cl  /D "UNICODE" /D "_UNICODE" /W3 /O2 P2PTunnelAgent.cpp /I ..\..\..\Include /I . /I .\ /c
	link /OUT:"P2PTunnelAgent.exe" /NOLOGO /LIBPATH:..\..\..\Lib\Win32\ IOTCAPIs.lib RDTAPIs.lib P2PTunnelAPIs.lib P2PTunnelAgent.obj

CLEAN :
	-@erase ".\*.obj"
	-@erase "$(OUTDIR)P2PTunnelAgent.exe"