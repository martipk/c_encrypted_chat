#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "socket.h"


#ifndef PORT
  #define PORT 50001
#endif
#define BUF_SIZE 512
#define IP "127.0.0.1"
#define ENCRYPTION_MODE "-aes-256-cbc"

int main(void) {

    char username[BUF_SIZE+1];
    printf("Enter Username: ");
    scanf("%s", username);

    char KEY[BUF_SIZE+1];
    printf("Enter Key: ");
    scanf("%s", KEY);
    printf("\n");

    // Initialize connection
    int sock_fd = connect_to_server(PORT, IP);

    // Read input from the user, send it to the server, and then accept the
    // echo that returns. Exit when stdin is closed.
    char buf[BUF_SIZE + 1];
    char msg[BUF_SIZE + 1];
    char msg2[BUF_SIZE*2];
    char msg3[BUF_SIZE*2];
    char enc[BUF_SIZE*3];
    char dec[BUF_SIZE*3];

    fd_set fdset, fdall;

    FD_ZERO(&fdall);

    FD_SET(sock_fd, &fdall);
    FD_SET(fileno(stdin), &fdall);

    close(2); // closing stderr to remove openssl warnings

    while (1) {

        fdset = fdall;

        if (select(sock_fd + 1, &fdset, NULL, NULL, NULL) < 0) {
            perror("select");
            return 1;
        }

        // If  there is data  from  stdin , send to server
        if (FD_ISSET(fileno(stdin), &fdset)) {
            int num_read = read(fileno(stdin), buf, BUF_SIZE);
            if (num_read == 0) {
                break;
            }
            buf[num_read] = '\0';

            // Clearly outline where the message ends
            const char delim[3] = "\r\n";
            char *rest;
            char *text = strtok_r(buf, delim, &rest);
            if ((rest != NULL) && (rest[0] != '\0')) {
                text = rest;
            }

            snprintf(msg, BUF_SIZE, "[%s]: %s", username, text);
            if (text == NULL) // Check if message is empty and dont send the text, since text will be "(null)"
                snprintf(msg, BUF_SIZE, "[%s]: ", username);

            // encrypt message before sending
            snprintf(enc, BUF_SIZE*2, "echo '%s' | openssl enc -a -e %s -k %s", msg, ENCRYPTION_MODE, KEY); 
            // tr removes trailing newline from cipher text in stdout of commandline

            memset(msg2, 0, BUF_SIZE*2);
            FILE *fp = popen(enc, "r");
            while (fgets(msg, BUF_SIZE, fp) != NULL) {
                //printf("CIPH:*%s*\n", msg);
                strncat(msg2, msg, strlen(msg));
            }
            pclose(fp);
            //printf("CIPHERS:*%s*\n", msg2);

            snprintf(msg3, BUF_SIZE*2, "%s\r\n", msg2);

            //write encoded message to server

            int num_written = write(sock_fd, msg3, strlen(msg3));
            if (num_written != strlen(msg3)) {
                perror("client: write");
                close(sock_fd);
                exit(1);
            }
        }

        // Read from server
        if (FD_ISSET(sock_fd, &fdset)) {
            int num_read = read(sock_fd, buf, BUF_SIZE);
            buf[num_read] = '\0'; 

            if (strncmp(buf, "[SERVER]", 8) == 0) { // if server command, dont decrypt since its plaintext
                printf("%s", buf);
                if (strncmp(buf, "[SERVER] Shutdown.", 18) == 0) {
                    exit(0);
                }
            } else {
                snprintf(dec, BUF_SIZE*2, "echo '%s' | openssl enc -a -d %s -k %s", buf, ENCRYPTION_MODE, KEY);
                FILE *fp = popen(dec, "r");
                int x = 0; // Allows 2 sets of clients to communcated without disruption
                while (fgets(msg, BUF_SIZE, fp) != NULL) {
                    if((strncmp(msg, "[", 1) == 0) || (x > 0)) { // x needed incase the message send in more than one piece
                        printf("\033[1m%s\033[0m", msg);
                        x++;
                    }
                }
                pclose(fp);
            }
        }
    }

    close(sock_fd);
    return 0;
}
