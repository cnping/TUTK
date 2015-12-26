all: 	
	gcc -s -m32 -O2 -Wall P2PTunnelServer.c -I./ -I../../../Include -L../../../Lib/Linux/x86 -lP2PTunnelAPIs -lRDTAPIs -lIOTCAPIs -lpthread -lrt -o P2PTunnelServer
	gcc -s -m32 -O2 -Wall P2PTunnelAgent.c -I./ -I../../../Include -L../../../Lib/Linux/x86  -lP2PTunnelAPIs -lRDTAPIs -lIOTCAPIs -lpthread -lrt -o P2PTunnelAgent
	gcc -s -m32 -O2 -Wall PasswordServer.c -o PasswordServer
	gcc -s -m32 -O2 -Wall PasswordAgent.c -o PasswordAgent
	
clean:
	rm -rf *.o *~ *.bak P2PTunnelServer P2PTunnelAgent PasswordServer PasswordAgent
