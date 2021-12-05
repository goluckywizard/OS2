#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#define COUNT_SEMAPHORES 2
#define NOT_PROCESS_SHARED 0
#define PTHREAD_SUCCESS 0
#define COUNT_LINES 10
#define TEXT_SIZE 10
#define FIRST_THREAD 0
#define SECOND_THREAD 1

typedef struct st_Args {
    char *text;
    sem_t *semaphores;
    int thread_number;
} Args;

int semaphore_initialization(sem_t *semaphores) {
    for (int i = 0; i < COUNT_SEMAPHORES; ++i) {
        int error = sem_init(&semaphores[i], NOT_PROCESS_SHARED, i);
        if (error != PTHREAD_SUCCESS) {
            return error;
        }
    }
    return PTHREAD_SUCCESS;
}
int semaphore_destroy(sem_t *semaphores) {
    for (int i = 0; i < COUNT_SEMAPHORES; ++i) {
        int error = sem_destroy(&semaphores[i]);
        if (error != PTHREAD_SUCCESS) {
            return error;
        }
    }
    return PTHREAD_SUCCESS;
}

void *print(void *args) {
    Args *arg = (Args *)args;
    int cur_sem = arg->thread_number;
    int next_sem = (cur_sem + 1) % COUNT_SEMAPHORES;
    int error;
    for (int i = 0; i < COUNT_LINES; ++i) {
        error = sem_wait(&arg->semaphores[next_sem]);
        if (error != PTHREAD_SUCCESS) {
            perror("Unable to wait semaphore");
            return (void *)EXIT_FAILURE;
        }

        printf("%s %d\n", arg->text, i);
        error = sem_post(&arg->semaphores[cur_sem]);
        if (error != PTHREAD_SUCCESS) {
            perror("Unable to post semaphore");
            return (void *)EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int main() {
    sem_t semaphores[COUNT_SEMAPHORES];
    int error;
    error = semaphore_initialization(semaphores);
    if (error != PTHREAD_SUCCESS) {
        perror("Unable to initialize semaphores");
    }
    char parent_text[TEXT_SIZE] = "Parent";
    char child_text[TEXT_SIZE] = "Child";

    Args parent_args, child_args;
    parent_args.semaphores = semaphores;
    child_args.semaphores = semaphores;
    parent_args.thread_number = FIRST_THREAD;
    child_args.thread_number = SECOND_THREAD;
    parent_args.text = parent_text;
    child_args.text = child_text;

    pthread_t thread;
    errno = pthread_create(&thread, NULL, print, &child_args);
    if (errno != PTHREAD_SUCCESS) {
        perror("Unable to create thread");
        semaphore_destroy(semaphores);
    }
    error = (int)print(&parent_args);
    int thread_func_error;

    errno = pthread_join(thread, (void **)&thread_func_error);
    if (errno != PTHREAD_SUCCESS) {
        perror("Unable to join thread");
        semaphore_destroy(semaphores);
    }

    if (error != EXIT_SUCCESS || thread_func_error != EXIT_SUCCESS) {
        fprintf(stderr, "Error in print function");
    }
    error = semaphore_destroy(semaphores);
    if (error != PTHREAD_SUCCESS) {
        perror("Unable to destroy semaphores");
    }
    return 0;
}
