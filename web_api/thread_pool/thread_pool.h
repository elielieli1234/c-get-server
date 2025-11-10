#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <string.h>

# define LOAD_BOUND 1280

struct io_operation {
    int host_fd;
    int client_fd;
    int kq;
};

/**
 * @brief Communicate over socket when io is available
 * @param op The io operation to be performed over the socket
 * @return 0 on successful communication, 1 on failure
 */
int communicate(struct io_operation *op);

/**
 * @brief Initialize threads in sleeped state for io operation
 * @param arg Null arguments to the thread
 */
void *thread_start(void *arg);

/**
 * @brief Submit an io operation to a thread
 * @param op The operation to be enqueued
 * @return 0 on successfull operation submission, 1 if the queue is full
 */
int submit_op(struct io_operation op);

/**
 * @brief Intiailize and return a handle to a pool of threads
 * @return The pointer to the handle of threads
 */
pthread_t *thread_pool_init(int num_threads);

/**
 * @brief Cleanup threads and locking mechanisms
 * @return 0 on successful destruction, 1 on failure
 */
int thread_pool_destroy(pthread_t *threads, int num_threads);

#endif