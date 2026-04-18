#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>

typedef struct {
    int seq_num;
    struct timeval send_time;
} packethead;

void error(const char *msg) {
    perror(msg);
    exit(1);
}
int main(int argc, char *argv[]) {

    if (argc < 6) {
        fprintf(stderr, "Usage: %s <serverip> <port> <msgcount> <intervalms> <packetsize>\n", argv[0]);
        exit(1);
    }
    char *serverip=argv[1];
    int portno=atoi(argv[2]);
    int msgcount=atoi(argv[3]);
    int intervalms=atoi(argv[4]);
    int packetsz=atoi(argv[5]);
    if (packetsz < sizeof(packethead)) {
        fprintf(stderr, "Packet size must be at least %lu bytes\n", sizeof(packethead));
        exit(1);
    }

    int sockfd;
    struct sockaddr_in serv_addr;


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    if (inet_pton(AF_INET, serverip, &serv_addr.sin_addr) <= 0) {
        error("ERROR invalid server IP address");
    }


    struct timeval timeout;
    timeout.tv_sec = 1;  
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        error("ERROR setting socket timeout");
    }

    char *buffer = (char *)malloc(packetsz);
    if (buffer == NULL) error("ERROR allocating memory");
    memset(buffer, 'A', packetsz); 

    printf("PING %s:%d %d bytes of data.\n", serverip, portno, packetsz);

    int recvcnt = 0;
    int lost_count = 0;
    double trtt = 0.0;

    for (int i=1;i<=msgcount;i++){
        packethead *head=(packethead *)buffer;
        head->seq_num = i;
        gettimeofday(&head->send_time, NULL);

        int n=sendto(sockfd,buffer,packetsz,0,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
        if (n<0){
            perror("ERROR writing to socket");
            lost_count++;
            continue;
        }


        struct sockaddr_in from_addr;
        socklen_t from_len=sizeof(from_addr);
        n=recvfrom(sockfd,buffer,packetsz,0,(struct sockaddr *)&from_addr,&from_len);

        if (n < 0) {

            printf("Request timeout for icmp_seq %d\n", i);
            lost_count++;
        } else {

            struct timeval rcvtime;
            gettimeofday(&rcvtime, NULL);

            packethead *rcvhead = (packethead *)buffer;
            

            if (rcvhead->seq_num == i) {

                double rtt = (rcvtime.tv_sec - rcvhead->send_time.tv_sec) * 1000.0 + (rcvtime.tv_usec - rcvhead->send_time.tv_usec) / 1000.0;
                printf("%d bytes from %s: seq=%d time=%.3f ms\n", n, serverip, rcvhead->seq_num, rtt);
                
                recvcnt++;
                trtt += rtt;
            } else {
                printf("Received out-of-order packet. Expected seq=%d, got seq=%d\n", i, rcvhead->seq_num);
            }
        }


        usleep(intervalms * 1000); 
    }


    printf("\n--- %s ping statistics ---\n", serverip);
    double lossp=((double)lost_count / msgcount) * 100.0;
    printf("%d packets transmitted, %d received, %.1f%% packet loss\n", msgcount, recvcnt, lossp);
    
    if (recvcnt>0) {
        printf("Average RTT: %.3f ms\n", trtt / recvcnt);
    }

    free(buffer);
    close(sockfd);
    return 0;
}
