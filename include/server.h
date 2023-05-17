#ifndef SERVER_H
#define SERVER_H

#include "utils.h"

struct arg_struct { // arguments to be passed into worker thread
    int thread_id;
    int sockfd;
    struct sockaddr* cli;
    int* len;
};

struct th { // transaction history linked list
    float amount;
    struct th* next;
};

struct account // stores account information
{
    char name[MAX_STR];
    char username[MAX_STR];
    time_t birthday;
    float balance;
    struct th *history;
    pthread_mutex_t mutex;
};


struct th* insert(struct th* head, float amount); // inserts a new transaction into the transaction history
int list_size(struct th * n); // returns the size of the transaction history linked list

void* worker(void *arg); // worker threads to accept connections from clients
void* logging(void *arg); // logging thread to generate the output files
void printSyntax();

#endif

