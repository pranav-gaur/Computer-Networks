#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <resolv.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

int ReadHttpStatus(int sock)
{
    char c;
    char buff[1024] = "", *ptr = buff + 1;
    int bytes_received, status;
    printf("Begin Response ..\n");
    while (bytes_received = recv(sock, ptr, 1, 0))
    {
        if (bytes_received == -1)
        {
            perror("ReadHttpStatus");
            exit(1);
        }

        if ((ptr[-1] == '\r') && (*ptr == '\n'))
            break;
        ptr++;
    }
    *ptr = 0;
    ptr = buff + 1;

    sscanf(ptr, "%*s %d ", &status);

    printf("%s\n", ptr);
    printf("status=%d\n", status);
    printf("End Response ..\n");
    return (bytes_received > 0) ? status : 0;
}

int ParseHeader(int sock)
{
    char c;
    char buff[1024] = "", *ptr = buff + 4;
    int bytes_received, status;
    printf("Begin HEADER ..\n");
    while (bytes_received = recv(sock, ptr, 1, 0))
    {
        if (bytes_received == -1)
        {
            perror("Parse Header");
            exit(1);
        }

        if (
            (ptr[-3] == '\r') && (ptr[-2] == '\n') &&
            (ptr[-1] == '\r') && (*ptr == '\n'))
            break;
        ptr++;
    }

    *ptr = 0;
    ptr = buff + 4;

    if (bytes_received)
    {
        ptr = strstr(ptr, "Content-Length:");
        if (ptr)
        {
            sscanf(ptr, "%*s %d", &bytes_received);
        }
        else
            bytes_received = -1; //unknown size

        printf("Content-Length: %d\n", bytes_received);
    }
    printf("End HEADER ..\n");
    return bytes_received;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Enter URL of file as cli\n");
        return 0;
    }
    BIO *certbio = NULL;
    BIO *outbio = NULL;
    X509 *cert = NULL;
    X509_NAME *certname = NULL;
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    SSL *ssl;
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    certbio = BIO_new(BIO_s_file());
    outbio = BIO_new_fp(stdout, BIO_NOCLOSE);
    if (SSL_library_init() < 0)
    {
        BIO_printf(outbio, "Could not initialize the OpenSSL library !\n");
    }
    method = SSLv23_client_method();
    if ((ctx = SSL_CTX_new(method)) == NULL)
    {
        BIO_printf(outbio, "Unable to create a new SSL context structure.\n");
    }
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    ssl = SSL_new(ctx);
    char host[256], path[256], filename[256];
    int portno = 80;

    if (strstr(argv[1], "http://"))
    {
        portno = 80;
        argv[1] += 7;
    }
    else if (strstr(argv[1], "https://"))
    {
        portno = 443;
        argv[1] += 8;
    }
    int count = 0;
    for (char *c = argv[1]; *c != '/'; c++)
    {
        host[count] = *c;
        count++;
    }
    host[count] = '\0';
    argv[1] += count + 1;
    for (int i = 0; i <= strlen(argv[1]); i++)
    {
        path[i] = argv[1][i];
        // printf("%c", path[i]);
    }
    // printf("\n\n\n");
    count = 0;
    for (int i = strlen(path) - 1; i > -1 && path[i] != '/' && path[i] != '=' && path[i] != ':' && path[i] != '?'; i--)
    {
        filename[count] = path[i];
        count++;
    }
    filename[count] = '\0';
    for (int i = 0; i < count / 2; i++)
    {
        char temp = filename[i];
        filename[i] = filename[count - 1 - i];
        filename[count - 1 - i] = temp;
    }

    int sock, bytes_received;
    char send_data[1024], recv_data[1024], *p;
    struct sockaddr_in server_addr;
    struct hostent *he;

    he = gethostbyname(host);
    if (he == NULL)
    {
        herror("Can't find host");
        exit(1);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Can't connect to socket");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(server_addr.sin_zero), 8);

    printf("Connecting ...\n");
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Cannot connect to server");
        exit(1);
    }
    if (portno == 443)
    {
        SSL_set_fd(ssl, sock);
        if (SSL_connect(ssl) != 1)
            BIO_printf(outbio, "Error: Could not build a SSL session.\n");
        else
            BIO_printf(outbio, "Successfully enabled SSL/TLS session. \n");
        snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host);
        SSL_write(ssl, send_data, sizeof(send_data));
        char c;
        char buff[1024] = "", *ptr = buff + 1;
        int bytes_received, status;
        printf("Begin Response ..\n");
        while (bytes_received = SSL_read(ssl, ptr, 1))
        {
            if (bytes_received == -1)
            {
                perror("ReadHttpStatus");
                exit(1);
            }

            if ((ptr[-1] == '\r') && (*ptr == '\n'))
                break;
            ptr++;
        }
        *ptr = 0;
        ptr = buff + 1;

        sscanf(ptr, "%*s %d ", &status);

        printf("%s\n", ptr);
        printf("status=%d\n", status);
        printf("End Response ..\n");

        char rbuff[1024] = "", *nptr = rbuff + 4;
        int contentlength;
        printf("Begin HEADER ..\n");
        while (contentlength = SSL_read(ssl, nptr, 1))
        {
            if (contentlength == -1)
            {
                perror("Parse Header");
                exit(1);
            }

            if (
                (nptr[-3] == '\r') && (nptr[-2] == '\n') &&
                (nptr[-1] == '\r') && (*nptr == '\n'))
                break;
            nptr++;
        }

        *nptr = 0;
        nptr = rbuff + 4;

        if (contentlength)
        {
            nptr = strstr(nptr, "Content-Length:");
            if (nptr)
            {
                sscanf(nptr, "%*s %d", &contentlength);
            }
            else
            {
                contentlength = -1; //unknown size
            }
            printf("Content-Length: %d\n", contentlength);
        }
        printf("End HEADER ..\n");
        if (contentlength > 0 && bytes_received > 0)
        {
            int bytes = 0;
            FILE *fd = fopen(filename, "wb");
            printf("Saving data...\n\n");

            while (bytes_received = SSL_read(ssl, recv_data, 1024))
            {
                if (bytes_received == -1)
                {
                    perror("recieve");
                    exit(3);
                }

                fwrite(recv_data, 1, bytes_received, fd);
                bytes += bytes_received;
                printf("Bytes recieved: %d from %d\n", bytes, contentlength);
                if (bytes >= contentlength)
                    break;
            }
            fclose(fd);
        }
    }
    else
    {
        printf("Sending data ...\n");

        snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host);

        if (send(sock, send_data, strlen(send_data), 0) == -1)
        {
            perror("send");
            exit(2);
        }
        printf("Data sent.\n");

        printf("Recieving data...\n\n");

        int contentlengh;

        if (ReadHttpStatus(sock) && (contentlengh = ParseHeader(sock)))
        {

            int bytes = 0;
            FILE *fd = fopen(filename, "wb");
            printf("Saving data...\n\n");

            while (bytes_received = recv(sock, recv_data, 1024, 0))
            {
                if (bytes_received == -1)
                {
                    perror("recieve");
                    exit(3);
                }

                fwrite(recv_data, 1, bytes_received, fd);
                bytes += bytes_received;
                printf("Bytes recieved: %d from %d\n", bytes, contentlengh);
                if (bytes == contentlengh)
                    break;
            }
            fclose(fd);
        }
    }
    SSL_free(ssl);
    close(sock);
    X509_free(cert);
    SSL_CTX_free(ctx);
    printf("\n\nDone.\n\n");
    return 0;
}