#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

int main(int argc, char *args[])
{
	struct sockaddr_in server, client;
	struct hostent *hp;
	char buffer[256];
	int option = 1;
	
	if(argc != 3)
	{
		fprintf(stderr,"Port error\n");
		exit(1);
	}
	
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(setsockopt(sockfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
	{
		perror("Could not call setsockopt.\n");
    	exit(1);
	}
	if(sockfd < 0)
	{
		perror("Couldn't make socket.");
	}
	
	server.sin_family = AF_INET;
	hp = gethostbyname(args[1]);
	
	if(hp == 0)
	{
		perror("Host not identified.");
	}
	
	bcopy((char*) hp->h_addr,(char*)&server.sin_addr,hp->h_length);
	server.sin_port = htons(atoi(args[2]));
	int len = sizeof(struct sockaddr_in);

	while(1)
	{
		bzero(buffer,256);
		fgets(buffer,256,stdin);
		buffer[strlen(buffer)-1] = '\0';
		// printf("Sent to client: %s\n",buffer);
		int n = sendto(sockfd,buffer,strlen(buffer),0,(struct sockaddr*)&server,len);
		if(n < 0)
		{
			perror("send failed");
		}
		bzero(buffer,256);
		char op[20];
		bzero(op,20);
		n = recvfrom(sockfd,op,20,0,(struct sockaddr*)&client,&len);
		if(n < 0)
		{
			perror("recieve failed");
		}
		printf("Received from server: %s\n",op);
	}
	return 0;
}	
