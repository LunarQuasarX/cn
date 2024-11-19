#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 3000
#define TIMEOUT 2  // Timeout in seconds
#define ACK "ACK"
#define DATA "Hello 22bce212 there. Checking the connection"

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sockfd;
    struct sockaddr_in serveraddr;
    char buffer[1024];
    fd_set readfds;
    struct timeval tv;
    int retry_count = 0;

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error("socket");
    }

    // Configure the server address
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(PORT);

    while (retry_count < 5) {
        printf("Sending: %s\n", DATA);
        if (sendto(sockfd, DATA, sizeof(DATA), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
            error("sendto");
        }

        // Set up timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;

        // Wait for acknowledgment or timeout
        if (select(sockfd + 1, &readfds, NULL, NULL, &tv) == 0) {
            // Timeout
            printf("Timeout. Retransmitting...\n");
            retry_count++;
        } else {
            // Receive acknowledgment
            if (recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL) < 0) {
                error("recvfrom");
            }

            if (strcmp(buffer, ACK) == 0) {
                printf("Received ACK. Message was received correctly.\n");
                break;
            } else {
                printf("Received incorrect ACK. Retransmitting...\n");
                retry_count++;
            }
        }
    }

    if (retry_count == 5) {
        printf("Failed to receive ACK after 5 retries. Exiting...\n");
    }

    close(sockfd);
    return 0;
}