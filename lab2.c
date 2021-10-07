  GNU nano 4.7                                                               main.c
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>
#include <thread.h>
#include <string.h>

static const int PTHREAD_CREATE_SUCCESS = 0;
static const int PTHREAD_JOIN_SUCCESS = 0;
void *print(char *arg) {
        int i = 0;
        for (i = 0; i < 10; ++i)
                printf("%s%d\n", arg, i);
        free(arg);
        return EXIT_SUCCESS;
}

int main(void) {
        const int LENGTH_OF_ARG = 10;
        pthread_t thread;
        char *childText = (char*)malloc(sizeof(char)*LENGTH_OF_ARG);
        char *parentText = (char*)malloc(sizeof(char)*LENGTH_OF_ARG);
        strcpy(childText, "child");
        strcpy(parentText, "parent");
        int err = pthread_create(&thread, NULL, print, childText);
        if (err != PTHREAD_CREATE_SUCCESS) {
                perror("Unable to create thread");
                free(childText);
                exit(EXIT_FAILURE);
        }
        err = pthread_join(thread, NULL);
        if (err != PTHREAD_JOIN_SUCCESS) {
                perror("Unable to join thread");
                free(parentText);
                exit(EXIT_FAILURE);
        }
        print(parentText);
        return EXIT_SUCCESS;
}
