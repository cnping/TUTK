/*****
Setting Password for Tunnel, Server side.
History:
1.0.0.0,	Jelly Jian,	1st version

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>


#define TUNNEL_CMD_SET_PASSWORD 0x0A
#define TUNNEL_CMD_SET_PASSWORD_SUCCESS 0x0B
#define TUNNEL_CMD_SET_PASSWORD_FAIL 0x0C

typedef struct st_PasswdCmd
{
	int cmdType;
	int len;
	unsigned char data[24];
} PasswdCmd;


int CreateSocket(unsigned short port)
{
	int listen_fd;
	struct sockaddr_in listen_addr;

	// socket
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror ("socket failed!\n");
		return -1;
	}
	bzero ((char *)&listen_addr, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(port);
	// bind
	if (bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) <0) {
		perror ("bind failed\n");
		close(listen_fd);
		return -1;
	}
	// Listen
	if (listen(listen_fd, 5) <0) {
		close(listen_fd);
		perror ("listen failed\n");
		return -1;
	}

	return listen_fd;
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


int main(int argc, char* argv[])
{
	int  listen_fd, client_fd;
	struct sockaddr_in client_addr;
	unsigned short listen_port = 9000;

	socklen_t length;
	int ret=0;
	char buf[ sizeof(PasswdCmd) ];
	PasswdCmd *pCmd=NULL;
	FILE *fp=NULL;

	if (argc != 2)
	{
	        fprintf (stderr, "Usage: %s [Listen_Port]\n", argv[0]);
	        return -1;
	}
	listen_port = atoi(argv[1]);

	if( (listen_fd = CreateSocket(listen_port))<0)
		return -1;
	printf("Server Listening...\n");

	while(1)
	{
		length = sizeof(client_addr);

		if ((client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &length)) <0) {
			perror ("accept fail.\n");
			continue;
		}
		printf("Client Connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
		// read cmd
		ret = readn(client_fd, buf, sizeof(buf));
		// client socket close or error
		if( ret<=0)
		{
			printf("Client closed or error!!\n");
			close(client_fd);
			continue;
		}
		pCmd = (PasswdCmd* )buf;
		// printf("Read cmd: CmdType=%d, len=%d, passwd=%s\n", pCmd->cmdType, pCmd->len, pCmd->data);
		printf("Set new Password: \"%s\" ", pCmd->data);

		// write file
		if( (fp = fopen("passwd.txt","w"))!=NULL )
		{
			ret = fwrite( pCmd->data, sizeof(char), pCmd->len, fp );
			pCmd->cmdType = (ret==pCmd->len) ? TUNNEL_CMD_SET_PASSWORD_SUCCESS: TUNNEL_CMD_SET_PASSWORD_FAIL;
			fclose(fp);
		}
		else
			pCmd->cmdType = TUNNEL_CMD_SET_PASSWORD_FAIL;

		if( pCmd->cmdType == TUNNEL_CMD_SET_PASSWORD_SUCCESS)
			printf("Success.\n\n");
		else
			printf("Fail.\n\n");

		// write resp
		//printf("Write Cmd: %d bytes, CmdType=%d, len=%d, passwd=%s\n",  sizeof(buf), pCmd->cmdType, pCmd->len, pCmd->data);
		if ( write(client_fd, buf, sizeof(buf)) > 0 )
		{
		}
		close(client_fd);

	}
	return 0;
}
