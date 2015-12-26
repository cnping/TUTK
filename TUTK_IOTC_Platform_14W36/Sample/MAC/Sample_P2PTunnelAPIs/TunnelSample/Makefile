all: 	
	gcc -m32 -O2 -Wall P2PTunnelServer.c -I./ -I./include -I../../../../Include -lpthread -L../../../../Lib/MAC -lIOTCAPIs_P2PTunnel -o P2PTunnelServer
	gcc -m32 -O2 -Wall P2PTunnelAgent.c -I./ -I./include -I../../../../Include -lpthread -L../../../../Lib/MAC -lIOTCAPIs_P2PTunnel -o P2PTunnelAgent
	
clean:
	rm -rf *.o *~ *.bak P2PTunnelServer P2PTunnelAgent
