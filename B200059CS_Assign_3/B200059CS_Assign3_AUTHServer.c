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

localCache *cache[200];

int count = 0;

// int nextToUpdate = 4;

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
    for(int i=0; i<count; i++)
    {
        if(cache[i] != NULL)
        {
        int n=strcmp(buffer,cache[i]->URL);
        // printf("strcmp gives: %d\n", n);
        // printf("%s\n",buffer);
        // printf("%s + length stored in cache: %ld\n",cache[i]->URL,strlen(cache[i]->URL));
            if(strcmp(buffer,cache[i]->URL) == 0)
                return i;
        }
    }

	return -1;
}

// void addToCache(char *buffer, char *IP)
// {
// 	localCache *c = (localCache *)malloc(sizeof(localCache));
// 	strcpy(c->URL,buffer);
// 	strcpy(c->resolvedIP,IP);
// 	cache[nextToUpdate%5] = c;
// 	nextToUpdate++;
// }



void init_cache(char *buffer, char *IP)
{
	str_trim(buffer,strlen(buffer));
    localCache *c = (localCache *)malloc(sizeof(localCache));
	strcpy(c->URL,buffer);
	strcpy(c->resolvedIP,IP);
	cache[count] = c;
    count++;
}

int main()
{
	struct sockaddr_in server, client;
	char buffer[256];
	
    init_cache("google.com","83.12.10.01");
    init_cache("yahoo.com","94.00.94.14");
    init_cache("xyz.com","192.8.1.100");
    init_cache("abc.com","12.12.12.14");
    init_cache("yt.com","101.17.23.00");
    init_cache("pqr.com","192.168.5.60");
    init_cache("msnbc.com","191.111.205.23");
    init_cache("ufc.com","54.83.91.101");
    init_cache("twitter.com","122.54.13.66");
    init_cache("bing.com","15.15.15.151");
    init_cache("reddit.com","32.18.87.14");
    init_cache("milton.com","133.144.21.95");

    // for(int i=0; i<count; i++)
    // {
    //     printf("%s : %s\n", cache[i]->URL,cache[i]->resolvedIP);
    // }

	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0)
	{
		printf("Could not open socket.");
		exit(1);
	}
	
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(9969);
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

		char IPtoSend[20];
		bzero(IPtoSend,20);

		str_trim(buffer,strlen(buffer));


		int val = search_through_cache(buffer);
        // printf("cache search returns: %d\n",val);

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
			// sprintf("Pinging Authoritative DNS Server. \n");
			struct sockaddr_in newserveraddr;
			char IPReceived[20];
			
			printf("URL Does Not Exist\n");
            strcpy(IPReceived,"Doesn't Exist");
			sendto(sockfd,(char*)IPReceived,strlen(IPReceived),0,(struct sockaddr*)&client,len);
		}
	}
	

	return 0;
}
