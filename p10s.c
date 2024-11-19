#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

#define PORT 12345
#define PACKET_SIZE 2 // 1 byte for character + 1 byte for sequence number
#define ACK_TIMEOUT 2 // seconds
#define NOISE_PROBABILITY 0.1 // Probability of packet loss

int simulate_noise() {
    return (rand() / (double)RAND_MAX) < NOISE_PROBABILITY;
}

void send_file(SOCKET sock, struct sockaddr_in *addr, const char *data) {
    int sequence_number = 0;
    char packet[PACKET_SIZE];
    struct timeval timeout;
    fd_set read_fds;
    int ack_received;

    for (int i = 0; i < strlen(data); ++i) {
        ack_received = 0;

        while (!ack_received) {
            // Prepare the packet: 1 byte for sequence number and 1 byte for the character
            packet[0] = sequence_number; // Sequence number (0 or 1)
            packet[1] = data[i];         // Character to send

            // Simulate packet loss
            if (!simulate_noise()) {
                sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)addr, sizeof(*addr));
                printf("Sent: %c (Sequence Number: %d)\n", data[i], sequence_number);
            } else {
                printf("Simulating packet loss for character %c\n", data[i]);
            }

            // Set up a timeout for receiving the ACK
            timeout.tv_sec = ACK_TIMEOUT;
            timeout.tv_usec = 0;

            // Wait for ACK
            while (1) {
                FD_ZERO(&read_fds);
                FD_SET(sock, &read_fds);
                int select_result = select(0, &read_fds, NULL, NULL, &timeout); // use 0 instead of sock + 1 in Windows

                if (select_result == SOCKET_ERROR) {
                    perror("select");
                    exit(EXIT_FAILURE);
                } else if (select_result == 0) {
                    // Timeout occurred, resend packet
                    printf("ACK timeout, resending packet...\n");
                    break;
                } else if (FD_ISSET(sock, &read_fds)) {
                    // ACK received
                    char ack[10];
                    recvfrom(sock, ack, sizeof(ack), 0, NULL, NULL);
                    ack_received = (ack[0] == sequence_number + '0');
                    if (ack_received) {
                        printf("Received ACK: %c\n", ack[0]);
                        sequence_number = 1 - sequence_number; // Toggle between 0 and 1
                        break;
                    }
                }
            }
        }
    }

    // Send end-of-transmission signal
    packet[0] = sequence_number;
    packet[1] = '\0'; // Indicate end of transmission with null character
    sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)addr, sizeof(*addr));
    printf("Sent end-of-transmission signal\n");
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr;

    srand(time(NULL));

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        perror("socket");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    char data[] = "Hello, World!";
    send_file(sock, &server_addr, data);

    // Clean up
    closesocket(sock);
    WSACleanup();

    return 0;
}


// .receive.exe - ./send.exe