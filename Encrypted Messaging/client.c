#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

int padding = RSA_PKCS1_PADDING;
RSA *createRSA(unsigned char *key, int isPublic)
{
    RSA *rsa = NULL;
    BIO *keybio;
    keybio = BIO_new_mem_buf(key, -1);
    if (keybio == NULL)
    {
        printf("Failed to create BIO of key");
        return 0;
    }
    if (isPublic)
    {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
    }
    else
    {
        rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
    }
    if (rsa == NULL)
    {
        printf("Failed to create RSA structure info");
    }
    return rsa;
}

int public_encrypt(unsigned char *data, int data_len, unsigned char *key, unsigned char *encrypted)
{
    RSA *rsa = createRSA(key, 1);
    int result = RSA_public_encrypt(data_len, data, encrypted, rsa, padding);
    return result;
}

int private_decrypt(unsigned char *enc_data, int data_len, unsigned char *key, unsigned char *decrypted)
{
    RSA *rsa = createRSA(key, 0);
    int result = RSA_private_decrypt(data_len, enc_data, decrypted, rsa, padding);
    return result;
}

void printLastError(char *msg)
{
    char *err = malloc(130);
    ;
    ERR_load_crypto_strings();
    ERR_error_string(ERR_get_error(), err);
    printf("%s ERROR: %s\n", msg, err);
    free(err);
}

void exitProcess(pid_t pid, int sockfd)
{
    close(sockfd);
    kill(pid, SIGKILL);
    wait(NULL);
    exit(0);
}
int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        printf("Command line arguments not proper\n");
        return 0;
    }
    int sockfd, portno;
    struct hostent *server;
    server = gethostbyname(argv[1]);
    portno = atoi(argv[2]);
    if (server == NULL)
    {
        printf("Cannot reach the server\n");
        exit(0);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(-1);
    }

    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Cannot reach the server\n");
        exit(-1);
    }
    pid_t pid = fork();
    while (pid == -1)
    {
        pid = fork();
    }
    if (pid != 0)
    {
        char buff;
        int size;
        FILE *fp;
        char publicKey[4096];
        fp = fopen(argv[4], "r");
        if (fp == NULL)
        {
            printf("Encryption key not found\n");
            exitProcess(pid, sockfd);
        }
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(publicKey, size, 1, fp);
        fclose(fp);
        while (1)
        {
            unsigned char encrypted[2048] = {};
            int encrypted_length;
            char write_buffer[2048], message[1025];
            printf("\n\nEnter message:-\n\n");
            fgets(message, 1025, stdin);
            // printf("%d\n", strlen(message));
            encrypted_length = public_encrypt(message, strlen(message), publicKey, encrypted);
            // printf("%d\n", encrypted_length);
            if (encrypted_length == -1)
            {
                printLastError("Public Encrypt failed");
                exitProcess(pid, sockfd);
            }
            write(sockfd, encrypted, encrypted_length);
            if (strcmp(message, "exit\n") == 0)
            {
                exitProcess(pid, sockfd);
            }
        }
    }
    else
    {
        char privateKey[7000];
        int size;
        FILE *fp;
        fp = fopen(argv[3], "r");
        if (fp == NULL)
        {
            printf("Decryption key not found\n");
            exitProcess(getppid(), sockfd);
        }
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(privateKey, size, 1, fp);
        fclose(fp);
        while (1)
        {
            unsigned char decrypted[4098] = {};
            unsigned char encrypted[4098] = {};
            int encrypted_length = 1035;
            int decrypted_length;
            int x = read(sockfd, encrypted, encrypted_length);
            // printf("%d\n", x);
            decrypted_length = private_decrypt(encrypted, encrypted_length, privateKey, decrypted);
            if (strcmp(decrypted, "exit\n") == 0)
            {
                exitProcess(pid, sockfd);
            }
            printf("\n\nMessage recieved:- \n\n");
            for (int i = 0; i < encrypted_length; i++)
            {
                printf("%c", encrypted[i]);
            }
            printf("\n\nDecrypted message:- \n\n");
            if (decrypted_length == -1)
            {
                printLastError("Private Decrypt failed");
                exitProcess(getppid(), sockfd);
            }
            printf("%s", decrypted);
            printf("\n\nEnter message:-\n\n");
        }
    }
}