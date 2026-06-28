#include "./thread_pool.h"
#include "../http/http.h" // Use request parser to build thread response

// Amorticize enqueueing and dequeuing
pthread_mutex_t mutex_queue;
pthread_cond_t cond_queue; // Signal to workers that io is available

// Queue of io operations
struct io_operation operations[LOAD_BOUND];
int num_operations = 0; // Number of queued operations

int communicate(struct io_operation *op) {
    int host_fd = op->host_fd;
    int client_fd = op->client_fd;
    int kq = op->kq;

    printf("    Reading from client socket...\n");

    char buffer[PACK_LIM] = {'\0'};
    ssize_t bytes = recv(client_fd, buffer, PACK_LIM - 1, 0);

    // Print contents to server debug
    printf("---- Got ----\n");
    printf("\n%s\n", buffer);
    printf("-------------\n");

    if (bytes > 0) {
        int packet_length = 0;
        struct packet_node *packets = response_list(buffer);
        if (packets != NULL) { // Send the full response in chunk
            while (packets != NULL) {
                send(client_fd, packets->packet, packets->length, 0);
                
                // Move onto the next packet
                struct packet_node *next = packets->next;
                free(packets->packet);
                free(packets);
                packets = next;
            }
        }
        else printf("\nFATAL ERROR GENERATING HOST RESPONSE.\n");
    }

    return 0;
}

void *thread_start(void *arg) {
    while (1) {
        struct io_operation op;

        pthread_mutex_lock(&mutex_queue);
        while (num_operations == 0) {
            pthread_cond_wait(&cond_queue, &mutex_queue);
        }

        op = operations[0];
        for (int i = 0; i < num_operations - 1; i++) {
            operations[i] = operations[i + 1];
        }

        num_operations--;
        
        pthread_mutex_unlock(&mutex_queue);
    
        // Communicate over the socket
        communicate(&op);
    }

    return NULL;
}

int submit_op(struct io_operation op) {
    pthread_mutex_lock(&mutex_queue);
    int busy = 0; // Indicate whether the server is busy

    // Don't add an operation if the server is busy
    if (num_operations < LOAD_BOUND - 1) {
        operations[num_operations] = op;
        num_operations++;
    } else busy = 1;
    
    pthread_mutex_unlock(&mutex_queue);
    pthread_cond_signal(&cond_queue);

    // If we're busy, return -1 (abide by socket interface)
    return busy == 1 ? -1 : 0;
}

pthread_t *thread_pool_init(int num_threads) {
    pthread_mutex_init(&mutex_queue, NULL);
    pthread_cond_init(&cond_queue, NULL);

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_start, NULL) != 0) {
            return NULL; 
        }
    }

    return threads;
}

int thread_pool_destroy(pthread_t *threads, int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL)) {
            return -1;
        }
    }

    free(threads);
    pthread_mutex_destroy(&mutex_queue);
    pthread_cond_destroy(&cond_queue);

    return 0;
}