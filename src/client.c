#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TCP 0
#define UDP 1

void remove_spaces(char* s) {
        char* d = s;
        do {
            while (*d == ' ') {
                ++d;
            }
        } while ((*s++ = *d++));
    }


int main(int argc, char* argv[]) {

    int process_type = TCP;

    if (argc == 4 && atoi(argv[3]) == 1) {
        process_type = UDP;
    }

    if (argc != 3 && !(argc == 4)) {
        printf("2 input is needed : ./clientCalc IPServer Port [ConnexionType (0 TCP, 1 UDP]\n");
        return -1;
    }

    if (process_type == TCP) {
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (client_socket < 0) {
            printf("Error during socket creating process\n");
            return -1;
        }

        struct sockaddr_in serv_addr;

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(argv[2]));

        if (inet_aton(argv[1], &serv_addr.sin_addr) == 0) {
            printf("Erreur lecture IP\n");
            return -1;
        };

        if (connect(client_socket, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != 0) {
            printf("Error during connection with the server\n");
            return -1;
        }

        char* send_buffer = (char*) malloc(1024*sizeof(char));
        char* recv_buffer = (char*) malloc(1024*sizeof(char));

        while (1) {
            printf("Enter the calcul : ");

            if (fgets(send_buffer, 1024, stdin) == NULL) {
                fprintf(stderr, "Error reading\n");
                break;
            }

            send_buffer[strcspn(send_buffer, "\n")] = '\0';

            remove_spaces(send_buffer);

            if (strcmp(send_buffer, "quit") == 0) break;

            size_t len = strlen(send_buffer);
            if (send(client_socket, send_buffer, len, 0) == -1) {
                printf("Error during message sending\n");
                break;
            }

            ssize_t n = recv(client_socket, recv_buffer, 1023, 0);
            if (n <= 0) {
                printf("Error during message reception\n");
                break;
            }
            recv_buffer[n] = '\0';

            printf("%s\n", recv_buffer);

            if (strcmp(recv_buffer, "Timeout: you've been kicked for inactivty.")) break;
        }

        close(client_socket);
        free(send_buffer);
        free(recv_buffer);

        return 0;
    } 
    else {
        int client_socket = socket(AF_INET, SOCK_DGRAM, 0);

        if (client_socket < 0) {
            printf("Error during socket creating process\n");
            return -1;
        }

        struct sockaddr_in serv_addr;

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(argv[2]));

        if (inet_aton(argv[1], &serv_addr.sin_addr) == 0) {
            printf("Erreur lecture IP\n");
            return -1;
        };

        char* send_buffer = (char*) malloc(1024*sizeof(char));
        char* recv_buffer = (char*) malloc(1024*sizeof(char));

        while (1) {
            printf("Enter the calcul : ");

            if (fgets(send_buffer, 1024, stdin) == NULL) {
                fprintf(stderr, "Error reading\n");
                break;
            }

            send_buffer[strcspn(send_buffer, "\n")] = '\0';

            remove_spaces(send_buffer);

            if (strcmp(send_buffer, "quit") == 0) break;

            size_t len = strlen(send_buffer);
            if (sendto(client_socket, send_buffer, len, 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
                printf("Error during message sending\n");
                break;
            }

            socklen_t addr_size = sizeof(addr_size);

            ssize_t n = recvfrom(client_socket, recv_buffer, 1023, 0, (struct sockaddr*) &serv_addr, &addr_size);
            if (n <= 0) {
                printf("Error during message reception\n");
                break;
            }
            recv_buffer[n] = '\0';

            printf("%s\n", recv_buffer);

            if (strcmp(recv_buffer, "Timeout: you've been kicked for inactivty.")) break;
        }

        close(client_socket);
        free(send_buffer);
        free(recv_buffer);

        return 0;
    }

}