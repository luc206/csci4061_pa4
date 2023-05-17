/** test machine: csel-kh4250-01.cselabs.umn.edu (ssh)
*   group number: G19
*   name: Timothy Anderson, Luc Rubin, Hank Hanson
*   x500: and09985, rubin206, hans7186 **/

#include "client.h"

float cashOnHand = START_CASH; 

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

int main(int argc, char *argv[]){
    // argument handling
    if(argc != 4)
    {
        printSyntax();
        return 0;
    }

    // command line arguments
    char* server_addr = argv[2];
    int server_port = atoi(argv[3]);

    // setup for socket
    int sockfd = -1, connfd;
    struct sockaddr_in servaddr, cli;

    // input folder and file
    char folder[MAX_STR];
    sprintf(folder, "input/%s", argv[1]);
    FILE *fp = fopen(folder,"r");
    if (fp == NULL){
      fprintf(stderr, "Client failed to open file \"%s\"\n", folder);
      return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    struct message msg; // msg structure to be populated from input
    struct message readMsg; // msg structure for responses from server

    clock_t start_time = clock(); // starting time for calculating elapsed time
    while ((read = getline(&line, &len, fp)) != -1) {
        // connect to the server if sockfd is -1
        if (sockfd == -1) { 
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                printf("Client \"%s\" socket creation failed...\n", folder);
                exit(0);
            } 
            bzero(&servaddr, sizeof(servaddr));

            servaddr.sin_family = AF_INET;
            servaddr.sin_addr.s_addr = inet_addr(server_addr);
            servaddr.sin_port = htons(server_port);

            if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) {
                printf("Client \"%s\" connection with the server failed...\n", folder);
                exit(0);
            } 
            else printf("Client \"%s\" connected to the server\n", folder);
        }
        
        // readline
        sscanf(line, "%d,%d,%[^,],%[^,],%ld,%f,%d\n", (int*)&msg.msg_type, &msg.acc_num, msg.name, msg.username, &msg.birthday, &msg.cash, &msg.num_tx);
        // process the message
        switch (msg.msg_type) { // send message        
            case REGISTER : case GET_ACCOUNT_INFO : case GET_BALANCE : case ERROR :
            // basic cases that send message as is and wait for response
                writeMessage(sockfd, msg); 
                readMessage(sockfd, NULL);
                break;
            case TRANSACT :
                readMsg = DEFAULT_MESSAGE;
                if (msg.cash < 0) { // extra checks for withdrawals
                    // get balance to check if there is enough to make the transaction
                    writeMessage(sockfd, (struct message){GET_BALANCE, 0, "\0", "\0", 0, msg.acc_num, 0.0, 0, NULL});
                    readMessage(sockfd, &readMsg);
                    if (msg.cash * -1 > readMsg.cash) break; // not enough balance, break
                    while (msg.cash * -1 > cashOnHand) { // request cash until the client has enough on hand
                        writeMessage(sockfd, (struct message){REQUEST_CASH, 0, "\0", "\0", 0, 0, CASH_AMOUNT, 0, NULL}); 
                        readMessage(sockfd, &readMsg); 
                        cashOnHand += readMsg.cash;
                    } 
                }
                cashOnHand += msg.cash;              
                writeMessage(sockfd, msg); 
                readMessage(sockfd, NULL);
                break;
            case REQUEST_CASH:
                writeMessage(sockfd, msg); 
                readMessage(sockfd, &readMsg); 
                cashOnHand += readMsg.cash;
                break;                               
            case TERMINATE :
                writeMessage(sockfd, msg);
                readMessage(sockfd, NULL);
                close(sockfd); sockfd = -1; // close socket to create new one
                break;
            case REQUEST_HISTORY :
                readMsg = DEFAULT_MESSAGE;
                writeMessage(sockfd, msg);
                readMessage(sockfd, &readMsg);
                // print out transaction history
                printf("Transaction History of Account %d:\n", msg.acc_num);
                for(int i = 0; i < readMsg.num_tx; i++)
                    printf("%.2f\n",readMsg.history[i]);
                break; 
            default : // handles invalid message types
                writeMessage(sockfd, (struct message){ERROR, msg.msg_type, "\0", "\0", 0, 0, 0.0, 0, NULL});
                readMessage(sockfd, NULL);
        }
    }

    clock_t end_time = clock(); // calculate the elapsed time
    printf("Elapsed Time: %.2f\n", ((double) (end_time-start_time))/CLOCKS_PER_SEC);
    
    // writes terminate message to the server
    writeMessage(sockfd, (struct message){TERMINATE, 0, "\0", "\0", 0, 0, 0.0, 0, NULL});
    readMessage(sockfd, NULL);
    
    close(sockfd);
    fclose(fp);
    free(line);
    
    return 0; 
}

