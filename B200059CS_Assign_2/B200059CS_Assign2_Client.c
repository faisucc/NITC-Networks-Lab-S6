#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048

volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void server_print()
{
    printf("\r%s", " => ");
    fflush(stdout);
}

void remove_newline_character(char* arr, int length)
{
  int i;
  for (i = 0; i < length; i++)
  {
    if (arr[i] == '\n')
	{
      arr[i] = '\0';
      break;
    }
  }
}

void catch_exit(int sig)
{
    flag = 1;
}

void sending_handler()
{
  	char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {};

	while(1)
	{
		server_print();
		fgets(message, LENGTH, stdin);
		remove_newline_character(message, LENGTH);

		if(strcmp(message, "/exit") == 0 || strcmp(message, "/quit") == 0 || strcmp(message, "/part") == 0)
		{
			break;
		}
		else
		{
			sprintf(buffer, "%s: %s\n", name, message);
			send(sockfd, buffer, strlen(buffer), 0);
		}

		bzero(message, LENGTH);
		bzero(buffer, LENGTH + 32);
	}
	catch_exit(2);
}

void receiving_handler()
{
	char message[LENGTH] = {};
	while (1)
	{
		int receive_length = recv(sockfd, message, LENGTH, 0);
		if (receive_length > 0)
		{
			printf("%s", message);
			server_print();
		}
		else if(receive_length == 0)
		{
			break;
		}
		memset(message, 0, sizeof(message));
	}
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	char *loopback_ip = "127.0.0.1";
	int port = atoi(argv[1]);

	signal(SIGINT, catch_exit);

	struct sockaddr_in server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(loopback_ip);
	server_addr.sin_port = htons(port);

	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err < 0)
	{
		printf("Cannot connect.\n");
		exit(1);
	}

	char name[32];
	strcpy(name,"Client ");

	send(sockfd, name, 32, 0);
	printf("Chatroom entered.\n");

	pthread_t thread_to_send_message;
	if(pthread_create(&thread_to_send_message, NULL, (void *) sending_handler, NULL) != 0)
	{
		printf("Could not create thread to send message.\n");
		exit(1);
	}

	pthread_t thread_to_receive_message;
	if(pthread_create(&thread_to_receive_message, NULL, (void *) receiving_handler, NULL) != 0)
	{
		printf("Could not create thread to receive message.\n");
		exit(1);
	}

	while (1)
	{
		if(flag)
		{
			printf("\nExited.\n");
			break;
    	}
	}

	close(sockfd);

	return 0;
}






















//gcc -Wall -g3 -fsanitize=address -pthread client.c -o client
