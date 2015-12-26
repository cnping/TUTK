/*****
Setting Password for Tunnel, Agent side.
History:
1.0.0.0,	Jelly Jian,	1st version

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#define TUNNEL_CMD_SET_PASSWORD 0x0A
#define TUNNEL_CMD_SET_PASSWORD_SUCCESS 0x0B
#define TUNNEL_CMD_SET_PASSWORD_FAIL 0x0C

typedef struct st_PasswdCmd
{
	int cmdType;
	int len;
	unsigned char data[24];
} PasswdCmd;

char gServerName[100]="\0";
unsigned short gServerPort = 9000;
struct sockaddr_in gServerAddr;

// It will try all IP addr until one of them connect success.
// Return socket id if success, others -1.
static int ConnectToServer()
{
	int connected = 0, skt=-1, i=0;

	char  **ppAddr;
	struct hostent *pHostEnt=NULL;
	char str[100]="\0";

	pHostEnt = gethostbyname(gServerName);
	if( pHostEnt==NULL)
	{
		printf("Fail to resolve host name!!\n");
		return -1;
	}
	skt = (int)socket(AF_INET, SOCK_STREAM, 0);
	if(skt<0)
	{
		printf("Create Socket error!\n");
		return -1;
	}

	// Connect to one alived TPNS servers
	ppAddr=pHostEnt->h_addr_list;
	for( i=0 ; ppAddr[i] ;  i++)
	{
		memcpy(&gServerAddr.sin_addr, ppAddr[i], pHostEnt->h_length);
		gServerAddr.sin_port = htons(gServerPort);
		gServerAddr.sin_family = AF_INET;

		inet_ntop( pHostEnt->h_addrtype, ppAddr[i], str, sizeof(str) );
		printf("Try to connect to: %s\n",  str);

		if (connect(skt, (struct sockaddr *) &gServerAddr, sizeof(struct sockaddr_in) )==0 )
		{
			connected = 1;
			break;
		}
	}
	if( !connected)
	{
		close(skt);
		return -1;
	}

	return skt;
}

// implement TCP read 'nsize' to buf
int readn(int fd, char *buf, int nsize)
{
	int nbytes = 0, ret = 0;
	while(nbytes < nsize)
	{
		if ((ret = read(fd, &buf[nbytes], nsize- nbytes)) <= 0)
			return ret;
		nbytes += ret;
	}
	return nbytes;
}


int main(int argc, char *argv[])
{
	int fd =0;
	char buf[ sizeof(PasswdCmd) ];
	PasswdCmd *pCmd = (PasswdCmd*)buf;

	if (argc!=4)
	{
		printf("Argment error!!\n");
		printf("Usage: ./passc [Server_Addr] [Port] [Password]\n");
		return -1;
	}
	strcpy(gServerName, argv[1]);
	gServerPort = atoi(argv[2]);
	if(strlen(argv[3])>23)
	{
		printf("Error! length of password must < 24 chars.\n");
		return -1;
	}
	strcpy((char*)pCmd->data, argv[3] );
	//printf("gServerName=%s, gServerPort=%d, password=%s\n", gServerName, gServerPort, pCmd->data);

	printf("Connect to server\n");
	fd = ConnectToServer();
	if (fd<0)
	{
		printf("Connect fail.\n");
		return -1;
	}

	pCmd->cmdType = TUNNEL_CMD_SET_PASSWORD;
	pCmd->len = strlen((char*)pCmd->data);
	printf("Setting password \"%s\" ", pCmd->data);
	if (  write(fd, buf, sizeof(buf)) > 0 )
	{
	}

	readn(fd, buf, sizeof(buf));

	if(pCmd->cmdType==TUNNEL_CMD_SET_PASSWORD_SUCCESS)
		printf("success!!\n");
	else
		printf("fail!!\n");
	close(fd);

	return 0;
}
