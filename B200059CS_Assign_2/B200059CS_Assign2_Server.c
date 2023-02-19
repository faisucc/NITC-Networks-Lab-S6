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

#define max_clients_allowed 10
#define size 2048

static _Atomic unsigned int cli_count = 0;
static int uid = 10;
pthread_mutex_t client_to_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} new_client;

new_client *clients[max_clients_allowed];

void server_print()
{
    printf("\r%s", " => ");
    fflush(stdout);
}

void print_client_addr(struct sockaddr_in addr)
{
    printf("%d.%d.%d.%d",addr.sin_addr.s_addr & 0xff,(addr.sin_addr.s_addr & 0xff00) >> 8,(addr.sin_addr.s_addr & 0xff0000) >> 16,(addr.sin_addr.s_addr & 0xff000000) >> 24);
}

void add_to_client_list(new_client *cl)
{
	pthread_mutex_lock(&client_to_lock);
	for(int i=0; i < max_clients_allowed; i++)
	{
		if(!clients[i])
		{
			clients[i] = cl;
			break;
		}
	}
	pthread_mutex_unlock(&client_to_lock);
}

void remove_from_queue(int uid)
{
	pthread_mutex_lock(&client_to_lock);
	for(int i=0; i < max_clients_allowed; i++)
	{
		if(clients[i])
		{
			if(clients[i]->uid == uid)
			{
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&client_to_lock);
}

void send_message(char *s, int uid)
{
	pthread_mutex_lock(&client_to_lock);

	for(int i=0; i<max_clients_allowed; i++)
	{
		if(clients[i])
		{
			if(clients[i]->uid != uid)
			{
				if(write(clients[i]->sockfd, s, strlen(s)) < 0)
				{
					perror("Writing failed.\n");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&client_to_lock);
}

void remove_newline_character (char* arr, int length)
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

void tostring(char str[], int num)
{
    int i, rem, len = 0, n;
 
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

void *handle_client(void *arg)
{
	char buff_out[size];
	char name[32];
	int leave_flag = 0;
	char num[5];
	char new_buff[size];
	// char num[5];

	cli_count++;
	new_client *cli = (new_client *)arg;

	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1)
	{
		printf("Please enter name correctly.\n");
		leave_flag = 1;
	}
	else
	{
		tostring(num,cli_count);
		strcat(name,num);
		strcpy(cli->name, name);
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		send_message(buff_out, cli->uid);
	}

	bzero(buff_out, size);
	bzero(num,strlen(num));

	while(1)
	{
		if (leave_flag)
		{
			break;
		}

		int receive = recv(cli->sockfd, buff_out, size, 0);
		if (receive > 0)
		{
			if(strlen(buff_out) > 0)
			{
				strcpy(new_buff,"Client ");
				tostring(num,cli->uid - 9);
				strcat(new_buff,num);
				strcat(new_buff,": ");
				strcat(new_buff,buff_out);
				send_message(new_buff, cli->uid);
				remove_newline_character(new_buff, strlen(new_buff));
				printf("%s \n", new_buff);
				memset(new_buff,0,strlen(new_buff));
				memset(num,0,strlen(num));
			}
		} 
		else if(receive == 0 || strcmp(buff_out, "/exit") == 0 || strcmp(buff_out, "/quit") || strcmp(buff_out, "/part"))
		{
			sprintf(buff_out, "%s has left\n", cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->uid);
			leave_flag = 1;
		}
		else 
		{
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buff_out, size);
		bzero(new_buff,strlen(new_buff));
		bzero(num,strlen(num));
	}

	close(cli->sockfd);
	remove_from_queue(cli->uid);
	free(cli);
	cli_count--;
	pthread_detach(pthread_self());

	return NULL;
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);
	int option = 1;
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);

	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
	{
		perror("Could not call setsockopt.\n");
    	exit(1);
	}
	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Could not bind. \n");
		exit(1);
	}

	if (listen(listenfd, 10) < 0)
	{
		perror("Could not listen. \n");
		exit(1);
	}

	printf("Chatroom created.\n");

	while(1)
	{
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

		if((cli_count + 1) == max_clients_allowed)
		{
			printf("Max clients reached. Rejected: ");
			print_client_addr(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

		new_client *cli = (new_client *)malloc(sizeof(new_client));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

		add_to_client_list(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

		sleep(1);
	}

	return 0;
}



















// gcc -Wall -g3 -fsanitize=address -pthread server.c -o server
