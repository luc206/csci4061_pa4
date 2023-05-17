/** test machine: csel-kh4250-01.cselabs.umn.edu (ssh)
*   group number: G19
*   name: Timothy Anderson, Luc Rubin, Hank Hanson
*   x500: and09985, rubin206, hans7186 **/

#include "server.h"

#define NCLIENTS 16

struct account accounts[MAX_ACC]; // global array of accounts
int account_num = 0; // number of accounts currently registered
pthread_mutex_t account_num_lock;
pthread_mutex_t socket_lock;

struct th* insert(struct th* head, float amount) {
    struct th *prev, *new;

    // iterates through until it finds the end
    for (new = head, prev = NULL; new; prev = new, new = new->next);

    new = (struct th*)malloc(sizeof(struct th));
    new->amount = amount;
    new->next = NULL;
    
    if (prev) // set the previous node to point towards the new one
        prev->next = new;
    else // if there is no previous node, the new node is now the head
        head = new;
    
    return head;
}

int list_size(struct th * n) {
    int i;
    for(i = 0; n; i++) n = n->next;
    return i;
}

void* worker(void *arg) {
    struct arg_struct *args = (struct arg_struct*)arg;
    while(1){
        pthread_mutex_lock(&socket_lock);
        // accept the connection using the socket passed to the worker thread
        int connfd = accept(args->sockfd, args->cli, args->len);
        if (connfd < 0) {
            printf("Server accept failed...\n");
            exit(0);
        } 
        else printf("Server accepted a client on worker thread %d\n", args->thread_id);
        pthread_mutex_unlock(&socket_lock);

        struct message m; // msg structure to store the data recieved from the client
        m.msg_type = 0; // initial value set
        while(m.msg_type != TERMINATE){
            readMessage(connfd, &m); // wait for and read message from the client
            // process the message
            switch (m.msg_type) {
                case REGISTER :
                    pthread_mutex_lock(&account_num_lock); // locks account_num varible
                    pthread_mutex_lock(&accounts[account_num].mutex); // lock for a specific account
                    strcpy(accounts[account_num].name, m.name);
                    strcpy(accounts[account_num].username, m.username);
                    accounts[account_num].birthday = m.birthday;
                    accounts[account_num].balance = 0.0;
                    accounts[account_num].history = NULL;
                    pthread_mutex_unlock(&accounts[account_num].mutex);
                    ++account_num; // increment the total amount of accounts
                    pthread_mutex_unlock(&account_num_lock);
                    
                    m.msg_type = BALANCE;
                    m.cash = 0.0;
                    writeMessage(connfd, m);
                    break;
                case GET_ACCOUNT_INFO :
                    pthread_mutex_lock(&accounts[m.acc_num].mutex);
                    strcpy(m.name, accounts[m.acc_num].name);
                    strcpy(m.username, accounts[m.acc_num].username);
                    m.birthday = accounts[m.acc_num].birthday;
                    pthread_mutex_unlock(&accounts[m.acc_num].mutex);
                    
                    m.msg_type = ACCOUNT_INFO;
                    writeMessage(connfd, m);
                    break;
                case TRANSACT :
                    pthread_mutex_lock(&accounts[m.acc_num].mutex);
                    accounts[m.acc_num].balance += m.cash;
                    accounts[m.acc_num].history = insert(accounts[m.acc_num].history, m.cash);
                    m.cash = accounts[m.acc_num].balance;
                    pthread_mutex_unlock(&accounts[m.acc_num].mutex);
                    
                    m.msg_type = BALANCE;
                    writeMessage(connfd, m);
                    break;
                case GET_BALANCE :
                    pthread_mutex_lock(&accounts[m.acc_num].mutex);
                    m.cash = accounts[m.acc_num].balance;
                    pthread_mutex_unlock(&accounts[m.acc_num].mutex);
                    
                    m.msg_type = BALANCE;
                    writeMessage(connfd, m);
                    break;       
                case REQUEST_CASH :
                    m.msg_type = CASH;
                    writeMessage(connfd, m);
                    break;
                case ERROR :
                    printf("Server recieved error message from client: %d\n", m.err_type); 
                    writeMessage(connfd, m);
                    break;           
                case TERMINATE :
                    writeMessage(connfd, m); // echoes the response back
                    break;
                case REQUEST_HISTORY :
                    pthread_mutex_lock(&accounts[m.acc_num].mutex);
                    struct th *n = accounts[m.acc_num].history;
                    if (m.num_tx == 0) m.num_tx = list_size(n); // set the number of transations to the list size when 0 is recieved
                    m.history = (float*)malloc(sizeof(float) * m.num_tx);
                    for(int i = 0; n && i < m.num_tx; i++) {
                        m.history[i] = n->amount;
                        n = n->next;
                    }
                    pthread_mutex_unlock(&accounts[m.acc_num].mutex);
                    
                    m.msg_type = HISTORY;
                    writeMessage(connfd, m);
                    break; 
                default :
                    printf("Server recieved an unexpected message from the client: %d\n",m.msg_type);
                    m.msg_type = ERROR;
                    writeMessage(connfd, m);
            }
        }
        close(connfd);
    }

    return NULL;
}

