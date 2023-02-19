#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

void err(const char *error)
{
	perror(error);
	exit(1);
}


char *strrev(char *str)
{
	char c, *front, *rear;
	if(!str || !*str)
		return str;
	for(front = str, rear = str + strlen(str) - 1; front < rear; front++, rear--)
	{
		c = *front;
		*front  = *rear;
		*rear = c;
	}
	
	return str;
}

int main(int countOfArguments, char *argumentValues[])
{
	int socketFileDescriptor, portNumber, n;
	struct sockaddr_in server_address;
	struct hostent *server;
	char buffer[255];
	if(countOfArguments < 3)
	{
		fprintf(stderr,"usage %s hostname port\n", argumentValues[0]);
		exit(0);
	}
	
	portNumber = atoi(argumentValues[2]);
	socketFileDescriptor = socket(AF_INET,SOCK_STREAM,0);
	
	if(socketFileDescriptor < 0)
	{
		err("Could not open socket. \n");
	}
	
	server = gethostbyname(argumentValues[1]);
	if(server == NULL)
	{
		fprintf(stderr,"Host does not exist.");
	}
	
	bzero((char*) &server_address,sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char*) server->h_addr, (char*) &server_address.sin_addr.s_addr, server->h_length);
	server_address.sin_port = htons(portNumber);
	
	if(connect(socketFileDescriptor, (struct sockaddr* )&server_address, sizeof(server_address)) < 0)
	{
		err("could not connect. \n");
	}
	
	
	bzero(buffer,255);
	fgets(buffer,255,stdin);
	buffer[strlen(buffer) - 1] = '\0';
	printf("Client sends: %s\n", buffer);
	n = write(socketFileDescriptor,buffer,strlen(buffer));
	if(n < 0)
		err("error on writing");
	bzero(buffer,255);
	n = read(socketFileDescriptor,buffer,255);
	printf("Client receives back: %s", buffer);
		
		
	
	close(socketFileDescriptor);
	return 0;
}
