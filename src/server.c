#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

#include "calculator.h"

#define BUFF_LEN 1024
#define MAX_CLIENT 10
#define MAX_PORT_LEN 32
#define TCP 1
#define UDP 0

int main(int argc, char *argv[]) {

    struct timeval timeout;
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

    int serv_port = -1;
    int protocol = 0;

    if (argc < 3 || atoi(argv[1]) < 22000 || atoi(argv[2]) < 0){
        do {
            printf("It is necessary to inform a valid server Port (>22000) or valid timeout (>0): \n");
            scanf("%d %d", &serv_port, (int*) &timeout.tv_sec);
        } while(serv_port < 22000 || &timeout.tv_sec < 0);
    }
    else serv_port = atoi(argv[1]);

    if (argc == 3){
        if (strcmp(argv[2], "tcp") == 0 || strcmp(argv[2], "TCP") == 0)
            protocol = TCP;
        else if (strcmp(argv[2], "udp") == 0 || strcmp(argv[2], "UDP") == 0)
            protocol = UDP;
    }
    else protocol = UDP;

    int server_fd;
    // SOCK_STREAM is TCP == 0. SOCK_DGRAM is UDP == 1;
    if (protocol) server_fd = socket(AF_INET, SOCK_STREAM, 0);
    else server_fd = socket(AF_INET, SOCK_DGRAM, 0);     
    
    if (server_fd == -1) return -1;

    
    struct sockaddr_in serv_address, client_address;
    
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

    if (protocol)
        listen(server_fd, MAX_CLIENT);
    
    while (true)
    {
        int calc_socket = 0;
        socklen_t client_addr_size = sizeof(client_address);
        
        if (protocol) {
            calc_socket = accept(server_fd, (struct sockaddr *) &client_address, &client_addr_size); 
            if (setsockopt(calc_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
                printf("Problem setting socket option");
            }
        }

        if (calc_socket == -1) {
            printf("Problem accepting connection");
        }
        
        pid_t pid = fork();

        if (pid == 0) { 
            
            if (protocol)
                close(server_fd); 

            int Inactivity = 0;

            char send_buffer[BUFF_LEN];
            char recv_buffer[BUFF_LEN];

            EXPRESSION expression;
            do { 
                ssize_t n;
                if (protocol)
                    n = recv(calc_socket, recv_buffer, BUFF_LEN, 0);
                else 
                    n = recvfrom(calc_socket, recv_buffer, BUFF_LEN, 0, (struct sockaddr*)&client_address, &client_addr_size);

                if (n == 0) break;

                if (n < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        const char *timeout_msg = "Timeout: you've been kicked for inactivty.\n";
                        send(calc_socket, timeout_msg, strlen(timeout_msg), 0);
                        printf("[-] %s (Inactivity)\n", inet_ntoa(client_address.sin_addr));
                        Inactivity = 1;
                    } else {
                        printf("Error during reception");
                    }
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
                
                if (protocol)
                    send(calc_socket, send_buffer, (size_t) m, 0);
                else
                    sendto(calc_socket, send_buffer, (size_t) m, 0, (struct sockaddr*)&client_address, client_addr_size);

            } while (protocol);

            close(calc_socket);

            if (!Inactivity) printf("[-] %s (Deconnection)\n", inet_ntoa(client_address.sin_addr));

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
