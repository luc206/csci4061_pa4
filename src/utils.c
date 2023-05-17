/** test machine: csel-kh4250-01.cselabs.umn.edu (ssh)
*   group number: G19
*   name: Timothy Anderson, Luc Rubin, Hank Hanson
*   x500: and09985, rubin206, hans7186 **/

#include "utils.h"

void _removeOutputDir()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        char *argv[] = {"rm", "-rf", "output", NULL};
        if (execvp(*argv, argv) < 0)
        {
            printf("ERROR: exec failed\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else{
        wait(NULL);
    }
}

void _createOutputDir()
{
    mkdir("output", ACCESSPERMS);
}

void bookeepingCode()
{
    _removeOutputDir();
    sleep(1);
    _createOutputDir();
}

void writeMessage(int sockfd, struct message m) {
    int net_write;
    
    net_write = htonl(m.msg_type);
    write(sockfd, &net_write, sizeof(int)); // write message type

    switch (m.msg_type) {
        case REGISTER : case ACCOUNT_INFO :
            write(sockfd, &m.username, MAX_STR);
            write(sockfd, &m.name, MAX_STR);
            write(sockfd, &m.birthday, sizeof(time_t));
            break;        
        case GET_ACCOUNT_INFO : case GET_BALANCE :
            write(sockfd, &m.acc_num, sizeof(int));
            break;
        case TRANSACT : case BALANCE :
            write(sockfd, &m.acc_num, sizeof(int));
            write(sockfd, &m.cash, sizeof(float));
            break;             
        case REQUEST_CASH : case CASH :
            write(sockfd, &m.cash, sizeof(float));
            break;    
        case ERROR :
            write(sockfd, &m.err_type, sizeof(int));
            break;           
        case TERMINATE :
            break;
        case REQUEST_HISTORY :
            write(sockfd, &m.acc_num, sizeof(int));
            write(sockfd, &m.num_tx, sizeof(int));
            break; 
        case HISTORY :
            write(sockfd, &m.acc_num, sizeof(int));
            write(sockfd, &m.num_tx, sizeof(int));
            for(int i = 0; i < m.num_tx; i++) {
                write(sockfd, &(m.history[i]), sizeof(float));
            }
            break;
    }

    return;
}
bool readMessage(int sockfd, struct message * m) {
    int net_read = 0;
    bool given_null = m == NULL; // if NULL is passed through it should still read the data to get it out of the socket
    if (given_null) m = (struct message*)malloc(sizeof(struct message)); // creates a new message if the m pointer is NULL
    
    read(sockfd, &net_read, sizeof(int));
    m->msg_type = ntohl(net_read); // read message type

    switch (m->msg_type) { // read data expected from client
        case REGISTER : case ACCOUNT_INFO :
            read(sockfd, &(m->username), MAX_STR);
            read(sockfd, &(m->name), MAX_STR);
            read(sockfd, &(m->birthday), sizeof(time_t));
            break;        
        case GET_ACCOUNT_INFO : case GET_BALANCE :
            read(sockfd, &(m->acc_num), sizeof(int));
            break;
        case TRANSACT : case BALANCE :
            read(sockfd, &(m->acc_num), sizeof(int));
            read(sockfd, &(m->cash), sizeof(float));
            break;             
        case REQUEST_CASH : case CASH :
            read(sockfd, &(m->cash), sizeof(float));
            break;    
        case ERROR :
            read(sockfd, &(m->err_type), sizeof(int));
            break;           
        case TERMINATE :
            break;
        case REQUEST_HISTORY :
            read(sockfd, &(m->acc_num), sizeof(int));
            read(sockfd, &(m->num_tx), sizeof(int));
            break; 
        case HISTORY :
            read(sockfd, &(m->acc_num), sizeof(int));
            read(sockfd, &(m->num_tx), sizeof(int));
            m->history = (float*)malloc(sizeof(float) * m->num_tx);
            for(int i = 0; i < m->num_tx; i++) {
                read(sockfd, &(m->history[i]), sizeof(float));
            }
            break;
    }

    if (given_null) free(m); // frees up the memory and voids the data read in

    return true;
}

void printMessage(struct message m) { // displays the message structure (for debugging)
    printf("{%d,%d,\"%s\",\"%s\",%ld,%d,%lf,%d,%s}", 
        m.msg_type, m.err_type, m.name, m.username, m.birthday, m.acc_num, m.cash, m.num_tx, m.history?"<*float>":"NULL");
}