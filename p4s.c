#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 3000
#define ACK "ACK"

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    WSADATA wsaData;
    int sockfd;
    struct sockaddr_in serveraddr, clientaddr;
    int addrlen = sizeof(clientaddr);
    char buffer[1024];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error("WSAStartup");
    }

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        error("socket");
    }

    // Configure the server address
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
        error("bind");
    }

    while (1) {
        // Receive data from client
        int recvlen = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&clientaddr, &addrlen);
        if (recvlen > 0) {
            buffer[recvlen] = '\0';
            printf("Received: %s\n", buffer);

            // Send acknowledgment to client
            if (sendto(sockfd, ACK, strlen(ACK), 0, (struct sockaddr *)&clientaddr, addrlen) == SOCKET_ERROR) {
                error("sendto");
            }
        }
    }

    // Cleanup
    closesocket(sockfd);
    WSACleanup();

    return 0;
}
