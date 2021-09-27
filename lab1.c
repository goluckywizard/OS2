#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>
#include <thread.h>

static const int PTHREAD_CREATE_SUCCESS = 0;

void *print(char *arg) {
        int i = 0;
        for (i = 0; i < 10; ++i)
                printf("%s%d\n", arg, i);
        return EXIT_SUCCESS;
}

static char childText[10] = "child";

int main(void) {
        pthread_t thread;
        int err = pthread_create(&thread, NULL, print, childText);
        if (err != PTHREAD_CREATE_SUCCESS) {
                fprintf(stderr, "Unable to create thread!");
                exit(EXIT_FAILURE);
        }
        print("parent");
        pthread_exit(EXIT_SUCCESS);
}
