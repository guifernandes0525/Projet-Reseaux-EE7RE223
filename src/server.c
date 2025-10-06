#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "calculator.h"

#define BUFF_LEN 1024
#define MAX_CLIENT 10
#define MAX_PORT_LEN 32

int main(int argc, char *argv[]) {

    int serv_port = -1;

    if (argc < 2 || atoi(argv[1]) < 22000){
        do {
            printf("It is necessary to inform a valid server Port (>22000): \n");
            scanf("%d", &serv_port);
        } while(serv_port < 22000);
    }
    else serv_port = atoi(argv[1]);

    int server_fd;

    // SOCK_STREAM is TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    
    if (server_fd == -1) return -1;

    struct sockaddr_in serv_address;
    // ipv4
    serv_address.sin_family = AF_INET; 
    serv_address.sin_port = htons(serv_port);
    // binds with all interfaces ()
    serv_address.sin_addr.s_addr = INADDR_ANY; 

    int opt = 1;

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        printf("Problem setting options for socket\n");        
        return -1;
    }
    
    socklen_t size_addr = sizeof(serv_address);

    if (bind(server_fd, (struct sockaddr *) &serv_address, size_addr)) {
        printf("Problem binding socket\n");        
        return -1;
    }

    listen(server_fd, MAX_CLIENT);
    
    struct sockaddr_in client_address;

    while (true)
    {
        int calc_socket;
        socklen_t client_addr_size = sizeof(client_address);
        calc_socket = accept(server_fd, (struct sockaddr *) &client_address, &client_addr_size); 
        
        if (calc_socket == -1) {
            printf("Problem accepting connection");
        }
        
        pid_t pid = fork();

        if (pid == 0) { 
            
            close(server_fd); 

            char send_buffer[BUFF_LEN];
            char recv_buffer[BUFF_LEN];

            EXPRESSION expression;

            while (1) {
                ssize_t n = recv(calc_socket, recv_buffer, BUFF_LEN, 0);

                if (n == 0) break;

                if (n < 0) { // error
                    printf("Error reading recv_buffer\n");
                    break;
                }

                recv_buffer[n] = '\0'; 

                clean_input(recv_buffer);
                
                int m;
                if (format_input(recv_buffer, &expression, inet_ntoa(client_address.sin_addr))) {
                    
                    m = snprintf(send_buffer, BUFF_LEN, "Invalid input format.\n");
                }
                else if (invalid_range(expression)) {
                    m = snprintf(send_buffer, BUFF_LEN, "Invalid range. Valid range is (0-10000).\n");
                }
                else{
                    calculate(&expression);
                    m = snprintf(send_buffer, BUFF_LEN, "%s%.2f\n", expression.message, expression.result);
                }
                send(calc_socket, send_buffer, (size_t) m, 0);
            }

            close(calc_socket);

            exit(0);

        }
        else if (pid > 0) { // this is the father process
            close(calc_socket);
        }
        else {
            close(calc_socket);
        }
    }

    return 0;
}
