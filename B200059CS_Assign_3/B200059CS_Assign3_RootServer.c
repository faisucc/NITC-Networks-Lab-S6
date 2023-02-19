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

localCache *cache[10];

int nextToUpdate = 2;

void str_trim (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

int search_through_cache(char *buffer)
{
    for(int i=0; i<10; i++)
    {
        if(cache[i] != NULL)
        {
        int n=strcmp(buffer,cache[i]->URL);
        // printf("%s\n",buffer);
        // printf("%s + length stored in cache: %ld\n",cache[i]->URL,strlen(cache[i]->URL));
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
	cache[nextToUpdate%10] = c;
	nextToUpdate++;
}

int main()
{
	struct sockaddr_in server, client;
	char buffer[256];
	
	localCache *c1 = (localCache *)malloc(sizeof(localCache));
	
	strcpy(c1->URL,"google.com");
	strcpy(c1->resolvedIP,"83.12.10.01");
	cache[0] = c1;

    localCache *c2 = (localCache *)malloc(sizeof(localCache));
	
	strcpy(c2->URL,"yahoo.com");
	strcpy(c2->resolvedIP,"94.00.94.14");
	cache[1] = c2;

	int cacheLength = 2;
	
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0)
	{
		printf("Could not open socket.");
		exit(1);
	}
	
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(9989);
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

		// printf("%s + length: %ld\n", buffer,strlen(buffer));
		//main operation comes here; 

		char IPtoSend[20];
		bzero(IPtoSend,20);
		// strcpy(IPtoSend,search_through_cache(buffer));
		str_trim(buffer,strlen(buffer));
		// printf("%s + length after trim : %ld\n", buffer,strlen(buffer));

		int val = search_through_cache(buffer);
		// printf("%d",val);
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
			printf("Pinging TLD DNS Server. \n");
			struct sockaddr_in newserveraddr;
			char IPReceived[20];

			//for TLD server: 
			memset(&newserveraddr,0,sizeof(newserveraddr));
			newserveraddr.sin_family = AF_INET;
			newserveraddr.sin_addr.s_addr = INADDR_ANY;
			newserveraddr.sin_port = htons(9979);

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
