#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "./web_api/socket/socket.h"
#include "./web_api/thread_pool/thread_pool.h"
#include "./web_api/http/http.h"

# define THREAD_NUM 20 // Number of threads handling io ops asynchronously

struct pack_event {
    int kq; // Event queue
    int fd; // Host file descriptor
};

void *marshall_io(void *arg) {
    while (1) {
        struct pack_event *pack = (struct pack_event *)arg;
        int kq = pack->kq;
        int fd = pack->fd;

        struct kevent event;
        int nev = kevent(kq, NULL, 0, &event, 1, NULL);
        if (event.filter == EVFILT_READ) { // Readable client socket
            if (event.flags & EV_EOF) {
                printf("\nClient %lu disconnected\n", event.ident);
                close(event.ident);  // Clean up
            } else {
                printf("\nClient %lu is readable\n", event.ident);
            
                // Queue the operation for the thread pool
                struct io_operation op;
                op.host_fd = fd;
                op.client_fd = event.ident;
                op.kq = kq; // Put the file descriptor back on kernel queue

                int request_handled = submit_op(op);
                if (request_handled == -1) { // The thread pool is busy
                    
                    // busy_fd = fopen("./www/503.html", "r");
                    // if (busy_fd == -1) 
                    //     continue; // Ignore the request
                    
                    // char packet_buffer[PACK_LIM] = {'\0'};

                    // int head_size = snprintf(packet_buffer, PACK_LIM - 1, "%s", packet_buffer);
                }
            }
        }     
    }

    return NULL;
}

int main() {
    char *host = "localhost";
    char *port = "3000";

    // Get listening host
    int log_host = 1;
    int sockfd = host_socket_init(log_host, host, port);
    if (sockfd == -1) {
        fprintf(stderr, "[ERROR]: %s", strerror(errno));
        return 1;
    }

    // Get listening client using kernel events
    int kq = kqueue();
    if (kq == -1) {
        perror("kqueue");
        return 1;
    }

    // Initialize a thread pool
    pthread_t *thread_pool = thread_pool_init(THREAD_NUM);

    // Queue events in thread pool
    pthread_t marshall_thread;
    struct pack_event event_listener = { .kq = kq , .fd = sockfd };
    pthread_create(&marshall_thread, NULL, marshall_io, (void *) &event_listener);
    pthread_detach(marshall_thread);

    // Accept arbitrary connections
    int session = 0;
    while (1) {
        int log_client = 1;
        int client_fd = client_socket_init(log_client, sockfd);
        if (client_fd == -1) {
            fprintf(stderr, "[ERROR]: %s", strerror(errno));
            return 1;
        }

        // Register the client socket for read & write events w/o blocking
        struct kevent change;
        EV_SET(&change, client_fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, NULL);
        
        // Register listener w/out blocking
        struct timespec non_block = { .tv_sec = 0, .tv_nsec = 0 };
        const struct timespec *nb_param = &non_block;
        if (kevent(kq, &change, 1, NULL, 0, nb_param) == -1) {
            perror("kevent register listen socket");
            return 1;
        }

        session++;
    }

    return 0;
}
