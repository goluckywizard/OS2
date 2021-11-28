#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define PTHREAD_SUCCESS 0
#define MUTEX_COUNT 3
#define FIRST_THREAD 0
#define SECOND_THREAD 1

typedef struct st_Args {
    pthread_mutex_t *mutexes;
    char *text;
    int cur_thread;
} Args;

int init_mutexes(pthread_mutex_t *mutexes) {
    pthread_mutexattr_t mutex_attr;
    int error;
    error = pthread_mutexattr_init(&mutex_attr);
    if (error != PTHREAD_SUCCESS) {
        return error;
    }
    error = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    if (error != PTHREAD_SUCCESS) {
        return error;
    }
    for (int i = 0; i < MUTEX_COUNT; ++i) {
        error = pthread_mutex_init(&(mutexes[i]), &mutex_attr);
        if (error != PTHREAD_SUCCESS) {
            return error;
        }
    }
    return PTHREAD_SUCCESS;
}
void *print(void *argument) {
    Args *arg = (Args *)argument;
    if (arg->cur_thread == SECOND_THREAD) {
        //printf("second thread mutex: %p\n", &(arg->mutexes[(arg->cur_thread) % 3]));
        errno = pthread_mutex_lock(&(arg->mutexes[(arg->cur_thread) % 3]));
        if (errno != PTHREAD_SUCCESS) {
            perror("Mutex lock error");
            return;
        }
    }

    for (int i = 0; i < 10; ++i) {
        errno = pthread_mutex_lock(&(arg->mutexes[(arg->cur_thread + 2) % 3]));
        if (errno != PTHREAD_SUCCESS) {
            perror("Mutex lock error");
            return;
        }

        printf("%s %d\n", arg->text, i);

        errno = pthread_mutex_unlock(&(arg->mutexes[arg->cur_thread]));
        if (errno != PTHREAD_SUCCESS) {
            perror("Mutex unlock error");
            return;
        }

        errno = pthread_mutex_lock(&(arg->mutexes[(arg->cur_thread + 1) % 3]));
        if (errno != PTHREAD_SUCCESS) {
            perror("Mutex lock error");
            return;
        }

        errno = pthread_mutex_unlock(&(arg->mutexes[(arg->cur_thread + 2) % 3]));
        if (errno != PTHREAD_SUCCESS) {
            perror("Mutex unlock error");
            return;
        }

        errno = pthread_mutex_lock(&(arg->mutexes[arg->cur_thread]));
        if (errno != PTHREAD_SUCCESS) {
            perror("Mutex lock error");
            return;
        }

        errno = pthread_mutex_unlock(&(arg->mutexes[(arg->cur_thread + 1) % 3]));
        if (errno != PTHREAD_SUCCESS) {
            perror("Mutex unlock error");
            return;
        }
    }

    if (arg->cur_thread == SECOND_THREAD) {
        errno = pthread_mutex_unlock(&(arg->mutexes[(arg->cur_thread) % 3]));
        if (errno != PTHREAD_SUCCESS) {
            perror("Mutex unlock error");
            return;
        }
    }
    pthread_exit(EXIT_SUCCESS);
}

int destroy_mutexes(pthread_mutex_t *mutexes) {
    for (int i = 0; i < MUTEX_COUNT; ++i) {
        int error = pthread_mutex_destroy(&mutexes[i]);
        if (error != PTHREAD_SUCCESS) {
            return error;
        }
    }
    return PTHREAD_SUCCESS;
}

int main(void) {
    pthread_mutex_t mutexes[MUTEX_COUNT];
    errno = init_mutexes(mutexes);
    if (errno != PTHREAD_SUCCESS) {
        perror("Mutexes initialization error");
        return EXIT_FAILURE;
    }

    char child[10] = "Child";
    char parent[10] = "Parent";
    Args arg1, arg2;
    arg1.cur_thread = FIRST_THREAD;
    arg1.mutexes = mutexes;
    arg1.text = parent;
    arg2.cur_thread = SECOND_THREAD;
    arg2.mutexes = mutexes;
    arg2.text = child;

    pthread_t thread;
    errno = pthread_mutex_lock(&(arg1.mutexes[(FIRST_THREAD) % 3]));
    if (errno != PTHREAD_SUCCESS) {
        perror("Lock error");
        return EXIT_FAILURE;
    }

    pthread_create(&thread, NULL, print, &arg2);
    sleep(1);
    print(&arg1);
    errno = pthread_mutex_unlock(&(arg1.mutexes[(FIRST_THREAD) % 3]));
    if (errno != PTHREAD_SUCCESS) {
        perror("Unlock error");
        destroy_mutexes(mutexes);
        return EXIT_FAILURE;
    }
    pthread_join(thread, NULL);

    errno = destroy_mutexes(mutexes);
    if (errno != PTHREAD_SUCCESS) {
        perror("Mutexes destroy error");
        destroy_mutexes(mutexes);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
