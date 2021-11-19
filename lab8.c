#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>

#define NOT_DIGIT 0
#define CORRECT_ARGS_COUNT 3
#define DECIMAL_SYSTEM 10
#define BUF_SIZE 1024
#define MIN_THREADS_COUNT 1
#define MAX_THREADS_COUNT 100000
#define MIN_ITERATION_COUNT 1

typedef enum en_parseInputArgsError {
    parse_SUCCESS,
    parse_INVALID_ARGS_COUNT,
    parse_NOT_POSITIVE_NUMBER,
    parse_ERANGE,
    parse_INVALID_NUM_THREADS,
    parse_INVALID_NUM_ITERATIONS,
    parse_ARGS_DONT_MATCH
} parseInputArgsError;

typedef enum en_threadError {
    SUCCESS = 0,
    THREAD_CREATE_ERROR,
    THREAD_JOIN_ERROR
} threadError;

typedef struct st_inputArgs {
    int numThreads;
    int numIterations;
} inputArgs;

typedef struct st_threadFuncArg {
    int iterCount;
    int shift;
    double partialSum;
} threadFuncArg;

int parseInputData(int argc, char **argv, inputArgs *argsValues) {
    if (argc != CORRECT_ARGS_COUNT)
        return parse_INVALID_ARGS_COUNT;
    int i = 0;
    do {
    if (isdigit(argv[1][i]) == NOT_DIGIT) {
            return parse_NOT_POSITIVE_NUMBER;
        }
        i++;
    } while (argv[1][i] != '\0');
    i = 0;
    do {
        if (isdigit(argv[2][i]) == NOT_DIGIT) {
            //printf("%c", argv[2][i]);
            return parse_NOT_POSITIVE_NUMBER;
        }
        i++;
    } while (argv[2][i] != '\0');
    long threads_count = strtol(argv[1], NULL, DECIMAL_SYSTEM);
    if (errno == ERANGE) {
        return parse_ERANGE;
    }
    if (threads_count < MIN_THREADS_COUNT || threads_count > MAX_THREADS_COUNT) {
        return parse_INVALID_NUM_THREADS;
    }
    argsValues->numThreads = (int)threads_count;

    long iter_count = strtol(argv[2], NULL, DECIMAL_SYSTEM);
    if (errno == ERANGE) {
        return parse_ERANGE;
    }
    if (iter_count < MIN_ITERATION_COUNT || iter_count >= INT_MAX) {
        return parse_INVALID_NUM_ITERATIONS;
    }
    argsValues->numIterations = (int)iter_count;

    if (threads_count > iter_count) {
        return parse_ARGS_DONT_MATCH;
    }
    return parse_SUCCESS;
}
void print_parse_error(int error_code) {
    switch (error_code) {
        case parse_INVALID_ARGS_COUNT: {
            fprintf(stderr, "Invalid arguments count\n");
            break;
        }
        case parse_NOT_POSITIVE_NUMBER: {
            fprintf(stderr, "Arguments should be positive numbers!\n");
            break;
        }
        case parse_ERANGE: {
            fprintf(stderr, "One or more arguments are out of range\n");
            break;
        }
        case parse_INVALID_NUM_THREADS: {
            fprintf(stderr, "Invalid threads count\n");
            break;
        }
        case parse_INVALID_NUM_ITERATIONS: {
            fprintf(stderr, "Invalid iterations count\n");
            break;
        }
        case parse_ARGS_DONT_MATCH: {
            fprintf(stderr, "Invalid value of arguments\n");
            break;
        }
        default: {
            fprintf(stderr, "Unknown parse error\n");
            break;
        }
    }
}
void create_thread_args(threadFuncArg *thread_args, int threads_count, int iterations_count) {
    int iterations_per_thread = iterations_count / threads_count;
    int extra_iterations = iterations_count % threads_count;
    thread_args[0].shift = 0;
    thread_args[0].iterCount = iterations_per_thread;
    if (extra_iterations != 0) {
        thread_args[0].iterCount++;
    }

    for (int i = 1; i < threads_count; ++i) {
        thread_args[i].shift = thread_args[i - 1].shift + thread_args[i - 1].iterCount;
        thread_args[i].iterCount = iterations_per_thread;
        if (i < extra_iterations) {
            thread_args[i].iterCount++;
        }
    }
}

void *calculatePartialSum(void *thread_arg) {
    threadFuncArg *arg = (threadFuncArg *)thread_arg;
    arg->partialSum = 0;
    for (int i = arg->shift; i < arg->shift + arg->iterCount; ++i) {
        if (i % 2 == 0) {
            arg->partialSum += 1.0 / (i * 2.0 + 1.0);
        } else {
            arg->partialSum -= 1.0 / (i * 2.0 + 1.0);
        }
    }
    pthread_exit(&(arg->partialSum));
}
int collectPartialSums(pthread_t *threadsID, int threads_count, double *result) {
    *result = 0;
    for (int i = 0; i < threads_count; ++i) {
        void *partialSum;
        int error = pthread_join(threadsID[i], &partialSum);
        if (error != SUCCESS) {
            return THREAD_JOIN_ERROR;
        }
        *result += *((double *)partialSum);
    }
    return SUCCESS;
}
int create_threads(int threads_count, void *(*thread_task)(void*), threadFuncArg *args, pthread_t *threadsID) {
    int error_num;
    for (int i = 0; i < threads_count; ++i) {
        error_num = pthread_create(&threadsID[i], NULL, thread_task, &args[i]);
        if (error_num != SUCCESS) {
            return THREAD_CREATE_ERROR;
        }
    }
    return SUCCESS;
}
int calculatePI(int threads_count, int iterations_count, double *result) {
    threadFuncArg threadsArgs[threads_count];
    create_thread_args(threadsArgs, threads_count, iterations_count);

    pthread_t threadsID[threads_count];
    int error = create_threads(threads_count, calculatePartialSum, threadsArgs, threadsID);
    if (error != SUCCESS) {
        return error;
    }
    error = collectPartialSums(threadsID, threads_count, result);
    if (error != SUCCESS) {
        return error;
    }
    *result *= 4;
    return SUCCESS;
}


int main(int argc, char *argv[]) {
    inputArgs args;
    int parse_result = parseInputData(argc, argv, &args);
    if (parse_result != parse_SUCCESS) {
        print_parse_error(parse_result);
        return EXIT_FAILURE;
    }
    //printf("%d %d\n", args.numThreads, args.numIterations);
    double pi;
    int error = calculatePI(args.numThreads, args.numIterations, &pi);
    if (error != SUCCESS) {
        char str[BUF_SIZE];
        strerror_r(error, str, BUF_SIZE);
        fprintf(stderr, "%s\n", str);
        return EXIT_FAILURE;
    }
    printf("pi = %.15lf\n", pi);
    return EXIT_SUCCESS;
}
