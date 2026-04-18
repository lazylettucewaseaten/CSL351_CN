#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#define MAXHOPS 64
#define PROBESPERHOP 3
#define RECVTIMEOUT 3
#define DESTPORT 33434 
#define PACKETSZ 52

void error(const char *msg) {
    perror(msg);
    exit(1);
}

double getrtt(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 + 
           (end->tv_usec - start->tv_usec) / 1000.0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <destination_ip_or_domain>\n", argv[0]);
        fprintf(stderr, "run with sudo!\n");
        exit(1);
    }

    const char *targhost = argv[1];
    struct hostent *host_info = gethostbyname(targhost);
    if (!host_info) {
        fprintf(stderr, "ERROR: Could not resolve hostname %s\n", targhost);
        exit(1);
    }

    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    memcpy(&destaddr.sin_addr, host_info->h_addr, host_info->h_length);
    destaddr.sin_port = htons(DESTPORT);

    char dest_ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &destaddr.sin_addr, dest_ip_str, sizeof(dest_ip_str));

    printf("traceroute to %s (%s), %d hops max, %d byte packets\n", 
           targhost, dest_ip_str, MAXHOPS, PACKETSZ);

    int sendsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sendsock < 0) error("ERROR creating UDP socket");

    int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (recv_sock < 0) {
        error("ERROR creating Raw socket , used sudo ?");
    }

    struct timeval timeout;
    timeout.tv_sec = RECVTIMEOUT;
    timeout.tv_usec = 0;
    if (setsockopt(recv_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        error("ERROR setting receive timeout");
    }

    char sendbuf[PACKETSZ];
    memset(sendbuf, 'A', sizeof(sendbuf)); 
    int destreached = 0;

    for (int ttl = 1; ttl <= MAXHOPS && !destreached; ttl++) {

        if (setsockopt(sendsock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
            error("ERROR setting TTL");
        }

        printf("%2d  ", ttl);
        
        char lastrouterip[INET_ADDRSTRLEN] = "";

        for (int probe = 0; probe < PROBESPERHOP; probe++) {

            destaddr.sin_port = htons(DESTPORT + ttl + probe);

            struct timeval start_time, end_time;
            gettimeofday(&start_time, NULL);

            if (sendto(sendsock, sendbuf, sizeof(sendbuf), 0, 
                       (struct sockaddr*)&destaddr, sizeof(destaddr)) < 0) {
                error("ERROR sending probe");
            }

            char recv_buf[512];
            struct sockaddr_in router_addr;
            socklen_t addr_len = sizeof(router_addr);
            int recv_len;
            int check = 0;
            struct icmp *icmp_hdr = NULL;

            while (!check) {
                recv_len = recvfrom(recv_sock, recv_buf, sizeof(recv_buf), 0, 
                                        (struct sockaddr*)&router_addr, &addr_len);
                
                gettimeofday(&end_time, NULL);

                if (recv_len < 0) {
                    break;
                }

                struct ip *ip_hdr = (struct ip *)recv_buf;
                int ip_hdr_len = ip_hdr->ip_hl << 2; 
                icmp_hdr = (struct icmp *)(recv_buf + ip_hdr_len);
                
                if (icmp_hdr->icmp_type == ICMP_TIME_EXCEEDED || icmp_hdr->icmp_type == ICMP_DEST_UNREACH) {
                    struct ip *inner_ip = (struct ip *)((char *)icmp_hdr + 8);
                    struct udphdr *inner_udp = (struct udphdr *)((char *)inner_ip + (inner_ip->ip_hl << 2));
                    
                    if (ntohs(inner_udp->uh_dport) == (DESTPORT + ttl + probe)) {
                        check = 1;
                    }
                }
            }

            if (!check) {
                printf("* ");
                continue; 
            }
        
            double rtt = getrtt(&start_time, &end_time);
            char routerip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &router_addr.sin_addr, routerip, sizeof(routerip));

            if (strcmp(lastrouterip, routerip) != 0) {
                printf("%s  ", routerip);
                strcpy(lastrouterip, routerip);
            }

            printf("%.3f ms  ", rtt);

            if (icmp_hdr->icmp_type == ICMP_DEST_UNREACH && icmp_hdr->icmp_code == ICMP_UNREACH_PORT) {
                destreached = 1;
            }
        }
        printf("\n");
    }

    close(sendsock);
    close(recv_sock);
    return 0;
}