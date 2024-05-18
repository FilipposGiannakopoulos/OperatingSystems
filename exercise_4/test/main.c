#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>

#define DEFAULT_HOST "os4.iot.dslab.ds.open-cloud.xyz"
#define DEFAULT_PORT 20241
#define BUFFER_SIZE 1024

int debug = 0;

void print_help(){
    printf("Available commands:\n");
    printf("help : Print this help message\n");
    printf("exit : Exit the program\n");
    printf("get : Retrieve data from the server\n");
    printf("N name surname reason : Request exit permission\n");
}

void parse_arguments(int argc, char *argv[], char**host, int *port){
    *host = DEFAULT_HOST;
    *port = DEFAULT_PORT;
    for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--host") == 0 && i + 1 < argc) {
                *host = argv[i + 1];
                i++;
            } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
                *port = atoi(argv[i + 1]);
                i++;
            } else if (strcmp(argv[i], "--debug") == 0) {
                debug = 1;
            } else {
                fprintf(stderr, "Unknown argument: %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }
}

int create_connect_socket(char *host, int port){
    int sockfd;
    struct sockaddr_in server_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void handle_get(int sockfd){
    char buffer[BUFFER_SIZE];
    int n;
    
    strcpy(buffer, "get\n");
    send(sockfd, buffer, strlen(buffer), 0);

    if (debug) {
        printf("[DEBUG] sent 'get'\n");
    }

    n = recv(sockfd, buffer, BUFFER_SIZE, 0);
    buffer[n] = '\0';

    if (debug) {
        printf("[DEBUG] received '%s'\n", buffer);
    }

    int event, light, temperature, timestamp;
    sscanf(buffer, "%d %d %d %d", &event, &light, &temperature, &timestamp);
    printf("Latest event: %d\n", event);

    switch(event){
        case 0: printf("boot"); break;
        case 1: printf("setup"); break;
        case 2: printf("interval"); break;
        case 3: printf("button"); break;
        case 4: printf("motion"); break;
        default: printf("unknown");
    }

    printf(" (%d)\n", event);
    printf("Temperature is: %.2f\n", temperature / 100.0);
    printf("Light level is: %d\n", light);

    time_t t = timestamp;
    struct tm *lt = localtime(&t);
    printf("Timestamp is :%s", asctime(lt));
}

void handle_exit_request(int sockfd, char *request){
    char buffer[BUFFER_SIZE];
    int n;
    
    send (sockfd, request, strlen(request), 0);
    if(debug){
        printf("[DEBUG] sent '%s'\n", request);
    }
    n = recv(sockfd, buffer, BUFFER_SIZE, 0);
    buffer[n] = '\0';

    if (debug) {
        printf("[DEBUG] received '%s'\n", buffer);
    }

    if (strncmp(buffer, "try again", 9) == 0){
        printf("Response: Try again\n");
    } else {
        printf("Send verification code: '%s'\n", buffer);
        printf("Enter verification code: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        send(sockfd, buffer, strlen(buffer), 0);
        if (debug) {
            printf("[DEBUG] sent '%s'\n", buffer);
        }
        n = recv(sockfd, buffer, BUFFER_SIZE, 0);
        buffer[n] = '\0';
        if (debug){
            printf("[DEBUG] received '%s'\n", buffer);
        }
        printf("Response: %s\n", buffer);
    }
}

void run_client(int sockfd) {
    fd_set readfds;
    char buffer[BUFFER_SIZE];

    while(1){
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;
        
        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(STDIN_FILENO, &readfds)){
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
                
            if (strcmp(buffer, "help") == 0){
                print_help();
            } else if (strcmp(buffer, "exit") == 0){
                break;
            } else if (strcmp(buffer, "get") == 0){
                handle_get(sockfd);
            } else if (strncmp(buffer, "N ", 2) == 0){
                handle_exit_request(sockfd, buffer);
            } else {
                printf("Unknown command: %s\n", buffer);
            }
            
        }
    }

    if ()

}
