#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 12345
#define PACKET_SIZE 2    // 1 byte for data + 1 byte for sequence number
#define NOISE_PROBABILITY 0.1

int simulate_noise() {
    return (rand() / (double)RAND_MAX) < NOISE_PROBABILITY;
}

void receive_file(SOCKET sock) {
    char packet[PACKET_SIZE];
    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);
    FILE *file = fopen("sheesh.txt", "w");
    
    if (file == NULL) {
        perror("File opening failed");
        return;
    }

    printf("Receiving file character by character...\n");

    while (1) {
        if (recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &addr_len) > 0) {
            // Simulate packet loss
            if (simulate_noise()) {
                printf("Simulating packet loss for sequence number %d\n", packet[0]);
                continue;
            }

            int sequence_number = packet[0];
            char character = packet[1];
            
            // Print the received character and sequence number
            printf("Received: %c (Sequence Number: %d)\n", character, sequence_number);

            // Write the character to file
            fputc(character, file);

            // Simulate ACK loss
            if (simulate_noise()) {
                printf("Simulating ACK loss for sequence number %d\n", sequence_number);
            } else {
                char ack[2];
                snprintf(ack, sizeof(ack), "%d", sequence_number);
                sendto(sock, ack, strlen(ack), 0, (struct sockaddr *)&client_addr, addr_len);
                printf("Sent ACK: %c\n", ack[0]);
            }
        } else {
            // If the sender signals end of transmission
            if (packet[1] == '\0') {
                printf("End of file transmission received.\n");
                break;
            }
        }
    }

    fclose(file);
    printf("File received successfully.\n");
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr;

    srand(time(NULL));

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        perror("socket");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Receiver is running...\n");
    receive_file(sock);

    closesocket(sock);
    WSACleanup();
    return 0;
}
