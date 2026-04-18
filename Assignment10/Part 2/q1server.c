#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int portno = atoi(argv[1]);


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    printf("UDP Echo Server is running on port %d\n", portno);

    char *buffer = (char *)malloc(65535);
    if (buffer == NULL) error("ERROR allocating memory");

    while (1) {
        clilen = sizeof(cli_addr);
        
        int n = recvfrom(sockfd, buffer, 65535, 0, (struct sockaddr *) &cli_addr, &clilen);
        if (n < 0) {
            perror("ERROR in recvfrom");
            continue;
        }

        int sent = sendto(sockfd, buffer, n, 0, (struct sockaddr *) &cli_addr, clilen);
        if (sent < 0) {
            perror("ERROR in sendto");
        }
    }

    free(buffer);
    close(sockfd);
    return 0;
}