void* logging(void *arg) {
    printf("Server logging thread started\n");
    while(1) {
        sleep(LOGGER_SLEEP);
        FILE *fp = fopen("output/balances.csv", "w");
        if(fp == NULL) {
            fprintf(stderr, "Error opening log file in server.c\n");
            return NULL;
        }
        
        pthread_mutex_lock(&account_num_lock);
        printf("Server logging %d accounts... ", account_num+1);
        for(int i = 0; i < account_num; i++) {
            pthread_mutex_lock(&accounts[i].mutex);
            
            char file_name[MAX_STR];
            sprintf(file_name, "output/account%d.csv", i);
            FILE *history_fp = fopen(file_name, "w");
            if(history_fp == NULL) {
                fprintf(stderr, "Error opening account log file \"%s\" in server.c\n", file_name);
                return NULL;
            }
            struct th *n = accounts[i].history;
            while(n->next) {
                fprintf(history_fp, "%.2f\n", n->amount);
                n = n-> next;
            }
            fclose(history_fp);

            fprintf(fp, "%d,%.2f,%s,%s,%ld\n", i, accounts[i].balance, accounts[i].name, accounts[i].username, accounts[i].birthday);
            pthread_mutex_unlock(&accounts[i].mutex);
        }
        pthread_mutex_unlock(&account_num_lock);
        printf("\n");
        
        fclose(fp);
    }
}

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./server server_addr server_port num_workers\n");
}

int main(int argc, char *argv[]){
    // argument handling
    if(argc != 4)
    {
        printSyntax();
        return 0;
    }

    char* server_addr = argv[1];
    int server_port = atoi(argv[2]);
    int num_workers = atoi(argv[3]);

    // create empty output folder
    bookeepingCode();

    // socket setup
    int sockfd, len;
    struct sockaddr_in servaddr, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Server socket creation failed...\n");
        exit(0);
    } 
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_addr);
    servaddr.sin_port = htons(server_port);

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) {
        printf("Server socket bind failed...\n");
        exit(0);
    }
    
    if(listen(sockfd, NCLIENTS)) {
        printf("Server listen failed...\n");
        exit(0);
    } 
    else printf("Server listening..\n");
    len = sizeof(cli);

    // init mutex locks
    for(int i = 0; i < MAX_ACC; i++) pthread_mutex_init(&accounts[i].mutex, NULL);
    pthread_mutex_init(&account_num_lock, NULL);
    pthread_mutex_init(&socket_lock, NULL);
    
    // start log thread
    pthread_t log_tid;
    pthread_create(&log_tid, NULL, logging, NULL);

    // loop and start accepting connections
    pthread_t tid;
    struct arg_struct worker_arr[num_workers];
    for(int i = 0; i < num_workers; i++) {
        worker_arr[i].thread_id = i;
        worker_arr[i].sockfd = sockfd;
        worker_arr[i].cli = (struct sockaddr *) &cli;
        worker_arr[i].len = &len;
        pthread_create(&tid, NULL, worker, (void*)&worker_arr[i]);
        pthread_detach(tid);
    }

    pthread_join(log_tid, NULL); // log should run forever so that main never exits

    close(sockfd);
    return 0; 
}