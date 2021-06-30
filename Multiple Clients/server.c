#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include<semaphore.h>
#include<pthread.h> 

sem_t sem;
void* connection(void* param);

int main(int argc, char *argv[]){

	if(argc != 2){
		printf("command line argument not proper\n");
		return 0;
	}

	int port = atoi(argv[1]);
	struct sockaddr_in server = {0};
	struct sockaddr_in client = {0};
	sem_init(&sem, 0, 4); // for limiting to 4 connections

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	while(1){
		sem_wait(&sem);
		int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
		if(socket_desc < -1){
			printf("Socket can't be created");
			return 1;
		}
		setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

		int binded = bind(socket_desc,(struct sockaddr *)&server,sizeof(server));

		// printf("binded:%d", binded);
		if(binded < 0){
			printf("not binded\n");
			return 1;
		}

		int listening = listen(socket_desc, 1);
		int len = sizeof(struct sockaddr_in);

		int* accepted = malloc(sizeof(int));
		*accepted = accept(socket_desc, (struct sockaddr*) &client,(socklen_t*) &len);
		if(*accepted < 0){
			printf("not accepting\n");
			return 1;
		}
		close(socket_desc);
		printf("connection established with client.\n");

		pthread_t tid;
		pthread_attr_t attr; 
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_create(&tid,&attr,connection, accepted);

	}
	sem_destroy(&sem);

	return 0;
}


void* connection(void* param){
	int accepted = *((int *)param);
	char receive_msg[2048] = {0};
	char send_msg[2048] = {0};
	int count;
	while((count = recv(accepted, receive_msg, 2000, 0)) > 0){
		receive_msg[count] = '\0';
		
		char temp;
		for(int i = 0; i < count/2; i++){
			temp = receive_msg[i];
			receive_msg[i] = receive_msg[count-1-i];
			receive_msg[count-1-i] = temp;
		}
		printf("%s\n", receive_msg);
		
		for(int i = 0; i < 2048; i++)
			send_msg[i] = '\0';

		printf("Enter line:");
		fgets(send_msg, 2000, stdin);
		send_msg[strlen(send_msg) - 1] = '\0';

		send(accepted, send_msg, strlen(send_msg), 0);
	}
	printf("client left\n");
	close(accepted);
	sem_post(&sem);
	pthread_exit(0);
}

