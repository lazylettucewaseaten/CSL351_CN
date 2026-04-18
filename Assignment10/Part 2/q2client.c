#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>

#define MAXI 3600 

typedef struct {
    int seq_num;
    struct timeval send_time;
} packethead;


typedef struct {
    long bytesrcv;
    double trtt;
    int packetsrcv;
} secondsts;

secondsts stats[MAXI];
int running = 1;
int sockfd;
struct timeval start_test_time;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void *receive_packets(void *arg) {
    char buffer[65535];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    while (running) {
        int n = recvfrom(sockfd, buffer, 65535, 0, (struct sockaddr *) &from_addr, &from_len);
        if (n > 0) {
            struct timeval rcvtime;
            gettimeofday(&rcvtime, NULL);
            
            packethead *hdr = (packethead *)buffer;
            double rtt = (rcvtime.tv_sec - hdr->send_time.tv_sec) * 1000.0 + (rcvtime.tv_usec - hdr->send_time.tv_usec) / 1000.0;

            int secindex = rcvtime.tv_sec - start_test_time.tv_sec;
            if (secindex >= 0 && secindex < MAXI) {
                stats[secindex].bytesrcv += n;
                stats[secindex].trtt += rtt;
                stats[secindex].packetsrcv++;
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <server_ip> <port_no> <duration_sec> <interval_us> <packet_size>\n", argv[0]);
        exit(1);
    }

    char *serverip = argv[1];
    int portno = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int intervalus = atoi(argv[4]);
    int packetsz = atoi(argv[5]);

    if (duration > MAXI) duration = MAXI;
    memset(stats, 0, sizeof(stats));

    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    struct timeval timeout = {0, 100000}; 
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    inet_pton(AF_INET, serverip, &serv_addr.sin_addr);

    char *buffer = (char *)malloc(packetsz);
    memset(buffer, 'A', packetsz);

    gettimeofday(&start_test_time, NULL);
    pthread_t rcvthread;
    if (pthread_create(&rcvthread, NULL, receive_packets, NULL) != 0) {
        error("Failed to create receiver thread");
    }

    int seq = 1;
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    
    fprintf(stderr, "testing for %d seconds. sending packets\n", duration);
    
    while ((current_time.tv_sec - start_test_time.tv_sec) < duration) {
        packethead *header = (packethead *)buffer;
        header->seq_num = seq++;
        gettimeofday(&header->send_time, NULL);

        sendto(sockfd, buffer, packetsz, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        
        if (intervalus > 0) {
            usleep(intervalus);
        }
        gettimeofday(&current_time, NULL);
    }

    running = 0;
    pthread_join(rcvthread, NULL);

    printf("Second,Throughput(Mbps),AvgRTT(ms)\n");
    for (int i = 0; i < duration; i++) {
        double mbps=(stats[i].bytesrcv*8.0)/1000000.0;
        double avgsrtt = 0.0;
        if (stats[i].packetsrcv>0){
            avgsrtt=stats[i].trtt/stats[i].packetsrcv;
        }
        printf("%d,%.3f,%.3f\n", i + 1, mbps, avgsrtt);
    }

    free(buffer);
    close(sockfd);
    return 0;
}