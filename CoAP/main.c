/**
 * Includes
 **/
#define _GNU_SOURCE
#define BUILDING_GATEWAY
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <time.h>

#define PIPE_BUF        40

/**
 * Global Variables
 **/
static pthread_mutex_t ipc_pipe_mutex; // use as mutex to IPC pipe for logging
static int pfds[2];

/**
 * Private Prototypes
 **/
static void * COAP_client_routine(void * arg);
static void print_help(void);
static int COAP_msg_num;
static int COAP_msg_period;

/**
 * Functions
 **/
int main(int argc, char *argv[])
{
    int COAP_clients;

    if(argc != 4)
    {
        print_help();
        exit(EXIT_SUCCESS);
    } else
    {
        COAP_clients = atoi(argv[1]);
        COAP_msg_num = atoi(argv[2]);
        COAP_msg_period = atoi(argv[3]);
    }

    pid_t parent_pid, child_pid;
    int result;
    parent_pid = getpid();
    
    #if (DEBUG_LVL > 0)
    printf("Parent process (%d) is started...\n", parent_pid);
    fflush(stdout);
    #endif
    
    result = pipe(pfds); // Create separate log process

    pthread_mutex_init(&ipc_pipe_mutex, NULL);

    child_pid = fork();
    
    if(child_pid == 0) // Child's Code - logging process
    {
        close(pfds[1]); // Child does not need writing end

        FILE * log_data = fopen("clients.log", "w");
        int sequence = 0;

        child_pid = getpid();
        char rcv_buf[PIPE_BUF];
        char * buf_to_write;
        char * msg;
        
        #if (DEBUG_LVL > 0)
        printf("\t\tChild process (%d) of parent (%d) is started...\n", child_pid, parent_pid);
        fflush(stdout);
        #endif        
        
        while((result = read(pfds[0], rcv_buf, PIPE_BUF)) > 0) // Process will stay in loop
        {
            asprintf(&buf_to_write, "%d %s\n", sequence++, rcv_buf);
            fwrite(buf_to_write, strlen(buf_to_write), 1, log_data); // Write data from pipe to log file
            free(buf_to_write);
        }

        if(result == -1) // If reading from pipe resulted in an error
        {
            asprintf(&msg, "%d %ld Error reading from pipe, pipe closed\n", sequence++, time(NULL));
        } else if(result == 0)
        {
            asprintf(&msg, "%d %ld Pipe between parent (%d) and child (%d) terminated normally\n", sequence++, time(NULL), parent_pid, child_pid);
        }

        fwrite(msg, strlen(msg), 1, log_data); // Write exit message to log file
        free(msg);
        close(pfds[0]);
        fclose(log_data); // Close log file
        
        #if (DEBUG_LVL > 0)
        printf("\t\tChild process (%d) of parent (%d) is terminating...\n", child_pid, parent_pid);
        fflush(stdout);
        #endif

        _exit(EXIT_SUCCESS); 
    }

    // Parent's Code - X threads to run as COAP clients
    close(pfds[0]); // Parent does not need reading end
    
    #if (DEBUG_LVL > 0)
    printf("Parent process (%d) has created child logging process (%d)...\n", parent_pid, child_pid);
    fflush(stdout);
    #endif

    pthread_t threads[COAP_clients];
    void * exit_codes[COAP_clients]; // array of pointers to thread returns
    int clients[COAP_clients];

    for(int i = 0; i < COAP_clients; i++)
    { 
        clients[i] = i;
        pthread_create(&(threads[i]), NULL, &COAP_client_routine, &(clients[i]));
    }

    for(int i = 0; i < COAP_clients; i++) pthread_join(threads[i], &exit_codes[i]); // blocks until all threads terminate

    #if (DEBUG_LVL > 0)
    printf("Threads stopped");
    fflush(stdout);
    #endif

    close(pfds[1]); // First, let threads terminate, then shut down IPC pipe -> ensures all data in pipe is written before closing
    wait(NULL); // Wait on child process

    #if (DEBUG_LVL > 0)
    printf("Child process stopped. Cleaning up\n");
    fflush(stdout);
    #endif
    
    for(int i = 0; i < COAP_clients; i++) free(exit_codes[i]); // release memory alloc'ed to exit codes

    exit(EXIT_SUCCESS);
}   

static void * COAP_client_routine(void * arg)
{
    int * retval = malloc(sizeof(int));
    *retval = EXIT_SUCCESS;
    
    #if (DEBUG_LVL > 0)
    printf("COAP client #%d online\n", *((int *) arg));
    fflush(stdout);
    #endif

    for(int i = 0; i < COAP_msg_num; i++)
    {
        system("coap-client coap://192.168.0.226/Espressif");
        sleep(COAP_msg_period);
    }

    #if (DEBUG_LVL > 0)
    printf("COAP client #%d is stopped\n", *((int *) arg));
    fflush(stdout);
    #endif

    pthread_exit(retval);
}

static void print_help(void)
{
    printf("Use this program with 3 command line options: \n");
    printf("\t%-15s : Number of COAP client threads\n", "\'COAP_clients\'");
    printf("\t%-15s : COAP total message number\n", "\'COAP_msg_num\'");
    printf("\t%-15s : COAP message periodicity\n", "\'COAP_msg_period\'");
    fflush(stdout);
}