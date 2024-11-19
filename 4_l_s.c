#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 3000
#define ACK "ACK"

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sockfd;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    char buffer[1024];

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error("socket");
    }

    // Configure the server address
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        error("bind");
    }

    while (1) {
        // Receive data
        if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientaddr, &addrlen) < 0) {
            error("recvfrom");
        }

        printf("Received: %s\n", buffer);

        // Simulate noise by randomly deciding whether to send an ACK or not
        if (rand() % 5) {
            // Send ACK
            if (sendto(sockfd, ACK, sizeof(ACK), 0, (struct sockaddr *)&clientaddr, addrlen) < 0) {
                error("sendto");
            }
            printf("Sent ACK.\n");
        } else {
            // Simulate no ACK (for noise)
            printf("Simulated noise: No ACK sent.\n");
        }
    }

    close(sockfd);
    return 0;
}
