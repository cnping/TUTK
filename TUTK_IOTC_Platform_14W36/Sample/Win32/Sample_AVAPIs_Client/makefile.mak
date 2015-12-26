all:
	cl  /D "UNICODE" /D "_UNICODE" /W3 /O2 AVAPIs_Client_win32.cpp /I ..\..\..\Include /I . /I .\ /c
	link /OUT:"AVAPIs_Client_win32.exe" /NOLOGO /LIBPATH:..\..\..\Lib\Win32 IOTCAPIs.lib AVAPIs.lib AVAPIs_Client_win32.obj

CLEAN :
	-@erase ".\*.obj"
	-@erase ".\AVAPIs_Client_win32.exe"