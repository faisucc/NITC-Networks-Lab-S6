#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

typedef struct{
    char URL[100];
    char resolvedIP[100];
} localCache;

localCache *cache[5];

// int cacheLength = 1;
int nextToUpdate = 1;

int search_through_cache(char *buffer)
{
    for(int i=0; i<5; i++)
    {
        if(cache[i] != NULL)
        {
            if(strcmp(buffer,cache[i]->URL) == 0)
                return i;
        }
    }

	return -1;
}

void addToCache(char *buffer, char *IP)
{
	localCache *c = (localCache *)malloc(sizeof(localCache));
	strcpy(c->URL,buffer);
	strcpy(c->resolvedIP,IP);
	cache[nextToUpdate%5] = c;
	nextToUpdate++;
}

int main()
{
	struct sockaddr_in server, client;
	char buffer[256];
	
	localCache *c = (localCache *)malloc(sizeof(localCache));
	
	strcpy(c->URL,"google.com");
	strcpy(c->resolvedIP,"83.12.10.01");
	cache[0] = c;

	
	
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0)
	{
		printf("Could not open socket.");
		exit(1);
	}
	
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(9999);
	if(bind(sockfd,(struct sockaddr*)&server,sizeof(server)) < 0)
	{
		printf("Could not bind.");
		exit(1);
	}
	int len = sizeof(struct sockaddr_in);

	while(1)
	{
		bzero(buffer,256);
		int n = recvfrom(sockfd,buffer,256,0,(struct sockaddr*)&client,&len);
		if(n < 0)
		{
			printf("Receiving failed.");
			exit(1);
		}
		//main operation comes here; 

		char IPtoSend[20];
		bzero(IPtoSend,20);
		// strcpy(IPtoSend,search_through_cache(buffer));
		int val = search_through_cache(buffer);
		if(val > -1)
		{
			strcpy(IPtoSend,cache[val]->resolvedIP);
			n = sendto(sockfd,IPtoSend,20,0,(struct sockaddr*) &client,len);
			if(n < 0)
			{
				printf("Sending failed.");
				exit(1);
			}
			printf("Sent from server: %s\n", IPtoSend);
		}
		else
		{
			printf("Pinging Root DNS Server. \n");
			struct sockaddr_in newserveraddr;
			char IPReceived[20];

			//for root server: 
			memset(&newserveraddr,0,sizeof(newserveraddr));
			newserveraddr.sin_family = AF_INET;
			newserveraddr.sin_addr.s_addr = INADDR_ANY;
			newserveraddr.sin_port = htons(9989);

			sendto(sockfd,(char*)buffer,strlen(buffer),0,(struct sockaddr*)&newserveraddr,sizeof(newserveraddr));

			int r = recvfrom(sockfd,(char*)IPReceived,20,0,(struct sockaddr*)&newserveraddr,&len);
			IPReceived[r]='\0';

			//update cache here

			addToCache(buffer,IPReceived);
			
			printf("%s\n",IPReceived);
			sendto(sockfd,(char*)IPReceived,strlen(IPReceived),0,(struct sockaddr*)&client,len);


			

		}

	}
	
	return 0;

}
