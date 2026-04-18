#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include <time.h>
#include <opencv2/opencv.hpp>

#define MAXLINE 65535
typedef struct {
    int id;
    char qname[256];
    char ipaddr[16];
} DNS;


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//The error() function is identical to that in the server, as are the variables sockfd, portno, and n. The variable serv_addr will contain the address of the server to which we want to connect. It is of type struct sockaddr_in.

int main(int argc, char *argv[])
{
int VIDEOPORT=8790;
srand(time(NULL));

if (argc < 5) {
        fprintf(stderr, "Usage: %s <dns_server_ip> <dns_server_port> <video_port> <domain_name>\n", argv[0]);
        exit(0);
    }
    

    int tcp_sockfd, dns_port;
    struct sockaddr_in dns_addr;
    struct hostent *server;
    DNS dns_req;

    dns_port=atoi(argv[2]);
    VIDEOPORT=atoi(argv[3]);
    tcp_sockfd=socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0) error("ERROR opening TCP socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &dns_addr, sizeof(dns_addr));
    dns_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&dns_addr.sin_addr.s_addr, server->h_length);
    dns_addr.sin_port = htons(dns_port);

    if (connect(tcp_sockfd, (struct sockaddr *)&dns_addr, sizeof(dns_addr)) < 0) 
        error("ERROR connecting to DNS server");


    bzero(&dns_req, sizeof(DNS));
    dns_req.id=rand()%65536;
    // dns_req.transaction_id = 12340370; 
    strncpy(dns_req.qname, argv[4], 255);


    write(tcp_sockfd, &dns_req, sizeof(DNS));
    read(tcp_sockfd, &dns_req, sizeof(DNS));
    close(tcp_sockfd);

    printf("\nDNS Resolution Output\n");
    printf("Domain: %s\n", dns_req.qname);
    printf("IP Address: %s\n", dns_req.ipaddr);
    printf("-----------------------------\n\n");

    if (strcmp(dns_req.ipaddr, "NOT_FOUND") == 0) {
        fprintf(stderr, "DNS Resolution Failed. Exiting.\n");
        exit(1);
    }

    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in video_serv_addr;
    char recvMessage[MAXLINE];

    bzero(&video_serv_addr, sizeof(video_serv_addr));
    video_serv_addr.sin_family = AF_INET;
    video_serv_addr.sin_port = htons(VIDEOPORT);
    inet_pton(AF_INET, dns_req.ipaddr, &video_serv_addr.sin_addr);

    socklen_t addressLength = sizeof(video_serv_addr);
    cv::namedWindow("video stream", cv::WINDOW_AUTOSIZE);

    printf("starting video stream. 'x' to exit.\n");

    while(1) {
        char keep_alive[] = "ALIVE";
        sendto(udp_sockfd, keep_alive, strlen(keep_alive), 0, (struct sockaddr*)&video_serv_addr, addressLength);


        int n=recvfrom(udp_sockfd, recvMessage, MAXLINE, 0, NULL, NULL);

        if (n>0) {
            std::vector<uchar> raw_data(recvMessage, recvMessage + n);
            cv::Mat decoded_frame = cv::imdecode(raw_data, cv::IMREAD_COLOR);
            if (!decoded_frame.empty()) {
                cv::imshow("video stream", decoded_frame);
            }
            char c = (char)cv::waitKey(1);
            if (c=='x') break;
        }
    }

    cv::destroyWindow("video stream");
    close(udp_sockfd);
    return 0;
}
