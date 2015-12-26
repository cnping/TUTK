all: 	
	gcc -m32 -O2 -Wall AVAPIs_Server.c -I./ -I../../../Include ../../../Lib/MAC/libIOTCAPIs_ALL.a   -lpthread -o AVAPIs_Server
	gcc -m32 -O2 -Wall AVAPIs_Client.c -I./ -I../../../Include ../../../Lib/MAC/libIOTCAPIs_ALL.a   -lpthread -o AVAPIs_Client

clean:
	rm -rf *.o *~ *.bak AVAPIs_Server AVAPIs_Client
