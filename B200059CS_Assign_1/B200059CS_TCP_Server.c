#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

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
	if(countOfArguments < 2)
	{
		fprintf(stderr,"No port number entered; exiting program. \n");
		exit(1);
	}
	
	char buffer[255];
	struct sockaddr_in server_address, client_address;
	socklen_t client_length;
	
	int socketFileDescriptor = socket(AF_INET,SOCK_STREAM,0);
	if(socketFileDescriptor < 0)
	{
		err("Could not open socket. \n");
	}
	
	bzero((char*) &server_address,sizeof(server_address));
	int portNumber = atoi(argumentValues[1]);
	
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(portNumber);
	
	if(bind(socketFileDescriptor, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		err("Could not bind. \n");
	}
	
	listen(socketFileDescriptor,5); //maximum limit of clients allowed to connect at a time
	client_length = sizeof(client_address);
	
	int newSocketFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *) &client_address, &client_length);
	
	if(newSocketFileDescriptor < 0)
	{
		err("Could not accept. \n");
	}
	
	int n;
	
	bzero(buffer,255);
	n = read(newSocketFileDescriptor,buffer,255);
	if(n < 0)
	{
		err("Could not read. \n");
	}
	printf("Recieved from Client: %s\n", buffer);
	strrev(buffer);
	printf("Reply from Server: %s", buffer);
	send(newSocketFileDescriptor,buffer,strlen(buffer),0);
		
	
	close(newSocketFileDescriptor);
	close(socketFileDescriptor);
	return 0;
}


