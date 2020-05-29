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
#include "mosquitto.h"

#define PIPE_BUF        40
#define MQTT_TIMEOUT    3000

/**
 * Custom Types
 **/
struct obj {
    int *retval;
    int *client_id;
};

/**
 * Global Variables
 **/
static pthread_mutex_t ipc_pipe_mutex; // use as mutex to IPC pipe for logging
static int pfds[2];

/**
 * Private Prototypes
 **/
static void * MQTT_client_routine(void * arg);
static void on_message_handler(struct mosquitto * MQTT_client, void * obj, const struct mosquitto_message * message);
static void print_help(void);
static int MQTT_msg_num;
static int MQTT_msg_period;
static int msg_qos;
static int will_qos;

/**
 * Functions
 **/
int main(int argc, char *argv[])
{
    int MQTT_clients;

    if(argc != 6)
    {
        print_help();
        exit(EXIT_SUCCESS);
    } else
    {
        MQTT_clients = atoi(argv[1]);
        MQTT_msg_num = atoi(argv[2]);
        MQTT_msg_period = atoi(argv[3]);
        msg_qos = atoi(argv[4]);
        will_qos = atoi(argv[5]);
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

    // Parent's Code - X threads to run as MQTT clients
    close(pfds[0]); // Parent does not need reading end
    
    #if (DEBUG_LVL > 0)
    printf("Parent process (%d) has created child logging process (%d)...\n", parent_pid, child_pid);
    fflush(stdout);
    #endif
    
    mosquitto_lib_init();

    pthread_t threads[MQTT_clients];
    void * exit_codes[MQTT_clients]; // array of pointers to thread returns
    int clients[MQTT_clients];

    for(int i = 0; i < MQTT_clients; i++)
    { 
        clients[i] = i;
        pthread_create(&(threads[i]), NULL, &MQTT_client_routine, &(clients[i]));
    }

    for(int i = 0; i < MQTT_clients; i++) pthread_join(threads[i], &exit_codes[i]); // blocks until all threads terminate

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
    
    for(int i = 0; i < MQTT_clients; i++) free(exit_codes[i]); // release memory alloc'ed to exit codes

    mosquitto_lib_cleanup();

    exit(EXIT_SUCCESS);
}   

static void * MQTT_client_routine(void * arg)
{
    int * retval = malloc(sizeof(int));
    *retval = MOSQ_ERR_SUCCESS;
    
    #if (DEBUG_LVL > 0)
    printf("MQTT client #%d online\n", *((int *) arg));
    fflush(stdout);
    #endif
    
    struct obj * usr_obj = malloc(sizeof(struct obj));
    usr_obj->retval = retval;
    usr_obj->client_id = (int *) arg;

    char* msg;
    asprintf(&msg, "%d", *((int *) arg));
    struct mosquitto* MQTT_client = mosquitto_new(msg, false, usr_obj);

    if(MQTT_client == NULL) *retval = MOSQ_ERR_UNKNOWN;

    asprintf(&msg, "MQTT client #%d disconnected", *((int *) arg));
    *retval = mosquitto_will_set(MQTT_client, "/connections", sizeof(char)*(strlen(msg)+1), msg, will_qos, true);
    
    *retval = mosquitto_connect(MQTT_client, "192.168.0.219", 1883, 360);

    mosquitto_message_callback_set(MQTT_client, &on_message_handler);
    *retval = mosquitto_subscribe(MQTT_client, NULL, "/connections", msg_qos);
    *retval = mosquitto_subscribe(MQTT_client, NULL, "/sensors", msg_qos);
    
    asprintf(&msg, "MQTT client #%d connected", *((int *) arg));
    *retval = mosquitto_publish(MQTT_client, NULL, "/connections", sizeof(char)*(strlen(msg)+1), msg, msg_qos, false);

    *retval = mosquitto_loop_start(MQTT_client);

    for(int i = 0; i < MQTT_msg_num; i++)
    {
        asprintf(&msg, "MQTT client #%d send message %d", *((int *) arg), i);
        *retval = mosquitto_publish(MQTT_client, NULL, "/control", sizeof(char)*(strlen(msg)+1), msg, msg_qos, false);
        sleep(MQTT_msg_period);
    }

    *retval = mosquitto_disconnect(MQTT_client);
    *retval = mosquitto_loop_stop(MQTT_client, false);
    mosquitto_destroy(MQTT_client);

    #if (DEBUG_LVL > 0)
    printf("MQTT client #%d is stopped\n", *((int *) arg));
    fflush(stdout);
    #endif

    pthread_exit(retval);
}

static void on_message_handler(struct mosquitto * MQTT_client, void * obj, const struct mosquitto_message * message)
{
    // char * send_buf;
    char payload[(message->payloadlen)/sizeof(char)];
    for(int i = 0; i < (message->payloadlen)/sizeof(char); i++) payload[i] = ((char *) message->payload)[i];
    printf("%ld: MQTT client #%d received message from topic %s\n\t\t%s\n", time(NULL), *(((struct obj *) obj)->client_id), message->topic, payload);
    // asprintf(&send_buf, "%ld: MQTT client #%d received message from topic %s\n\t\t%s\n", time(NULL), *(((struct obj *) obj)->client_id), message->topic, payload);
    // pthread_mutex_lock(&ipc_pipe_mutex);
    // write(*(pfds+1), send_buf, strlen(send_buf)+1);
    // pthread_mutex_unlock(&ipc_pipe_mutex);
    // free(send_buf);
}

static void print_help(void)
{
    printf("Use this program with 5 command line options: \n");
    printf("\t%-15s : Number of MQTT client threads\n", "\'MQTT_clients\'");
    printf("\t%-15s : MQTT total message number\n", "\'MQTT_msg_num\'");
    printf("\t%-15s : MQTT message periodicity\n", "\'MQTT_msg_period\'");
    printf("\t%-15s : MQTT message QOS\n", "\'msg_qos\'");
    printf("\t%-15s : MQTT LWT QOS\n", "\'will_qos\'");
    fflush(stdout);
}