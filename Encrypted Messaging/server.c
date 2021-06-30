#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    int portno;
    if (argc != 2)
    {
        printf("cli not proper\n");
        return 0;
    }
    portno = atoi(argv[1]);
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(-1);
    }
    struct sockaddr_in server_address, client_address;
    bzero((char *)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("ERROR on binding");
        exit(-2);
    }
    listen(sockfd, 2);
    socklen_t client_len;
    client_len = sizeof(client_address);
    int newsockfd1 = accept(sockfd, (struct sockaddr *)&client_address, &client_len);
    if (newsockfd1 < 0)
    {
        perror("Error while accepting");
        exit(-1);
    }
    else
    {
        printf("Successfully connected to client 1\n");
    }
    int newsockfd2 = accept(sockfd, (struct sockaddr *)&client_address, &client_len);
    if (newsockfd2 < 0)
    {
        perror("Error while accepting");
        exit(-1);
    }
    else
    {
        printf("Successfully connected to client 2\n");
    }
    pid_t pid = fork();
    int encrypted_length = 1035;
    if (pid != 0)
    {
        unsigned char encrypted[2048] = {};
        while (1)
        {
            int x = 0, y = 0;
            if ((x = read(newsockfd1, encrypted, encrypted_length)))
            {
                y = write(newsockfd2, encrypted, encrypted_length);
                printf("Message recieved from client 1. Forwarding to client 2\n");
            }
        }
    }
    else
    {
        unsigned char encrypted[2048] = {};
        while (1)
        {
            if (read(newsockfd2, encrypted, encrypted_length))
            {
                printf("Message recieved from client 2. Forwwarding to client 1\n");
                write(newsockfd1, encrypted, encrypted_length);
            }
        }
    }
}