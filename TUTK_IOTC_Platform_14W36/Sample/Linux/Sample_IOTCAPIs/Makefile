all: 	
	gcc -m32 -O2 -Wall IOTCAPIs_Client.c -I./ -I../../../Include -L../../../Lib/Linux/x86 -lIOTCAPIs -lpthread -o IOTCAPIs_Client
	gcc -m32 -O2 -Wall IOTCAPIs_Device.c -I./ -I../../../Include -L../../../Lib/Linux/x86 -lIOTCAPIs -lpthread -o IOTCAPIs_Device

clean:
	rm -rf *.o *~ *.bak IOTCAPIs_Client IOTCAPIs_Device
