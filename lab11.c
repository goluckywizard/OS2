#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#define PTHREAD_MUTEX_INIT_SUCCESS 0
#define PTHREAD_MUTEX_DESTROY_SUCCESS 0
#define MUTEX_COUNT 3
#define FIRST_THREAD 1
#define SECOND_THREAD 2

int init_mutexes(pthread_mutex_t *mutexes) {
    for (int i = 0; i < MUTEX_COUNT; ++i) {
        int error = pthread_mutex_init(&mutexes[i], NULL);
        if (error != PTHREAD_MUTEX_INIT_SUCCESS) {
            return error;
        }
    }
    return PTHREAD_MUTEX_INIT_SUCCESS;
}
void print(pthread_mutex_t *mutexes, int cur_thread) {
    pthread_mutex_lock(mutexes[cur_thread]);
    printf("%d\n", cur_thread);

}

int destroy_mutexes(pthread_mutex_t *mutexes) {
    for (int i = 0; i < MUTEX_COUNT; ++i) {
        int error = pthread_mutex_destroy(&mutexes[i]);
        if (error != PTHREAD_MUTEX_DESTROY_SUCCESS) {
            return error;
        }
    }
    return PTHREAD_MUTEX_DESTROY_SUCCESS;
}

int main(void) {
    pthread_mutex_t mutexes[MUTEX_COUNT];
    int init_error = init_mutexes(mutexes);
    if (init_error != PTHREAD_MUTEX_INIT_SUCCESS) {
        errno = init_error;
        perror("Mutexes initialization error");
        return EXIT_FAILURE;
    }
    int destroy_error = destroy_mutexes(mutexes);
    if (destroy_error != PTHREAD_MUTEX_DESTROY_SUCCESS) {
        errno = destroy_error;
        perror("Mutexes destroy error");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
