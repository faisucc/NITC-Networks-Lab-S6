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

int main(int argc, char *args[])
{
	struct sockaddr_in server, client;
	char buffer[256];
	
	if(argc < 2)
	{
		fprintf(stderr,"Port not entered\n");
		exit(1);
	}
	
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0)
	{
		err("Could not open socket.");
	}
	
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(args[1]));
	if(bind(sockfd,(struct sockaddr*)&server,sizeof(server)) < 0)
	{
		err("Could not bind.");
	}
	int len = sizeof(struct sockaddr_in);
	int n = recvfrom(sockfd,buffer,256,0,(struct sockaddr*)&client,&len);
	if(n < 0)
	{
		err("Receiving failed.");
	}
	printf("Received from client: %s\n", buffer);
	strrev(buffer);
	printf("Sent from server: %s", buffer);
	n = sendto(sockfd,buffer,256,0,(struct sockaddr*) &client,len);
	if(n < 0)
	{
		err("Sending failed.");
	}
}
