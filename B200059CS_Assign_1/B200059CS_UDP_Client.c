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

int main(int argc, char *args[])
{
	struct sockaddr_in server, client;
	struct hostent *hp;
	char buffer[256];
	
	if(argc != 3)
	{
		fprintf(stderr,"Port error\n");
		exit(1);
	}
	
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0)
	{
		err("Couldn't make socket.");
	}
	
	server.sin_family = AF_INET;
	hp = gethostbyname(args[1]);
	
	if(hp == 0)
	{
		err("Host not identified.");
	}
	
	bcopy((char*) hp->h_addr,(char*)&server.sin_addr,hp->h_length);
	server.sin_port = htons(atoi(args[2]));
	int len = sizeof(struct sockaddr_in);
	bzero(buffer,256);
	fgets(buffer,256,stdin);
	buffer[strlen(buffer)-1] = '\0';
	printf("Sent to client: %s\n",buffer);
	int n = sendto(sockfd,buffer,strlen(buffer),0,(struct sockaddr*)&server,len);
	if(n < 0)
	{
		err("send failed");
	}
	bzero(buffer,256);
	n = recvfrom(sockfd,buffer,256,0,(struct sockaddr*)&client,&len);
	if(n < 0)
	{
		err("recieve failed");
	}
	printf("Received from server: %s",buffer);
}	



