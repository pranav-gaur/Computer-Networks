#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>


int main(int argc, char *argv[]){

	if(argc != 3){
		printf("command line argument not proper\n");
		return 0;
	}
	struct sockaddr_in server = {0};
	char send_msg[2048] = {0};
	char receive_msg[2048] = {0};

	int socket_desc, port;
	char* ip_addr;
	if((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Socket can't be created");
		return 1;
	}

	ip_addr = argv[1];
	port = atoi(argv[2]);

	server.sin_addr.s_addr = inet_addr(ip_addr); //Local Host//change
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	int connected = connect(socket_desc,(struct sockaddr *)&server,sizeof(struct sockaddr_in));
	if(connected < 0){
		printf("Either connection rejected or server is unreachable!! Exiting, Bye!\n");
		return 1;
	}
	printf("Sucessfully conected with server\n");
	while(1){
		for(int i = 0; i < 2048; i++)
			send_msg[i] = '\0';
		printf("Enter line:");
		fgets(send_msg, 2000, stdin);
		send_msg[strlen(send_msg) - 1] = '\0';

		if(strcmp(send_msg, "exit") == 0)
			break;

		send(socket_desc, send_msg, strlen(send_msg), 0);

		int count = recv(socket_desc, receive_msg, 2000, 0);
		receive_msg[count] = '\0';
		char temp;
		for(int i = 0; i < count/2; i++){
			temp = receive_msg[i];
			receive_msg[i] = receive_msg[count-1-i];
			receive_msg[count-1-i] = temp;
		}

		printf("%s\n", receive_msg);
	}
	close(socket_desc);
	shutdown(socket_desc, 0);
	shutdown(socket_desc, 1);
	shutdown(socket_desc, 2);
	return 0;
}


